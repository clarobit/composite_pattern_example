# Module Layer

Module Layer는 Device Layer를 조합하여 **기능 단위 로직을 구현하는 계층**.

이 계층에서는 HAL API를 직접 호출하지 않고
Motor, EncoderMotor, Switch, Bluetooth와 같은 Device를 조합하여
실제 동작 목적에 맞는 제어 로직을 구현함.

Module Layer는 단순한 장치 제어가 아니라
상태 기반 동작, 목표값 추적, 시퀀스 처리, 모듈 간 연동과 같은
상위 수준의 firmware logic을 담당함.

## Module Summary

현재 Module Layer는 다음과 같은 역할 분리를 가짐.

- **CommunicationModule**  
Bluetooth 통신 및 frame buffer 관리

- **ControlModule**  
x, y 위치 제어

- **FeederModule**  
공 배급 및 공 인식

- **ShootModule**  
발사 모터 속도 제어

---

## 1. CommunicationModule

CommunicationModule은 Bluetooth 통신을 처리하는 Module.  

- Bluetooth device를 사용하여 고정 길이 frame을 수신하고 데이터 송신 인터페이스를 제공함.
- 또한 2바이트 데이터를 int16_t로 변환하는 parsing utility를 포함함.

### Constructor

```cpp
CommunicationModule(
  UART_HandleTypeDef *huart, GPIO_TypeDef *bt_power_port,
  uint16_t bt_power_pin, GPIO_TypeDef *bt_state_port,
  uint16_t bt_state_pin, uint8_t *rx_buffer, uint16_t frame_size
);
```

Bluetooth 통신에 필요한 UART, 전원 GPIO, 연결 상태 GPIO와  
RX buffer와 frame size를 전달받아 객체를 생성함.

### Functions

#### start

```cpp
void start();
```

Bluetooth 전원을 켜고 내부 통신 상태를 초기화함.

#### stop

```cpp
void stop();
```

Bluetooth 전원을 끄고 내부 통신 상태를 초기화함.

#### startReceive

```cpp
HAL_StatusTypeDef startReceive();
```

UART interrupt 방식으로 frame_size 길이만큼 수신을 시작함.

#### send

```cpp
HAL_StatusTypeDef send(const uint8_t *data, uint16_t len);
```

Bluetooth device를 사용하여 데이터를 송신함.

#### getRxBuffer

```cpp
uint8_t *getRxBuffer();
```

UART interrupt 수신에 사용할 RX buffer 주소를 반환함.

#### getFrameSize

```cpp
uint16_t getFrameSize() const;
```

현재 CommunicationModule이 사용하는 frame size를 반환함.

#### parseInt16

```cpp
static int16_t parseInt16(const uint8_t *data);
```

2바이트 데이터를 big-endian 기준으로 묶어 int16_t 값으로 변환함.

예

```cpp
data[0] = high byte
data[1] = low byte
```

## 2. ControlModule

ControlModule은 x축, y축 위치 제어를 담당하는 Module.

- 두 개의 EncoderMotor를 사용하여 외부에서 설정한 target count까지 모터를 이동시킴.
- 각 축의 warning switch를 확인하여 위험 상태가 감지되면 전체 모터를 정지하고 warning state로 전환함.

---

### Constructor

```cpp
ControlModule(
  TIM_HandleTypeDef *x_pwm_htim, uint32_t x_pwm_channel,
  GPIO_TypeDef *x_dir_port, uint16_t x_dir_pin,
  TIM_HandleTypeDef *x_encoder_htim,
  TIM_HandleTypeDef *y_pwm_htim, uint32_t y_pwm_channel,
  GPIO_TypeDef *y_dir_port, uint16_t y_dir_pin,
  TIM_HandleTypeDef *y_encoder_htim,
  GPIO_TypeDef *x_warn_port, uint16_t x_warn_pin,
  GPIO_TypeDef *y_warn_port, uint16_t y_warn_pin
);
```

x축, y축 EncoderMotor와 각 축의 warning switch를 사용하는 객체를 생성함.

---

### State

```cpp
enum class State : uint8_t {
  IDLE = 0,
  MOVING = 1,
  READY = 2,
  WARNING_X = 3,
  WARNING_Y = 4,
  WARNING_XY = 5,
};
```

**IDLE**  
정지 상태.

**MOVING**  
x축, y축이 target count를 향해 이동 중인 상태.

**READY**  
x축, y축이 모두 목표 위치에 도달한 상태.

**WARNING_X**  
x축 warning switch가 눌린 상태.

**WARNING_Y**  
y축 warning switch가 눌린 상태.

**WARNING_XY**  
x축, y축 warning switch가 모두 눌린 상태.

---

### Functions

#### start

```cpp
void start();
```

x축, y축 EncoderMotor를 시작하고 duty를 0으로 초기화함.  
현재 encoder count를 읽어 target count 초기값으로 사용하며 state를 IDLE로 설정함.

#### stop

```cpp
void stop();
```

x축, y축 모터 duty를 0으로 설정한 뒤 모터를 정지하고 target count를 0으로 초기화하며 state를 IDLE로 설정함.

#### setTarget

