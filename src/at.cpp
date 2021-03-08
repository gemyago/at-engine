#include <at.h>
#include <stdlib.h>
#include <string.h>

namespace at
{

    Input::Input(const char *body, const size_t length) : body(body), length(length)
    {
    }

    Responder::Responder(at::TextStream *stream)
    {
        this->_stream = stream;
    }

    void Responder::writeLine(const char *line)
    {
        this->writeLine(line, strlen(line));
    }

    void Responder::writeLine(const char *line, const size_t length)
    {
        // We need space trailing '\n'
        size_t outputSize = length + 1;
        char *output = (char *)malloc(outputSize);
        memcpy(&output[0], line, length);
        output[length] = '\n';
        this->_stream->write(output, outputSize);
        free(output);
    }

    void Responder::write(const int value)
    {
        this->_stream->write(value);
    }

    void Responder::write(const char value)
    {
        this->_stream->write(value);
    }

    void Responder::write(const char *value)
    {
        this->_stream->write(value, strlen(value));
    }

    void Responder::writeOk()
    {
        this->_stream->write(RESPONSE_OK, strlen(RESPONSE_OK));
    }

    void Responder::writeError()
    {
        this->_stream->write(RESPONSE_ERROR, strlen(RESPONSE_ERROR));
    }

    Handler::Handler(const char *name)
    {
        _name = name;
    }

    Handler::~Handler()
    {
    }

    const char *Handler::Name()
    {
        return _name;
    }

    DefaultHandler::DefaultHandler() : Handler("AT")
    {
    }

    void DefaultHandler::Handle(const Input input, Responder *resp)
    {
        resp->writeOk();
    }

    Engine::Engine(at::TextStream *stream)
    {
        _stream = stream;
        _defaultHandler = new DefaultHandler();
    }

    Engine::~Engine()
    {
        free(_handlers);
        delete _defaultHandler;
    }

    void Engine::resetBuffer()
    {
        memset(_cmdBuffer, 0, MAX_COMMAND_SIZE);
        _cmdBufferConsumed = 0;
        _cmdInputStart = 0;
    }

    void Engine::addCommandHandler(Handler *handler)
    {
        at_engine_log("Registering AT handler: [%d]: %s", _handlersCount, handler->Name());

        Handler **nextHandlers = (Handler **)malloc(sizeof(Handler *) * (_handlersCount + 1));
        if (_handlersCount != 0)
        {
            memcpy(nextHandlers, _handlers, sizeof(Handler *) * _handlersCount);
        }
        nextHandlers[_handlersCount++] = handler;
        free(_handlers);
        _handlers = nextHandlers;
    }

    void Engine::setup()
    {
        this->addCommandHandler(_defaultHandler);
        at_engine_log("AT engine initialized. Total handlers: %d", _handlersCount);
    }

    // Use for debugging
    // void dumpBuffer(char * buffer, size_t length)
    // {
    //     println("Buffer dump. Size:" << length);
    //     for(int i = 0; i < length; i++)
    //     {
    //         println("char " << i << ":" << int(buffer[i]) << ":" << buffer[i]);
    //     }
    // }

    void Engine::loop()
    {
        const size_t available = _stream->available();
        if (available > 0)
        {
            if (available + _cmdBufferConsumed > MAX_COMMAND_SIZE)
            {
                this->resetBuffer();
                Responder(_stream).writeError();
                return;
            }

            size_t cmdSize = _stream->read(&_cmdBuffer[_cmdBufferConsumed], available);
            size_t cmdEnd = 0;
            bool cmdEndFound = false;

            int nextConsumedSize = _cmdBufferConsumed + cmdSize;
            for (size_t i = _cmdBufferConsumed; i < nextConsumedSize; i++)
            {
                // Handle backspaces (e.g remove corresponding chars)
                if (_cmdBuffer[i] == '\b')
                {
                    for (int j = i - 1; j < nextConsumedSize - 2; j++)
                    {
                        _cmdBuffer[j] = _cmdBuffer[j + 2];
                    }
                    _cmdBuffer[nextConsumedSize - 1] = 0;
                    _cmdBuffer[nextConsumedSize - 2] = 0;
                    nextConsumedSize -= 2;
                    i -= 2;
                    continue;
                }

                cmdEnd = i;
                if (_cmdBuffer[i] == '=')
                {
                    _cmdInputStart = i + 1;
                }
                if (_cmdBuffer[i] == '\n')
                {
                    cmdEndFound = true;
                    break;
                }
            }
            _cmdBufferConsumed = nextConsumedSize;
            if (!cmdEndFound)
            {
                return;
            }

            // Ignore CR
            if (_cmdBuffer[cmdEnd - 1] == '\r')
            {
                cmdEnd = cmdEnd - 1;
            }

            Responder responder(_stream);

            bool handled = false;
            for (size_t i = 0; i < _handlersCount; i++)
            {
                at::Handler *handler = _handlers[i];
                const char *name = handler->Name();

                size_t cmdLength = _cmdInputStart > 0 ? _cmdInputStart - 1 : cmdEnd;
                if (strlen(name) == cmdLength && strncmp(name, _cmdBuffer, cmdLength) == 0)
                {
                    handled = true;
                    char *body = NULL;
                    size_t length = 0;
                    if (_cmdInputStart > 0)
                    {
                        body = &_cmdBuffer[_cmdInputStart];
                        length = cmdEnd - _cmdInputStart;
                    }
                    handler->Handle(at::Input(body, length), &responder);
                    break;
                }
            }

            if (!handled)
            {
                at_engine_log("Unexpected command: '%s'", _cmdBuffer);

                // Use for debugging
                // for(int i = 0; i < _cmdBufferConsumed; i++)
                // {
                //     at_engine_log("Char code: %d:%d", i, _cmdBuffer[i]);
                // }

                responder.writeError();
            }

            this->resetBuffer();
        }
    }

} // namespace at