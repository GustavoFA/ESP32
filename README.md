# ESP32 

This repository is a personal study of the microcontroller ESP32 from Espressif.

The ESP32 has two cores, which are 0 (PRO_CPU) and 1 (APP_CPU). Typically, the tasks responsible for handling protocol related processing such as Wi-Fi or Bluetooth are pinned to Core 0 (thus the name PRO_CPU), where as the tasks handling the remainder of the application are pinned to Core 1, (thus the name APP_CPU). 

## ESP IDF

For the idf I'm using the version 6.0.2.

### IDF FreeRTOS

The IDF FreeRTOS source code is based on Vanilla FreeRTOS v10.5.1.

To support dual-core ESP targets, such as ESP32, ESP32-S3, and ESP32-P4, ESP-IDF provides a unique implementation of FreeRTOS with dual-core symmetric multiprocessing (SMP) capabilities. SMP is a computing architecture where two or more identical CPU cores are connected to a single shared main memory and controlled by a single operating system. The SMP system has multiple cores running independently, which each core has its own register file, interrupts, and interrupt handling; and it presents an identical view of memory to each core, thus, a piece of code that accesses a particular memory address has the same effect regardless of which core it runs on.

## Arduino IDE