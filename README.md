# composite_pattern_example

STM32 기반 임베디드 시스템에서 **C++과 Composite Pattern을 활용한 펌웨어 구조 예제**

이 프로젝트는 STM32CubeIDE와 HAL 드라이버를 사용하여, 임베디드 펌웨어를 **계층 구조(Layered Architecture)**로 설계하고 **Composite Pattern을 적용하는 방법**을 보여주는 것을 목표로 합니다.

---

# 개발 환경

- MCU: STM32H723ZG (NUCLEO-H723ZG)
- IDE: STM32CubeIDE
- Language: C++
- Driver: STM32 HAL

---

# 프로젝트 목적

이 프로젝트에서는 실제 로봇 시스템의 구조와 유사한 방식으로 펌웨어 아키텍처를 설계함  
로봇이 여러 하드웨어 부품과 기능 모듈로 구성되듯이, 코드 또한 **MCU → Device → Module** 계층 구조로 구성함

각 계층의 역할을 분리하여 시스템 구조와 코드 구조를 최대한 유사하게 유지하고,  
기능 추가나 수정 시 영향 범위를 최소화하여 **유지보수성과 확장성이 좋은 펌웨어 구조를 만드는 것**을 목표로 함


---

# 아키텍처 구조

펌웨어는 다음과 같은 계층 구조로 구성함.

```
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

이 중 MCU, Device, Module 계층은 실제 시스템 구조와 유사하도록 설계한 펌웨어 레이어  
HAL 아래는 STM32 HAL 드라이버와 실제 하드웨어 계층을 의미함

---

## MCU Layer

MCU의 Peripheral을 제어하기 위한 **저수준 abstraction 계층**.

STM32 HAL을 직접 사용하는 코드를 이 계층으로 제한하여  
상위 계층(Device, Module)이 MCU에 직접 의존하지 않도록 구성함

예를 들어 GPIO, PWM, UART, ADC 등의 MCU 기능을  
클래스 형태로 감싸는 wrapper를 구현하여 사용함

이를 통해 상위 계층에서는 HAL API를 직접 사용하지 않고  
MCU 기능을 추상화된 인터페이스를 통해 사용할 수 있도록 함

예:

`app/mcu/gpio`

---

## Device Layer

실제 하드웨어 장치를 표현하기 위한 **장치 단위 abstraction 계층**.

MCU Layer에서 제공하는 Peripheral wrapper들을 조합하여  
모터, 스위치, 통신 장치와 같은 실제 하드웨어 장치를 표현하도록 구성함.

예를 들어 모터는 PWM과 방향 제어용 GPIO를 함께 사용하여 동작하며,  
블루투스 모듈은 UART 통신과 GPIO 기반 전원 제어 및 상태 확인 기능을 함께 사용함.

이처럼 여러 MCU 기능을 조합하여 하나의 장치를 표현하고  
상위 계층(Module)이 장치 단위 인터페이스를 통해 하드웨어를 제어할 수 있도록 함.

예:

`app/device/motor`

---

## Module Layer

(작성 예정)