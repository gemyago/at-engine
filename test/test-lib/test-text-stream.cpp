#include "test-text-stream.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>

TestTextStream::TestTextStream()
{
}

TestTextStream::~TestTextStream()
{
}

int TestTextStream::available()
{
    return this->readBuffer.length();
}

size_t TestTextStream::read(char *buffer, size_t length)
{
    if(this->readBuffer.length() == 0)
    {
        return 0;
    }
    return this->readBuffer.copy(buffer, length);
}

size_t TestTextStream::write(const char *buffer, size_t size)
{
    this->writeBuffer.append(buffer, size);
    return size;
}

size_t TestTextStream::write(const int number)
{
    auto val = std::to_string(number);
    this->writeBuffer.append(val);
    return val.length();
}

size_t TestTextStream::write(const unsigned int number)
{
    auto val = std::to_string(number);
    this->writeBuffer.append(val);
    return val.length();
}

size_t TestTextStream::write(const long number)
{
    auto val = std::to_string(number);
    this->writeBuffer.append(val);
    return val.length();
}

size_t TestTextStream::write(const unsigned long number)
{
    auto val = std::to_string(number);
    this->writeBuffer.append(val);
    return val.length();
}

size_t TestTextStream::write(const char number)
{
    this->writeBuffer.push_back(number);
    return 1;
}

void TestTextStream::setReadBuffer(const char *buffer)
{
    this->readBuffer.assign(buffer);
}

void TestTextStream::reset()
{
    this->readBuffer.clear();
    this->writeBuffer.clear();
}