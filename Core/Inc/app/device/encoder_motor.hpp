#pragma once

#include "app/mcu/encoder.hpp"
#include "app/mcu/gpio.hpp"
#include "app/mcu/pwm.hpp"
#include "main.h"
#include <cstdint>

namespace app::device {

class EncoderMotor {
public:
  EncoderMotor(TIM_HandleTypeDef *pwm_htim, uint32_t pwm_channel,
               GPIO_TypeDef *dir_port, uint16_t dir_pin,
               TIM_HandleTypeDef *encoder_htim);

  void start();
  void stop();

  void forward();
  void reverse();

  void setDuty(uint16_t duty);
  uint16_t getDuty() const;
  uint16_t getMaxDuty() const;

  int32_t getCount() const;
  void setCount(int32_t count);
  void resetCount();

private:
  app::mcu::Pwm pwm_;
  app::mcu::GpioOut dir_;
  app::mcu::Encoder encoder_;
};

} // namespace app::device