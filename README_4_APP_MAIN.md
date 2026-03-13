# Application Layer

Application Layer는 전체 firmware의 **최상위 실행 계층**.

이 계층은 STM32Cube에서 생성되는 `main.c`와  
C++ 기반 Module Layer 사이의 **연결 역할**을 수행함.

또한 Module Layer를 조합하여  
시스템 전체 동작 흐름을 **관리하고 감독하는 역할**을 담당함.

Application Layer에서는 아래 동작을 수행함

- Module 객체 생성 및 초기화
- 통신 frame parsing 및 Module 제어
- Module 동작 흐름 관리

---

## 주요 역할

- C 기반 `main.c`와 C++ Module Layer 연결
- Module 객체 생성 및 초기화
- 통신 frame parsing 및 Module 제어
- Module 동작 흐름 관리

## 설계 목적

- `main.c`는 최소한의 코드만 유지
- Module Layer는 독립적인 C++ 로직 유지
- firmware 계층 구조 분리

## 동작 개요

1) UART를 통해 제어 frame을 수신
2) Application Layer에서 frame을 parsing
3) parsing된 target 값을 각 Module에 전달
4) main loop에서 Module update가 반복 실행됨
5) 동작 완료 시 통신을 통해 상태를 응답

---

## Application Layer 기능

### 1. Layered Composition

Application Layer에서는 Module Layer의 객체들을 생성하고 조합하여 전체 시스템 동작을 구성함.

이 구조는 **Layered Architecture와 Composition 구조**를 기반으로 설계됨.

```cpp
namespace {

app::module::ControlModule control(...);

app::module::ShootModule shoot(...);

app::module::FeederModule feeder(...);

app::module::CommunicationModule communication(...);

}
```

각 Module 객체는 namespace 내부 static 영역에 생성되어 프로그램 전체에서 하나의 instance로 사용됨.

Application Layer는 이러한 Module 객체들을 조합하여 시스템 동작 흐름을 구성함.

---

### 2. C / C++ Bridge Interface

STM32Cube에서 생성되는 `main.c`는 C 코드이므로 C++ 클래스 객체를 직접 사용할 수 없음.

따라서 Application Layer에서는 `extern "C"` 인터페이스를 사용하여 C에서 호출 가능한 함수들을 제공함.

예

```cpp
#ifdef __cplusplus
extern "C" {
#endif

void example_init(void);
void example_update(void);
void example_interrupt(void);

#ifdef __cplusplus
}
#endif
```

이 인터페이스를 통해 `main.c`는 Application Layer의 함수만 호출하여 전체 시스템을 실행할 수 있음.

---

## Internal State

Application Layer에서는 통신 처리와 feeder 동작 흐름을 관리하기 위해 내부 상태 변수를 사용함.

### Frame State

통신 frame의 수신 및 처리 상태를 관리함.

```cpp
FRAME_IDLE
FRAME_RECEIVED
FRAME_PROCESSING
```

- FRAME_IDLE : 새로운 frame을 기다리는 상태
- FRAME_RECEIVED : UART interrupt로 frame 수신이 완료된 상태
- FRAME_PROCESSING : 수신된 frame을 처리하는 상태

### Drop State

feeder 동작 흐름을 관리하기 위한 상태.

```cpp
DROP_IDLE
DROP_READY
DROP_RUNNING
```

- DROP_IDLE : drop 동작이 없는 대기 상태
- DROP_READY : drop 요청이 발생한 상태
- DROP_RUNNING : feeder가 동작 중인 상태

---

## System Execution Flow

Application Layer에서는 main loop와 interrupt를 기반으로 시스템 동작이 실행됨.

### System Flow

전체 firmware 실행 흐름은 다음과 같음.

```cpp
main.c
↓
app_init()

main loop
↓
control_update()
shoot_update()
feeder_update()
communication_update()

UART interrupt
↓
comm_set_rx_done()
```

- **app_init()** : Application Layer 초기화 및 모든 Module 시작
- **control_update()** : ControlModule 상태 업데이트 및 위치 제어 수행
- **shoot_update()** : ShootModule 상태 업데이트 및 모터 속도 제어 수행
- **feeder_update()** : FeederModule 상태 업데이트 및 공 공급 동작 수행
- **communication_update()** : 통신 frame 처리 및 시스템 동작 흐름 관리
- **comm_set_rx_done()** : 새로운 통신 frame 수신 완료 상태 설정


### Frame Processing Flow

새로운 통신 frame이 수신되면 Application Layer에서 다음 순서로 처리됨.

```cpp
UART interrupt
↓
comm_set_rx_done()
↓
communication_update()
↓
applyReceivedFrame()
↓
handleFrameProcessing()
```

- **UART interrupt** : UART 수신 완료
- **comm_set_rx_done()** : frame 수신 완료 표시
- **communication_update()** : frame 처리 상태 확인
- **applyReceivedFrame()** : frame parsing 및 명령 적용
- **handleFrameProcessing()** : 시스템 동작 흐름 관리

---

## Functions

Application Layer에서는 다음 함수들을 통해  
Module update와 frame 처리를 수행함.

### app_init

```cpp
void app_init(void);
```

모든 Module을 초기화하고  
시스템 상태를 초기 상태로 설정함.

초기화 과정에서 다음을 수행함.

- ControlModule start
- ShootModule start
- FeederModule start
- CommunicationModule start
- RX buffer 초기화
- UART receive 시작


### control_update

```cpp
void control_update(void);
```

ControlModule의 update 함수를 호출함.

### shoot_update

```cpp
void shoot_update(void);
```

ShootModule의 update 함수를 호출함.

### feeder_update

```cpp
void feeder_update(void);
```

FeederModule의 update 함수를 호출함.

### communication_update

```cpp
void communication_update(void);
```

통신 frame 상태를 확인하여 frame 처리 및 시스템 동작을 수행함.

- 새로운 frame 수신 시 `applyReceivedFrame()` 실행
- frame 처리 상태일 때 `handleFrameProcessing()` 실행

### comm_set_rx_done

```cpp
void comm_set_rx_done(void);
```

UART interrupt에서 호출되는 함수.

새로운 통신 frame이 수신되었음을 Application Layer에 전달함.

---