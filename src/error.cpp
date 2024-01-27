#include "error.hpp"

#include <Windows.h>

#include <bit>
#include <format>

using namespace std;
using namespace error;

WinApiError::WinApiError(uint32_t errorcode, source_location loc) noexcept : runtime_error{""} {
  char* buffer = nullptr;
  auto result = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorcode,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), std::bit_cast<LPSTR>(&buffer), 0, nullptr);
  if (result == 0) {
    message_ = format("Unknown Errorcode: {:08X}", errorcode);
  } else {
    message_ = format("Errorcode: {:08X}\n{}", errorcode, buffer);
  }

  message_.append(format("from {}:{}:{}: {}", loc.file_name(), loc.line(), loc.column(), loc.function_name()));
};
