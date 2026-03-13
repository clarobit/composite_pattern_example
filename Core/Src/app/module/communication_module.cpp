#include "app/module/communication_module.hpp"

namespace app::module {

CommunicationModule::CommunicationModule(
    UART_HandleTypeDef *huart, GPIO_TypeDef *bt_power_port,
    uint16_t bt_power_pin, GPIO_TypeDef *bt_state_port, uint16_t bt_state_pin,
    uint8_t *rx_buffer, uint16_t frame_size)
    : huart_(huart), bluetooth_(app::mcu::Uart(huart),
                                app::mcu::GpioOut(bt_power_port, bt_power_pin),
                                app::mcu::GpioIn(bt_state_port, bt_state_pin)),
      rx_buf_(rx_buffer), frame_size_(frame_size) {}

void CommunicationModule::start() { bluetooth_.powerOn(); }

void CommunicationModule::stop() { bluetooth_.powerOff(); }

HAL_StatusTypeDef CommunicationModule::startReceive() {
  return HAL_UART_Receive_IT(huart_, rx_buf_, frame_size_);
}

HAL_StatusTypeDef CommunicationModule::send(const uint8_t *data, uint16_t len) {
  return bluetooth_.send(data, len);
}

uint8_t *CommunicationModule::getRxBuffer() { return rx_buf_; }

uint16_t CommunicationModule::getFrameSize() const { return frame_size_; }

int16_t CommunicationModule::parseInt16(const uint8_t *data) {
  const uint16_t value =
      (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);

  return static_cast<int16_t>(value);
}

} // namespace app::module