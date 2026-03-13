#include "app/device/encoder_motor.hpp"

namespace app::device {

EncoderMotor::EncoderMotor(TIM_HandleTypeDef *pwm_htim, uint32_t pwm_channel,
                           GPIO_TypeDef *dir_port, uint16_t dir_pin,
                           TIM_HandleTypeDef *encoder_htim)
    : pwm_(pwm_htim, pwm_channel), dir_(dir_port, dir_pin),
      encoder_(encoder_htim) {}

void EncoderMotor::start() {
  pwm_.start();
  encoder_.start();
}

void EncoderMotor::stop() {
  pwm_.setDuty(0);
  pwm_.stop();
  encoder_.stop();
}

void EncoderMotor::forward() { dir_.set(); }

void EncoderMotor::reverse() { dir_.reset(); }

void EncoderMotor::setDuty(uint16_t duty) {
  const uint16_t max_duty = pwm_.getMaxDuty();

  if (duty > max_duty) {
    duty = max_duty;
  }

  pwm_.setDuty(duty);
}

uint16_t EncoderMotor::getDuty() const { return pwm_.getDuty(); }

uint16_t EncoderMotor::getMaxDuty() const { return pwm_.getMaxDuty(); }

int32_t EncoderMotor::getCount() const { return encoder_.getCount(); }

void EncoderMotor::setCount(int32_t count) { encoder_.setCount(count); }

void EncoderMotor::resetCount() { encoder_.reset(); }

} // namespace app::device