#pragma once
#include "../system/stm32h7xx.h"

//{{{
#ifdef __cplusplus
 extern "C" {
#endif
//}}}

void lsm303c_init_la();
uint8_t lsm303c_read_la_status();
void lsm303c_read_la (int16_t* buf);

void lsm303c_init_mf();
uint8_t lsm303c_read_mf_status();
void lsm303c_read_mf (int16_t* buf);

//{{{
#ifdef __cplusplus
}
#endif
//}}}
