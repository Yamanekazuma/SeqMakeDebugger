#include "SeqMakeDebugger.hpp"

#include <SeqMaker/SeqMaker.h>

#include "debugger.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

using namespace std;

static std::string run(string_view filename, uint32_t addr, size_t length, size_t n, SEQ_UNITS unit);
static optional<unsigned int> str_to_uint(string_view s) noexcept;

int main(int argc, char** argv) {
  bool is_redirected = seq::Preferences::GetInstance().is_redirected();

  if (argc != 6) {
    cerr << "Usage: ./SeqMakeDebugger.exe <filename> <address> <length> <n> <opcode|mnemonic|original>" << endl;
    return 0;
  }

  char* filename = argv[1];
  auto address = str_to_uint(argv[2]);
  if (!address.has_value()) {
    cerr << "The address should be a number." << endl;
    return 1;
  }

  auto length = str_to_uint(argv[3]);
  if (!length.has_value()) {
    cerr << "The length should be a number." << endl;
    return 1;
  }

  auto n = str_to_uint(argv[4]);
  if (!n.has_value()) {
    cerr << "The n should be a number." << endl;
    return 1;
  }
  if (*n == 0 || *n >= 4) {
    cerr << "The n should be 1-3." << endl;
    return 1;
  }

  SEQ_UNITS unit;
  if (strcmp(argv[5], "opcode") == 0) {
    unit = SEQ_UNIT_OPCODE;
  } else if (strcmp(argv[5], "mnemonic") == 0) {
    unit = SEQ_UNIT_MNEMONIC;
  } else if (strcmp(argv[5], "original") == 0) {
    unit = SEQ_UNIT_ORIGINAL;
  } else {
    cerr << "The unit should be opcode or mnemonic, original.";
    return 1;
  }

  if (!is_redirected) {
    cout << "===== INPUTS =====" << endl;
    cout << "filename: " << filename << endl;
    cout << "address: 0x" << hex << *address << endl;
    cout << "length: " << *length << endl;
    cout << "n: " << *n << endl;
    cout << "unit: " << argv[5] << endl;
    cout << "==================" << endl;
  }

  try {
    std::string s{run(filename, *address, *length, *n, unit)};
    if (is_redirected) {
      cout << s << endl;
    } else {
      cout << "NGram = " << s << endl;
    }
  } catch (const exception& e) {
    cerr << e.what() << endl;
    return 1;
  }

  return 0;
}

static std::string run(string_view filename, uint32_t addr, size_t length, size_t n, SEQ_UNITS unit) {
  try {
    Debugger dbg{filename, unit};
    auto temp = dbg.SetAddress(addr);
    if (!seq::Preferences::GetInstance().is_redirected()) {
      cout << "baseaddress: 0x" << hex << dbg.BaseAddress() << endl;
      cout << "set_address: 0x" << hex << temp << endl;
    }
    dbg.Start(length);
    const char* p = dbg.GetNGram(n);
    if (p == nullptr) {
      return std::string{"{}"};
    } else {
      return std::string{p};
    }
  } catch (...) {
    throw;
  }
}

static optional<unsigned int> str_to_uint(string_view s) noexcept {
  int base = 10;

  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    base = 16;
  }

  const char* p = s.data();
  char* end;
  unsigned long x = strtoul(p, &end, base);
  if (*end != '\0' || errno == ERANGE || errno == EINVAL) {
    return nullopt;
  }

  return make_optional(static_cast<int>(x));
}
