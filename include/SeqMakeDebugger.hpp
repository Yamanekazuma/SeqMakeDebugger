#pragma once

#include <Windows.h>

namespace seq {

class Preferences {
 public:
  inline static const Preferences& GetInstance() noexcept {
    static Preferences instance{};
    return instance;
  }

  inline bool is_redirected() const noexcept { return is_redirected_; }

 private:
  static inline bool is_redirected_;
  Preferences() noexcept;
};

};  // namespace seq
