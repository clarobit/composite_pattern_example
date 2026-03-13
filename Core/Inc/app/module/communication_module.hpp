#pragma once

#include <cstdint>

#include "app/device/bluetooth.hpp"

namespace app::module {

class CommunicationModule {
public:
  CommunicationModule(UART_HandleTypeDef *huart,
                      GPIO_TypeDef *bt_power_port,
                      uint16_t bt_power_pin,
                      GPIO_TypeDef *bt_state_port,
                      uint16_t bt_state_pin,
                      uint8_t *rx_buffer,
                      uint16_t frame_size);

  void start();
  void stop();

  HAL_StatusTypeDef startReceive();
  HAL_StatusTypeDef send(const uint8_t *data, uint16_t len);

  uint8_t *getRxBuffer();
  uint16_t getFrameSize() const;

  static int16_t parseInt16(const uint8_t *data);

private:
  UART_HandleTypeDef *huart_;
  app::device::Bluetooth bluetooth_;

  uint8_t *rx_buf_;
  uint16_t frame_size_;
};

} // namespace app::module