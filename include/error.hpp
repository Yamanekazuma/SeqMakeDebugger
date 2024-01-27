#pragma once

#include <cstdint>
#include <source_location>
#include <stdexcept>
#include <string>

namespace error {
class WinApiError : public std::runtime_error {
 public:
  explicit WinApiError(uint32_t errorcode, std::source_location loc = std::source_location::current()) noexcept;
  inline const char* what() const noexcept override { return message_.c_str(); }

 private:
  std::string message_;
};
};  // namespace error
