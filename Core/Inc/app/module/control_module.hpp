#pragma once

#include "app/device/encoder_motor.hpp"
#include "app/device/switch.hpp"
#include "main.h"

#include <cstdint>

namespace app::module {

class ControlModule {
public:
  enum class State : uint8_t {
    IDLE = 0,
    MOVING = 1,
    READY = 2,
    WARNING_X = 3,
    WARNING_Y = 4,
    WARNING_XY = 5,
  };

  ControlModule(TIM_HandleTypeDef *x_pwm_htim, uint32_t x_pwm_channel,
                GPIO_TypeDef *x_dir_port, uint16_t x_dir_pin,
                TIM_HandleTypeDef *x_encoder_htim,
                TIM_HandleTypeDef *y_pwm_htim, uint32_t y_pwm_channel,
                GPIO_TypeDef *y_dir_port, uint16_t y_dir_pin,
                TIM_HandleTypeDef *y_encoder_htim, GPIO_TypeDef *x_warn_port,
                uint16_t x_warn_pin, GPIO_TypeDef *y_warn_port,
                uint16_t y_warn_pin);

  void start();
  void stop();

  void setTarget(int32_t x_target_cnt, int32_t y_target_cnt);
  void update();

  State getState() const;

  int32_t getXCount() const;
  int32_t getYCount() const;

private:
  app::device::EncoderMotor x_motor_;
  app::device::EncoderMotor y_motor_;

  app::device::Switch x_warn_sw_;
  app::device::Switch y_warn_sw_;

  int32_t target_x_cnt_ = 0;
  int32_t target_y_cnt_ = 0;

  State state_ = State::IDLE;

  static constexpr int32_t POSITION_TOLERANCE = 50; // 오차범위 50cnt = 1도
  static constexpr uint16_t MOVE_DUTY = 300;

  uint8_t updateAxis(app::device::EncoderMotor &motor, int32_t target_cnt);
  void stopAll();
  void updateWarningState();
};

} // namespace app::module