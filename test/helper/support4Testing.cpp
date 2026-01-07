// dummy class/struct code

#include "pch.h"
#include "support4Testing.h"
#include "support4Testing.hpp"


DummySerial Serial;
DummySerial SerialUSB;

NVIC_Type nvic;
NVIC_Type* NVIC = &nvic;

Pio pioa;
Pio* PIOA = &pioa;

Pio piob;
Pio* PIOB = &piob;

Pio pioc;
Pio* PIOC = &pioc;

Pio piod;
Pio* PIOD = &piod;

Tc tc0;
Tc* TC0 = &tc0;

Tc tc1;
Tc* TC1 = &tc1;

Tc tc2;
Tc* TC2 = &tc2;

Tc tc3;
Tc* TC3 = &tc3;

Tc tc4;
Tc* TC4 = &tc4;

Tc tc5;
Tc* TC5 = &tc5;

Tc tc6;
Tc* TC6 = &tc6;

Spi spi0;
Spi* SPI0 = &spi0;

Dmac dmac0;
Dmac* DMAC = &dmac0;

Usart usart1;
Usart* USART1 = &usart1;

Usart usart2;
Usart* USART2 = &usart2;

Usart usart3;
Usart* USART3 = &usart3;

Usart usart4;
Usart* USART4 = &usart4;

Wdt wdt;
Wdt* WDT = &wdt;

void __asm__(const char* ch){}

uint32_t pmc_enable_periph_clk(uint32_t ul_id) { return 0; }

void delay(uint32_t dwMs) {}

void NVIC_SetPriority(IRQn_Type irq, uint8_t prio) {}
void NVIC_EnableIRQ(IRQn_Type irq) {}
void NVIC_DisableIRQ(IRQn_Type irq) {}
void NVIC_ClearPendingIRQ(IRQn_Type irq) {}
void NVIC_SetPendingIRQ(IRQn_Type IRQn) {}