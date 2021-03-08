#ifndef MOCKS_TEST_TEXT_STREAM
#define MOCKS_TEST_TEXT_STREAM

#include <streams.h>
#include <string>

class TestTextStream : public at::TextStream
{
public:
    std::string writeBuffer;
    std::string readBuffer;

    TestTextStream();
    ~TestTextStream();

    int available();
    size_t read(char *buffer, size_t length);
    size_t write(const char *buffer, size_t size);
    size_t write(const int number);
    size_t write(const char value);

    void setReadBuffer(const char *buffer);
    void reset();
};

#endif