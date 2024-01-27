#pragma once

#include <SeqMaker/SeqMaker.h>

#include <Windows.h>

#include "error.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

class Debugger {
 public:
  explicit Debugger(std::string_view filename, SEQ_UNITS unit);
  ~Debugger() noexcept;

  uint32_t SetAddress(std::uint32_t address);
  void Start(std::size_t length);
  const char* GetNGram(std::size_t n) const noexcept;

  inline uint32_t BaseAddress() const noexcept { return proc_->baseaddress(); }

 private:
  std::string filename_;
  SEQMAKER seq_;

  class Process {
   public:
    Process(std::string_view filename);

    inline ~Process() noexcept {
      TerminateProcess(hProcess_, 0);
      // プロセス終了まで最大1秒待機する
      WaitForSingleObject(hProcess_, 1000);
      CloseHandle(hThread_);
      CloseHandle(hProcess_);
    }

    inline uint32_t processId() const noexcept { return processId_; }
    inline uint32_t threadId() const noexcept { return threadId_; }
    inline HANDLE hProcess() const noexcept { return hProcess_; }
    inline HANDLE hThread() const noexcept { return hThread_; }
    inline uint32_t baseaddress() const noexcept { return baseaddress_; }

   private:
    uint32_t processId_;
    uint32_t threadId_;
    HANDLE hProcess_;
    HANDLE hThread_;
    uint32_t baseaddress_;
  };

  std::optional<Process> proc_;

  void CollectDataForNGram(const DEBUG_EVENT& de);

  struct HookData {
    bool isReplaced;
    std::uint8_t original_code;
  };

  std::unordered_map<std::uint32_t, HookData> hooks_;
};