```cpp
void setTarget(int32_t x_target_cnt, int32_t y_target_cnt);
```

x축, y축의 목표 count를 설정함.

warning switch가 눌린 상태가 아니라면 state를 MOVING으로 변경함.

#### update

```cpp
void update();
```

warning switch 상태를 먼저 확인함.  
warning이 감지되면 전체 모터를 정지하고 warning state로 전환함.

warning 상태가 아니고 state가 MOVING일 때,  
현재 encoder count를 읽어 target과 비교하고  
오차가 허용 범위보다 크면 방향을 설정한 뒤 이동시킴.

x축과 y축이 모두 목표 위치에 도달하면 state를 READY로 변경함.

#### getState

```cpp
State getState() const;
```

현재 ControlModule state를 반환함.

#### getXCount

```cpp
int32_t getXCount() const;
```

현재 x축 encoder count를 반환함.

#### getYCount

```cpp
int32_t getYCount() const;
```

현재 y축 encoder count를 반환함.

---

## 3. FeederModule

FeederModule은 공을 공급하는 Module.

- Motor 1개와 ball detect switch 1개를 사용함.
- 외부에서 state를 RUNNING으로 변경하면 feeder motor를 구동함.
- 공이 떨어져 switch가 눌리면 모터를 정지하고 state를 DONE으로 변경함.

### Constructor

```cpp
FeederModule(
  TIM_HandleTypeDef *htim, uint32_t channel,
  GPIO_TypeDef *dir_port, uint16_t dir_pin,
  GPIO_TypeDef *sw_port, uint16_t sw_pin
);
```

feeder motor와 ball detect switch를 사용하는 객체를 생성함.

### State

```cpp
enum class State : uint8_t {
  IDLE = 0,
  RUNNING = 1,
  DONE = 2,
};
```

**IDLE**  
대기 상태. 모터는 정지 상태를 유지함.

**RUNNING**  
공급 동작이 진행 중인 상태.  
모터를 정방향으로 구동함.

**DONE**  
공 1개 공급이 완료된 상태.  
모터를 정지한 상태를 유지함.

### Functions

#### start

```cpp
void start();
```

feeder motor를 시작하고 duty를 0으로 초기화한 뒤 state를 IDLE로 설정함.

#### stop

```cpp
void stop();
```

feeder motor duty를 0으로 설정한 뒤 모터를 정지하고 state를 IDLE로 설정함.

#### setState

```cpp
void setState(State state);
```

FeederModule state를 외부에서 설정함.

#### getState

```cpp
State getState() const;
```

현재 FeederModule state를 반환함.

#### update

```cpp
void update();
```

현재 state에 따라 feeder motor 동작을 수행함.

IDLE 상태에서는 duty를 0으로 유지함.  
RUNNING 상태에서는 motor를 정방향으로 구동함.  
switch가 눌리면 motor를 정지하고 state를 DONE으로 변경함.  
DONE 상태에서는 motor를 정지 상태로 유지함.

---

## 4. ShootModule

ShootModule은 발사부의 상/하 모터 2개를 제어하는 Module.

- 두 개의 Motor device를 사용하여 상단과 하단 모터를 제어함.
- 외부에서 설정한 목표 duty까지 motor duty 값을 점진적으로 변경함.

---

### Constructor

```cpp
ShootModule(
  TIM_HandleTypeDef *up_htim, uint32_t up_channel,
  GPIO_TypeDef *up_dir_port, uint16_t up_dir_pin,
  TIM_HandleTypeDef *down_htim, uint32_t down_channel,
  GPIO_TypeDef *down_dir_port, uint16_t down_dir_pin
);
```

상단 모터와 하단 모터를 제어하기 위한 PWM Timer, Channel, 방향 제어 GPIO를 사용하는 객체를 생성함.

---

### State

```cpp
enum class State : uint8_t {
  IDLE = 0,
  RAMPING = 1,
  READY = 2,
};
```

**IDLE**  
정지 상태.

**RAMPING**  
목표 duty까지 현재 duty를 단계적으로 증가 또는 감소시키는 상태.

**READY**  
목표 duty에 도달한 상태.

---

### Functions

#### start

```cpp
void start();
```

두 개의 모터를 시작하고 duty를 0으로 초기화함.  
현재 duty와 target duty도 모두 0으로 설정하며 state를 IDLE로 초기화함.

#### stop

```cpp
void stop();
```

두 개의 모터를 정지하고 target duty와 current duty를 모두 0으로 초기화한 뒤 state를 IDLE로 설정함.

#### setTarget

```cpp
void setTarget(int16_t up_duty, int16_t down_duty);
```

상단 모터와 하단 모터의 목표 duty를 설정함.

입력값은 각 모터의 최대 duty 범위를 초과하지 않도록 제한되며
target이 설정되면 state는 RAMPING으로 변경됨.

#### update

```cpp
void update();
```

현재 state가 RAMPING일 때 동작하며
각 모터의 현재 duty를 STEP 단위로 target 방향으로 이동시킴.

두 모터가 모두 target에 도달하면 state를 READY로 변경함.

#### getState

```cpp
State getState() const;
```

현재 ShootModule state를 반환함.

---