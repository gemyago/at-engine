#ifndef AT_H
#define AT_H

#include "streams.h"

namespace at
{

#ifdef logger_log
#define at_engine_log logger_log
#else
#define at_engine_log
#endif

#define MAX_COMMAND_SIZE 30

#define RESPONSE_OK "OK\n"
#define RESPONSE_ERROR "ERROR\n"

    class Input
    {
    public:
        const char *body = 0;
        const size_t length = 0;
        Input(const char *body, const size_t length);
    };

    class Responder
    {
    private:
        at::TextStream *_stream;

    public:
        Responder(at::TextStream *stream);

        void write(const char ch);
        void write(const char *data);
        void write(const int number);
        void writeLine(const char *line);
        void writeLine(const char *line, const size_t length);
        void writeOk();
        void writeError();
    };

    class Handler
    {
    private:
        const char *_name;

    public:
        Handler(const char *name);
        virtual ~Handler();
        const char *Name();
        virtual void Handle(const Input input, Responder *resp) = 0;
    };

    class DefaultHandler : public Handler
    {
    public:
        DefaultHandler();
        void Handle(const Input input, Responder *resp);
    };

    class Engine
    {
    private:
        at::TextStream *_stream;
        bool _destructStream = false;
        Handler *_defaultHandler;
        Handler **_handlers = 0;
        size_t _handlersCount = 0;
        char _cmdBuffer[MAX_COMMAND_SIZE] = {};
        size_t _cmdBufferConsumed = 0;
        size_t _cmdInputStart = 0;

        void resetBuffer();

    public:
        Engine(at::TextStream *stream);
#ifdef ARDUINO
        Engine(Stream *stream);
#endif

        ~Engine();

        void addCommandHandler(Handler *handler);
        void setup();
        void loop();
    };

} // namespace at

#endif