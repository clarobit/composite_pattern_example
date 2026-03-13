#include "app/app_main.hpp"

#include <cstring>

#include "app/module/communication_module.hpp"
#include "app/module/control_module.hpp"
#include "app/module/feeder_module.hpp"
#include "app/module/shoot_module.hpp"

#include "main.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern UART_HandleTypeDef huart2;

namespace {

constexpr uint16_t COMM_FRAME_SIZE = 9U;

constexpr uint8_t FRAME_IDLE = 0U;
constexpr uint8_t FRAME_RECEIVED = 1U;
constexpr uint8_t FRAME_PROCESSING = 2U;

constexpr uint8_t DROP_IDLE = 0U;
constexpr uint8_t DROP_READY = 1U;
constexpr uint8_t DROP_RUNNING = 2U;

uint8_t comm_rx_buffer[COMM_FRAME_SIZE]{};
uint8_t tx_buf[COMM_FRAME_SIZE]{};

volatile uint8_t frame_state = FRAME_IDLE;
uint8_t drop_state = DROP_IDLE;

app::module::ControlModule control(&htim3, TIM_CHANNEL_1, GPIOB, GPIO_PIN_0,
                                   &htim4, &htim3, TIM_CHANNEL_2, GPIOB,
                                   GPIO_PIN_1, &htim4, GPIOB, GPIO_PIN_2, GPIOB,
                                   GPIO_PIN_3);

app::module::ShootModule shoot(&htim3, TIM_CHANNEL_3, GPIOB, GPIO_PIN_4, &htim3,
                               TIM_CHANNEL_4, GPIOB, GPIO_PIN_5);

app::module::FeederModule feeder(&htim5, TIM_CHANNEL_1, GPIOB, GPIO_PIN_6,
                                 GPIOB, GPIO_PIN_7);

app::module::CommunicationModule communication(&huart2, GPIOB, GPIO_PIN_8,
                                               GPIOB, GPIO_PIN_9,
                                               comm_rx_buffer, COMM_FRAME_SIZE);

void applyReceivedFrame() {
  uint8_t *rx_buf = communication.getRxBuffer();

  std::memcpy(tx_buf, rx_buf, COMM_FRAME_SIZE);

  const int16_t x_target =
      app::module::CommunicationModule::parseInt16(&rx_buf[0]);
  const int16_t y_target =
      app::module::CommunicationModule::parseInt16(&rx_buf[2]);
  const int16_t up_target =
      app::module::CommunicationModule::parseInt16(&rx_buf[4]);
  const int16_t down_target =
      app::module::CommunicationModule::parseInt16(&rx_buf[6]);

  const uint8_t drop_cmd = rx_buf[8];

  control.setTarget(static_cast<int32_t>(x_target),
                    static_cast<int32_t>(y_target));

  shoot.setTarget(up_target, down_target);

  if (drop_cmd != 0U) {
    drop_state = DROP_READY;
  } else {
    drop_state = DROP_IDLE;
    feeder.setState(app::module::FeederModule::State::IDLE);
  }

  communication.send(tx_buf, COMM_FRAME_SIZE);

  frame_state = FRAME_PROCESSING;
}

void handleFrameProcessing() {
  // control 모듈, shoot module 준비 완료 확인
  if ((control.getState() != app::module::ControlModule::State::READY) ||
      (shoot.getState() != app::module::ShootModule::State::READY)) {
    return;
  }

  // control 모듈, shoot module 준비 완료 후
  switch (drop_state) {
  // 1) drop 명령이 없을 경우 종료
  case DROP_IDLE:
    frame_state = FRAME_IDLE;
    break;

  // 2) drop 명령이 있을 경우 feeder 모듈 실행
  case DROP_READY:
    feeder.setState(app::module::FeederModule::State::RUNNING);
    drop_state = DROP_RUNNING;
    break;

  // 3) 공이 떨어지면 Idle 상태로 변경
  case DROP_RUNNING:
    if (feeder.getState() == app::module::FeederModule::State::DONE) {
      tx_buf[8] = drop_state;

      communication.send(tx_buf, COMM_FRAME_SIZE);

      feeder.setState(app::module::FeederModule::State::IDLE);

      drop_state = DROP_IDLE;
      frame_state = FRAME_IDLE;
    }
    break;

  default:
    break;
  }
}

} // namespace

extern "C" {

void app_init(void) {
  control.start();
  shoot.start();
  feeder.start();
  communication.start();

  frame_state = FRAME_IDLE;
  drop_state = DROP_IDLE;

  std::memset(comm_rx_buffer, 0, sizeof(comm_rx_buffer));
  std::memset(tx_buf, 0, sizeof(tx_buf));

  communication.startReceive();
}

void control_update(void) { control.update(); }

void shoot_update(void) { shoot.update(); }

void feeder_update(void) { feeder.update(); }

void communication_update(void) {
  if (frame_state == FRAME_RECEIVED) {
    applyReceivedFrame();

    communication.startReceive();
  }

  if (frame_state == FRAME_PROCESSING) {
    handleFrameProcessing();
  }
}

void comm_set_rx_done(void) { frame_state = FRAME_RECEIVED; }
}