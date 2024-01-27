#include "SeqMakeDebugger.hpp"

#include <io.h>
#include <cstdio>

seq::Preferences::Preferences() noexcept {
  is_redirected_ = (_isatty(STDOUT_FILENO) == 0);
}
