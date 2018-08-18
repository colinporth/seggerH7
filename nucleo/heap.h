#pragma once

#define DTCM_ADDR 0x20000000
#define DTCM_SIZE 0x00020000

//{{{
#ifdef __cplusplus
 extern "C" {
#endif
//}}}

void dtcmInit (uint32_t start, uint32_t size);
uint8_t* dtcmAlloc (size_t bytes);
void dtcmFree (void* p);

uint8_t* sram123Alloc (size_t bytes);
void sram123Free (void* p);
size_t getSram123FreeSize();
size_t getSram123MinFreeSize();

void sdRamInit (uint32_t start, uint32_t size);
uint8_t* sdRamAlloc (size_t size);
uint8_t* sdRamAllocInt (size_t size);
void sdRamFree (void* p);
size_t getSdRamFreeSize();
size_t getSdRamMinFreeSize();

//{{{
#ifdef __cplusplus
}
#endif
//}}}
