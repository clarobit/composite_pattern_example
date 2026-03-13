#include "app/module/control_module.hpp"

namespace app::module {

ControlModule::ControlModule(
    TIM_HandleTypeDef *x_pwm_htim, uint32_t x_pwm_channel,
    GPIO_TypeDef *x_dir_port, uint16_t x_dir_pin,
    TIM_HandleTypeDef *x_encoder_htim, TIM_HandleTypeDef *y_pwm_htim,
    uint32_t y_pwm_channel, GPIO_TypeDef *y_dir_port, uint16_t y_dir_pin,
    TIM_HandleTypeDef *y_encoder_htim, GPIO_TypeDef *x_warn_port,
    uint16_t x_warn_pin, GPIO_TypeDef *y_warn_port, uint16_t y_warn_pin)
    : x_motor_(x_pwm_htim, x_pwm_channel, x_dir_port, x_dir_pin,
               x_encoder_htim),
      y_motor_(y_pwm_htim, y_pwm_channel, y_dir_port, y_dir_pin,
               y_encoder_htim),
      x_warn_sw_(x_warn_port, x_warn_pin), y_warn_sw_(y_warn_port, y_warn_pin) {
}

void ControlModule::start() {
  x_motor_.start();
  y_motor_.start();

  x_motor_.setDuty(0);
  y_motor_.setDuty(0);

  target_x_cnt_ = x_motor_.getCount();
  target_y_cnt_ = y_motor_.getCount();

  state_ = State::IDLE;
}

void ControlModule::stop() {
  x_motor_.setDuty(0);
  y_motor_.setDuty(0);

  x_motor_.stop();
  y_motor_.stop();

  target_x_cnt_ = 0;
  target_y_cnt_ = 0;

  state_ = State::IDLE;
}

void ControlModule::setTarget(int32_t x_target_cnt, int32_t y_target_cnt) {
  target_x_cnt_ = x_target_cnt;
  target_y_cnt_ = y_target_cnt;

  state_ = State::MOVING;
}

void ControlModule::update() {
  if (x_warn_sw_.isPressed() || y_warn_sw_.isPressed()) {
    stopAll();
    updateWarningState();
    return;
  }

  if (state_ != State::MOVING) {
    return;
  }

  const uint8_t x_done = updateAxis(x_motor_, target_x_cnt_);
  const uint8_t y_done = updateAxis(y_motor_, target_y_cnt_);

  if ((x_done != 0U) && (y_done != 0U)) {
    state_ = State::READY;
  }
}

uint8_t ControlModule::updateAxis(app::device::EncoderMotor &motor,
                                  int32_t target_cnt) {
  const int32_t current_cnt = motor.getCount();
  const int32_t error = target_cnt - current_cnt;

  if ((error >= -POSITION_TOLERANCE) && (error <= POSITION_TOLERANCE)) {
    motor.setDuty(0);
    return 1U;
  }

  if (error > 0) {
    motor.forward();
  } else {
    motor.reverse();
  }

  motor.setDuty(MOVE_DUTY);
  return 0U;
}

void ControlModule::stopAll() {
  x_motor_.setDuty(0);
  y_motor_.setDuty(0);
}

void ControlModule::updateWarningState() {
  const uint8_t x_warn = x_warn_sw_.isPressed();
  const uint8_t y_warn = y_warn_sw_.isPressed();

  if ((x_warn != 0U) && (y_warn != 0U)) {
    state_ = State::WARNING_XY;
  } else if (x_warn != 0U) {
    state_ = State::WARNING_X;
  } else if (y_warn != 0U) {
    state_ = State::WARNING_Y;
  }
}

ControlModule::State ControlModule::getState() const { return state_; }

int32_t ControlModule::getXCount() const { return x_motor_.getCount(); }

int32_t ControlModule::getYCount() const { return y_motor_.getCount(); }

} // namespace app::module