#include <Arduino.h>
#include <at.h>

// Define command handler
class ATPing : public at::Handler
{
public:
    ATPing() : at::Handler("AT+PING"){};
    void Handle(at::Input input, at::Responder *resp)
    {
        if (input.length == 0)
        {
            resp->writeLine("PONG");
        }
        else
        {
            resp->writeLine(input.body, input.length);
        }
        resp->writeOk();
    }
};

// Setup AT engine for default Serial
at::Engine atEngine(&Serial);

auto atPing = ATPing();

void setup()
{
    Serial.begin(115200);

    // Register command handler
    atEngine.addCommandHandler(&atPing);
    atEngine.setup();
}

void loop()
{
    atEngine.loop();
}