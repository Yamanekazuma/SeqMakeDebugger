#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t EAX;
  uint32_t EBX;
  uint32_t ECX;
  uint32_t EDX;
  uint32_t EBP;
  uint32_t ESP;
  uint32_t ESI;
  uint32_t EDI;
  uint32_t EIP;
  uint16_t DS;
  uint16_t ES;
  uint16_t CS;
  uint16_t SS;
  uint16_t FS;
  uint16_t GS;
} Registers;

#ifdef __cplusplus
}
#endif
