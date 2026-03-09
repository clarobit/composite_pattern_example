# MCU Layer

MCU Layer는 STM32 Peripheral을 제어하기 위한 **저수준 abstraction 계층**.

STM32 HAL API를 직접 사용하는 코드를 이 계층으로 제한하여
상위 계층(Device, Module)이 HAL이나 MCU의 세부 구현에 직접 의존하지 않도록 구성함.

GPIO, PWM, UART, Encoder 등의 MCU 기능을
C++ 클래스 형태의 wrapper로 구현하여 사용함.

---

## Encoder

Timer의 Encoder mode를 사용하여 엔코더 카운트를 제어하고 읽기 위한 클래스.

### Constructor

```cpp
Encoder(TIM_HandleTypeDef* htim);
```

Encoder mode로 설정된 Timer를 사용하는 객체를 생성함.

### Functions

#### start

```cpp
void start();
```

Encoder 카운트를 시작함.

#### stop

```cpp
void stop();
```

Encoder 카운트를 정지함.

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

#### reset

```cpp
void reset();
```

Encoder count 값을 0으로 초기화함.

---

## GpioOut

GPIO 출력을 제어하기 위한 클래스.

### Constructor

```cpp
GpioOut(GPIO_TypeDef *port, uint16_t pin);
```

지정한 GPIO 포트와 핀을 사용하는 출력 객체를 생성함.

### Functions

#### set

```cpp
void set();
```

GPIO 출력을 High로 설정함.

#### reset

```cpp
void reset();
```

GPIO 출력을 Low로 설정함.

#### toggle

```cpp
void toggle();
```

GPIO 출력 상태를 반전함.

---

## GpioIn

GPIO 입력 상태를 읽기 위한 클래스.

### Constructor

```cpp
GpioIn(GPIO_TypeDef *port, uint16_t pin);
```

지정한 GPIO 포트와 핀을 사용하는 입력 객체를 생성함.

### Functions

#### read

```cpp
bool read() const;
```

GPIO 입력 상태를 읽어 반환함.

true : High
false : Low

---

## Pwm

Timer PWM 출력을 제어하기 위한 클래스.

### Constructor

```cpp
Pwm(TIM_HandleTypeDef *htim, uint32_t channel);
```

지정한 Timer와 Channel을 사용하는 PWM 객체를 생성함.

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

PWM 출력을 정지함.

#### setDuty

```cpp
void setDuty(uint16_t duty);
```

PWM duty 값을 설정함.

#### getDuty

```cpp
uint16_t getDuty() const;
```

현재 PWM duty 값을 반환함.

#### getMaxDuty

```cpp
uint16_t getMaxDuty() const;
```

Timer period 기준 최대 duty 값을 반환함.

---

## Uart

UART 통신을 위한 클래스.

### Constructor

```cpp
explicit Uart(UART_HandleTypeDef *huart);
```

지정한 UART handle을 사용하는 객체를 생성함.

### Functions

#### transmit

```cpp
HAL_StatusTypeDef transmit(const uint8_t *data, uint16_t len, uint32_t timeout_ms);
```

데이터를 blocking 방식으로 송신함.

#### receiveIt

```cpp
HAL_StatusTypeDef receiveIt(uint8_t *data, uint16_t len);
```

Interrupt 기반으로 데이터를 수신함.

#### handle

```cpp
UART_HandleTypeDef *handle() const;
```

내부 UART handle을 반환함.
