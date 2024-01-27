#include "debugger.hpp"

#include <ntstatus.h>
#include <psapi.h>
#include <filesystem>
#include <stdexcept>

using namespace std;

static void context_to_registers(const WOW64_CONTEXT& context, Registers& regs) noexcept;
static uint64_t get_base_address(HANDLE hProcess) noexcept;

Debugger::Debugger(string_view filename, SEQ_UNITS unit) {
  filesystem::path path{filename};

  if (filesystem::is_directory(path)) {
    throw invalid_argument("The filename is directory.");
  } else if (!filesystem::exists(path)) {
    throw invalid_argument("The specified file is not exists.");
  }

  filename_ = filesystem::absolute(path).string();

  try {
    proc_.emplace(filename_.c_str());
  } catch (...) {
    throw;
  }

  seq_ = SeqMaker_Init(proc_->processId(), unit);
  if (seq_ == nullptr) {
    throw runtime_error("SeqMaker_Init failed.");
  }
}

Debugger::Process::Process(string_view filename) {
  PROCESS_INFORMATION pi{};
  STARTUPINFOA si{};
  si.cb = sizeof(si);
  if (CreateProcessA(filename.data(), nullptr, nullptr, nullptr, FALSE, DEBUG_PROCESS | CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi) == 0) {
    throw error::WinApiError(GetLastError());
  }

  processId_ = pi.dwProcessId;
  threadId_ = pi.dwThreadId;
  hProcess_ = pi.hProcess;
  hThread_ = pi.hThread;

  uint64_t address = get_base_address(hProcess_);
  DEBUG_EVENT de{};
  while (address == 0) {
    if (WaitForDebugEvent(&de, INFINITE) == 0) {
      throw error::WinApiError(GetLastError());
    }
    if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
      ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
    } else {
      ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
    }

    address = get_base_address(hProcess_);
  }

  baseaddress_ = static_cast<uint32_t>(address & 0xFFFFFFFF);
}

static uint64_t get_base_address(HANDLE hProcess) noexcept {
  HMODULE modules[1];
  DWORD needed;
  if (EnumProcessModules(hProcess, modules, sizeof(modules), &needed) == 0) {
    return 0;
  }
  return (uint64_t)modules[0];
}

Debugger::~Debugger() noexcept {
  SeqMaker_DeInit(seq_);

  for (const auto& it : hooks_) {
    if (it.second.isReplaced) {
      WriteProcessMemory(proc_->hProcess(), reinterpret_cast<void*>(it.first), &(it.second.original_code), 1, nullptr);
    }
  }
}

uint32_t Debugger::SetAddress(uint32_t address) {
  uint32_t addr = address + proc_->baseaddress();
  if (address >= 0x400000) {
    addr -= 0x400000;
  }

  if (hooks_.find(addr) != hooks_.end()) {
    return addr;
  }

  uint8_t original_code[1];
  if (ReadProcessMemory(proc_->hProcess(), reinterpret_cast<LPCVOID>(addr), original_code, 1, nullptr) == 0) {
    throw error::WinApiError(GetLastError());
  }

  hooks_.emplace(addr, HookData{true, original_code[0]});

  static uint8_t int3[1] = {0xCC};
  if (WriteProcessMemory(proc_->hProcess(), reinterpret_cast<LPVOID>(addr), int3, 1, nullptr) == 0) {
    throw error::WinApiError(GetLastError());
  }

  return addr;
}

void Debugger::Start(size_t length) {
  DEBUG_EVENT de{};
  ResumeThread(proc_->hThread());

  size_t cnt = 0;

  try {
    while (cnt < length) {
      if (WaitForDebugEvent(&de, INFINITE) == 0) {
        throw error::WinApiError(GetLastError());
      }

      if (de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
        ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
        break;
      } else if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
        if (de.u.Exception.ExceptionRecord.ExceptionCode == STATUS_WX86_BREAKPOINT) {
          uint32_t addr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(de.u.Exception.ExceptionRecord.ExceptionAddress) & 0xFFFFFFFF);
          auto&& iter = hooks_.find(addr);
          if (iter != hooks_.end()) {
            CollectDataForNGram(de);
            if (WriteProcessMemory(proc_->hProcess(), de.u.Exception.ExceptionRecord.ExceptionAddress, &(iter->second.original_code), 1, nullptr) ==
                0) {
              throw error::WinApiError(GetLastError());
            }
            iter->second.isReplaced = false;
            ++cnt;
            ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
          } else {
            ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
          }
        } else if (de.u.Exception.ExceptionRecord.ExceptionCode == STATUS_WX86_SINGLE_STEP) {
          CollectDataForNGram(de);
          ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
          ++cnt;
        } else {
          ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
        }
      } else {
        ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
      }
      // ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
    }
  } catch (...) {
    throw;
  }
}

void Debugger::CollectDataForNGram(const DEBUG_EVENT& de) {
  // CONTEXTを取得
  // Registerを構築
  // AddInstruction
  // trapフラグ設定

  HANDLE dbgThread = OpenThread(THREAD_ALL_ACCESS, FALSE, de.dwThreadId);
  if (dbgThread == nullptr) {
    throw error::WinApiError(GetLastError());
  }

  WOW64_CONTEXT context{};
  context.ContextFlags = CONTEXT_SEGMENTS | CONTEXT_INTEGER | CONTEXT_CONTROL;
  if (Wow64GetThreadContext(dbgThread, &context) == 0) {
    throw error::WinApiError(GetLastError());
  }

  Registers regs{};
  context_to_registers(context, regs);
  if (SeqMaker_AddInstruction(seq_, &regs)) {
    throw runtime_error("AddInstruction failed.");
  }

  context.EFlags |= 0x00000100;
  if (Wow64SetThreadContext(dbgThread, &context) == 0) {
    throw error::WinApiError(GetLastError());
  }
}

static void context_to_registers(const WOW64_CONTEXT& context, Registers& regs) noexcept {
  regs.EAX = context.Eax;
  regs.EBX = context.Ebx;
  regs.ECX = context.Ecx;
  regs.EDX = context.Edx;
  regs.EBP = context.Ebp;
  regs.ESP = context.Esp;
  regs.ESI = context.Esi;
  regs.EDI = context.Edi;
  regs.EIP = context.Eip;
  regs.DS = static_cast<uint16_t>(context.SegDs);
  regs.ES = static_cast<uint16_t>(context.SegEs);
  regs.CS = static_cast<uint16_t>(context.SegCs);
  regs.SS = static_cast<uint16_t>(context.SegSs);
  regs.FS = static_cast<uint16_t>(context.SegFs);
  regs.GS = static_cast<uint16_t>(context.SegGs);
}

const char* Debugger::GetNGram(size_t n) const noexcept {
  switch (n) {
    case 1:
      return SeqMaker_CreateUniGram(seq_);
    case 2:
      return SeqMaker_CreateBiGram(seq_);
    case 3:
      return SeqMaker_CreateTriGram(seq_);
    default:
      return nullptr;
  }
}
