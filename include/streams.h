#ifndef AT_STREAMS_H
#define AT_STREAMS_H

#include <stddef.h>

namespace at
{

    class TextStream
    {
    public:
        virtual int available() = 0;
        virtual size_t read(char *buffer, size_t length) = 0;
        virtual size_t write(const char *buffer, size_t size) = 0;
        virtual size_t write(const int number) = 0;
        virtual size_t write(const char value) = 0;
    };

#ifdef ARDUINO

    class SerialTextStream : public TextStream
    {
    private:
        Stream *_serial;

    public:
        SerialTextStream(Stream *serial);

        int available();
        size_t read(char *buffer, size_t length);
        size_t write(const char *buffer, size_t size);
        size_t write(const int number);
        size_t write(const char value);
    };

#endif

} // namespace at

#endif