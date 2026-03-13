# layered_composition_firmware

**Layered Architecture**와 **Object Composition**을 활용하여 STM32 펌웨어 구조를 설계한 예제 프로젝트

이 프로젝트는 STM32CubeIDE와 HAL 드라이버를 사용하여  
**Layered Architecture**와 **Composition**을 기반으로 임베디드 펌웨어 구조를 설계하는 방법을 보여주는 것을 목표로 합니다.

---

## 개발 환경

- MCU: STM32H723ZG (NUCLEO-H723ZG)
- IDE: STM32CubeIDE
- Language: C++
- Driver: STM32 HAL

---

## 프로젝트 목적

실제 로봇 시스템의 구조와 유사한 방식으로 펌웨어 아키텍처를 설계함.  
로봇이 여러 하드웨어 부품과 기능 모듈로 구성되듯이, 코드 또한 **MCU → Device → Module → Application** 계층 구조로 구성함

각 계층의 역할을 분리하여 시스템 구조와 코드 구조를 최대한 유사하게 유지하고  
기능 추가나 수정 시 영향 범위를 최소화하여 **유지보수성과 확장성이 좋은 펌웨어 구조를 만드는 것**을 목표로 함.

---

## 아키텍처 구조

### A. 아키텍처 개요
펌웨어는 다음과 같은 **Layered Architecture**로 구성함.

```
Application
  ↓
Module
  ↓
Device
  ↓
MCU
  ↓
HAL
  ↓
Hardware
```

각 계층은 역할에 따라 분리되어 있으며  
상위 계층은 하위 계층을 **Composition 방식으로 조합하여** 시스템 동작을 구성함.

- MCU Layer   
STM32 Peripheral을 제어하기 위한 저수준 abstraction 계층

- Device Layer   
여러 MCU Peripheral을 조합하여 실제 하드웨어 장치를 표현하는 계층

- Module Layer   
Device를 조합하여 기능 단위 동작을 구현하는 계층

- Application Layer   
전체 Module을 연결하고 main loop와 interrupt를 통해 시스템을 실행하는 계층

HAL 아래 계층은 STM32 HAL 드라이버와 실제 하드웨어를 의미하며 MCU Layer를 통해서만 간접적으로 접근하도록 설계함.

이 구조를 통해 상위 계층은 하위 계층의 구현 세부사항에 직접 의존하지 않도록 구성함.

### B. 아키텍처 구조의 장단점
#### 장점
- **코드 의존성이 명확함**  
계층 구조를 통해 각 레이어의 기능과 의존 관계가 명확해져 개발 단계 구분이 쉬움
- **협업이 쉬움**  
레이어별 역할이 분리되어 있어 동시 개발이 용이함
- **재사용성이 높음**  
각 계층이 분리되어 있어 중복되는 기능(MCU)이나 부품(Device)을  
프로젝트 내부에서 재사용할 수 있으며 다른 프로젝트에서도 쉽게 활용 가능

#### 단점
- **코드 양이 증가함**  
단순한 기능도 여러 레이어를 거쳐 구현되어 코드가 늘어날 수 있음
- **MCU 추상화 단위가 크면 불필요한 기능까지 포함될 수 있음**  
  예를 들어 MCU 클래스가 A, B, C 기능을 모두 제공할 때  
  어떤 Device는 A 기능만 필요하더라도 A, B, C 전체를 포함한 객체에 의존하게 될 수 있음
- **하위 계층 수정 시 의존성을 가진 상위 계층을 전부 수정이 필요할 수 있음**  
하위 계층 변경 시 이를 사용하는 상위 계층 수정이 필요할 수 있음

---


## 레이어 구성

각 계층은 실제 프로젝트 코드에서 다음과 같은 구조로 구현되어 있음.

```
Application Layer
 └ app_main

Module Layer
 ├ CommunicationModule
 ├ ControlModule
 ├ FeederModule
 └ ShootModule

Device Layer
 ├ Bluetooth
 ├ EncoderMotor
 ├ Motor
 └ Switch

MCU Layer
 ├ Encoder
 ├ Gpio
 ├ Pwm
 └ Uart
```

---

### A. MCU Layer

MCU의 Peripheral을 제어하기 위한 **저수준 abstraction 계층**.

STM32 HAL을 직접 사용하는 코드를 이 계층으로 제한하여 상위 계층(Device, Module)이 MCU에 직접 의존하지 않도록 구성함

예를 들어 GPIO, PWM, UART, ADC 등의 MCU 기능을 클래스 형태로 감싸는 wrapper를 구현하여 사용함

이를 통해 상위 계층에서는 HAL API를 직접 사용하지 않고 MCU 기능을 추상화된 인터페이스를 통해 사용할 수 있도록 함

예:

`app/mcu/gpio`

자세한 내용은 README_1_MCU.md 참고

---

### B. Device Layer

실제 하드웨어 장치를 표현하기 위한 **장치 단위 abstraction 계층**.

MCU Layer에서 제공하는 Peripheral wrapper들을 조합하여  
모터, 스위치, 통신 장치와 같은 실제 하드웨어 장치를 표현하도록 구성함.

예를 들어 모터는 PWM과 방향 제어용 GPIO를 함께 사용하여 동작하며,  
블루투스 모듈은 UART 통신과 GPIO 기반 전원 제어 및 상태 확인 기능을 함께 사용함.

이처럼 여러 MCU 기능을 조합하여 하나의 장치를 표현하고  
상위 계층(Module)이 장치 단위 인터페이스를 통해 하드웨어를 제어할 수 있도록 함.

예:

`app/device/motor`

자세한 내용은 README_2_DEVICE.md 참고

---

### C. Module Layer

기능 단위 동작을 구현하기 위한 상위 로직 계층.

Device Layer에서 제공하는 Motor, EncoderMotor, Switch, Bluetooth 등의 장치를 조합하여 실제 시스템의 동작 목적에 맞는 기능을 구현하도록 구성함.

예를 들어 위치 제어, 발사 모터 제어, 공 공급, 통신 처리와 같은 기능을
각각 독립적인 Module로 분리하여 설계함.

이 계층에서는 상태 기반 제어, 목표값 기반 제어, 모듈 간 연동과 같은
상위 수준의 firmware logic을 담당함.

예:

`app/module/control_module`

자세한 내용은 README_3_MODULE.md 참고

---

### D. Application Layer

Application Layer는 전체 firmware 동작을 실행하고 연결하는 **최상위 계층**.

STM32CubeIDE에서 생성되는 main.c는 C 기반 코드이므로
C++로 구현된 Module 객체를 직접 다루기 어려움.

이를 해결하기 위해 Application Layer에서
Module 객체 생성, 초기화, update 호출, UART callback 연동과 같은
C ↔ C++ bridge 역할을 수행하도록 구성함.

즉 main.c는 하드웨어 초기화와 main loop를 담당하고,
실제 firmware 동작 연결은 Application Layer를 통해 수행됨.

예:

`app/app_main`

자세한 내용은 README_4_APP_MAIN.md 참고