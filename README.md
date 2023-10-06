# MOS-STM32

#### 介绍

STM32F429上的简单RTOS，Mini-RTOS <=> MOS

MCU: STM32F429ZIT6 (256KB SRAM, 2MB FLASH)

#### 使用
```
Task::create();
Task::yield();
Task::block();
Task::resume();
```