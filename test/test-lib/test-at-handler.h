#ifndef TEST_AT_HANDLER_H
#define TEST_AT_HANDLER_H

#include <at.h>
#include <string>

class TestATHandler : public at::Handler
{
private:
    const char *_response;
public:
    bool called = false;
    std::string gotInput;
    TestATHandler(const char *name, const char *response) : at::Handler(name)
    {
        _response = response;
    }

    void Handle(const at::Input input, at::Responder *resp)
    {
        called = true;
        if(input.length != 0)
        {
            gotInput.assign(input.body, input.length);
        }
        resp->writeLine(_response);
        resp->writeOk();
    }
};

#endif