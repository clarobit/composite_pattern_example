# Device Layer

Device Layer는 여러 MCU Peripheral을 조합하여 **하드웨어 장치를 추상화하는 계층**.

MCU Layer에서 제공하는 GPIO, PWM, UART, Encoder 등의 wrapper를 조합하여
모터, 스위치, 블루투스와 같은 실제 장치를 제어할 수 있도록 구성함.

이 계층을 통해 상위 계층(Module)은
개별 MCU Peripheral이 아니라 **의미 있는 장치 단위(Device)**로 하드웨어를 제어할 수 있음.

---

## Motor

PWM 출력과 방향 제어 GPIO를 사용하여 DC Motor를 제어하기 위한 클래스.

### Constructor

```cpp
Motor(TIM_HandleTypeDef *htim, uint32_t channel, GPIO_TypeDef *dir_port, uint16_t dir_pin);
```

PWM 출력을 위한 Timer와 Channel,
Motor 방향 제어를 위한 GPIO를 사용하는 객체를 생성함.

### Functions

#### start

```cpp
void start();
```

PWM 출력을 시작함.

#### stop

```cpp
void stop();
```

PWM duty를 0으로 설정한 뒤 PWM 출력을 정지함.

#### forward

```cpp
void forward();
```

Motor 방향을 정방향으로 설정함.

#### reverse

```cpp
void reverse();
```

Motor 방향을 역방향으로 설정함.

#### setDuty

```cpp
void setDuty(uint16_t duty);
```

PWM duty 값을 설정함.

설정 가능한 최대 duty 값을 초과할 경우 자동으로 최대값으로 제한됨.

#### getMaxDuty

```cpp
uint16_t getMaxDuty() const;
```

Timer period 기준 최대 PWM duty 값을 반환함.

---

## EncoderMotor

PWM 기반 모터 제어와 encoder count 측정을 함께 사용하는 모터 클래스.

PWM과 방향 제어 GPIO를 사용하여 모터를 구동하고
Timer Encoder mode를 사용하여 모터 회전량을 측정함.

### Constructor

```cpp
EncoderMotor(
    TIM_HandleTypeDef *pwm_htim,
    uint32_t pwm_channel,
    GPIO_TypeDef *dir_port,
    uint16_t dir_pin,
    TIM_HandleTypeDef *encoder_htim
);
```

PWM 제어용 Timer와 Channel,
방향 제어 GPIO,
Encoder mode로 설정된 Timer를 사용하는 객체를 생성함.

### Functions

#### start

```cpp
void start();
```

PWM 출력과 encoder count를 동시에 시작함.

#### stop

```cpp
void stop();
```

PWM duty를 0으로 설정하고 PWM 출력을 정지하며
encoder count도 함께 정지함.

#### forward

```cpp
void forward();
```

Motor 방향을 정방향으로 설정함.

#### reverse

```cpp
void reverse();
```

Motor 방향을 역방향으로 설정함.

#### setDuty

```cpp
void setDuty(uint16_t duty);
```

PWM duty 값을 설정함.

설정 가능한 최대 duty 값을 초과할 경우 자동으로 최대값으로 제한됨.

#### getMaxDuty

```cpp
uint16_t getMaxDuty() const;
```

Timer period 기준 최대 PWM duty 값을 반환함.

#### getCount

```cpp
int32_t getCount() const;
```

현재 encoder count 값을 반환함.

#### setCount

```cpp
void setCount(int32_t count);
```

Encoder count 값을 설정함.

#### resetCount

```cpp
void resetCount();
```

Encoder count 값을 0으로 초기화함.

---

## Switch

GPIO 입력을 사용하여 스위치 상태를 확인하기 위한 클래스.

### Constructor

```cpp
Switch(GPIO_TypeDef *port, uint16_t pin);
```

지정한 GPIO 포트와 핀을 사용하는 스위치 객체를 생성함.

### Functions

#### isPressed

```cpp
bool isPressed() const;
```

스위치 입력 상태를 읽어 반환함.

```
true  : pressed
false : released
```

---

## Bluetooth

UART 통신을 기반으로 하는 Bluetooth 모듈을 제어하기 위한 클래스.

UART 통신과 함께
Bluetooth 모듈의 전원 제어와 연결 상태 확인을 위한 GPIO를 사용함.

### Constructor

```cpp
Bluetooth(
    app::mcu::Uart uart,
    app::mcu::GpioOut power_pin,
    app::mcu::GpioIn state_pin
);
```

UART 통신 객체와
Bluetooth 전원 제어 GPIO,
연결 상태 확인 GPIO를 사용하는 객체를 생성함.

### Functions

#### powerOn

```cpp
void powerOn();
```

Bluetooth 모듈의 전원을 켬.

#### powerOff

```cpp
void powerOff();
```

Bluetooth 모듈의 전원을 끔.

#### isConnected

```cpp
bool isConnected() const;
```

Bluetooth 연결 상태를 반환함.

```
true  : connected
false : disconnected
```

#### send

```cpp
HAL_StatusTypeDef send(const uint8_t *data, uint16_t len, uint32_t timeout_ms);
```

UART를 통해 데이터를 blocking 방식으로 송신함.

#### receiveIt

```cpp
HAL_StatusTypeDef receiveIt(uint8_t *data, uint16_t len);
```

UART interrupt 기반으로 데이터를 수신함.
