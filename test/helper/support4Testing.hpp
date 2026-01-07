// dummy classes for testing

#ifndef SUPPPORT_4_TESTING_HPP
#define SUPPPORT_4_TESTING_HPP

#include "stdint.h"
#include <string>

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class DummySerial
{
public:
    //DummySerial(){}

    void begin(const uint32_t dwBaudRate) {}
    void end(void) {}
	int available(void) { return 0; }
    int availableForWrite(void) {}
    int peek(void) {}
	int read(void) { return 0; }
    void flush(void) {}
    size_t write(const uint8_t c) { return 0; }
    size_t write(const uint8_t* buffer, size_t size) { return 0; }

    size_t print(long, int = DEC) { return 0; }
    size_t print(unsigned long, int = DEC) { return 0; }
    size_t print(double, int = 2) { return 0; }
    size_t print(char, int = 2) { return 0; }
    size_t print(char[], int = 2) { return 0; }
    size_t print(int, int = DEC) { return 0; }
    size_t print(uint8_t, int = 2) { return 0; }
    size_t print(uint16_t, int = 2) { return 0; }
    size_t print(uint32_t, int = 2) { return 0; }
    size_t print(std::string&, int = 2) { return 0; }
    //size_t print(const Printable&) {}

    //size_t println(const __FlashStringHelper*) { return 0; }
    size_t println(const std::string& s) { return 0; }
    size_t println(const char[]) { return 0; }
    size_t println(char) { return 0; }
    size_t println(char[]) { return 0; }
    size_t println(unsigned char, int = DEC) { return 0; }
    size_t println(int, int = DEC) { return 0; }
    size_t println(unsigned int, int = DEC) { return 0; }
    size_t println(long, int = DEC) { return 0; }
    size_t println(unsigned long, int = DEC) { return 0; }
    size_t println(double, int = 2) { return 0; }
    //size_t println(const Printable&) { return 0; }
    size_t println(void) { return 0; }
};


#endif // SUPPPORT_4_TESTING_HPP