#pragma once

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void app_init(void);

void control_update(void);
void shoot_update(void);
void feeder_update(void);
void communication_update(void);

void comm_set_rx_done(void);

#ifdef __cplusplus
}
#endif