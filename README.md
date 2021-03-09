# at-engine
Arduino library to setup serial communication using AT protocol

## Installing

### Platformio

```
pio lib install --save evgenymyasishchev/AT-Engine
```

## Example usage

Define AT command handler
```cpp
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
```

Then run the example and start serial monitor (with echo mode preferably). Then:

Type `AT` (a built-in test command). Should respond with `OK`
```
AT
OK
```

Type `AT+PING`
```
AT+PING
PONG
OK
```

Type `AT+PING=Hello World`
```
AT+PING
Hello World
OK
```

## Dev notes

You may need to have direnv and reflex installed

### Running tests

Install native deps:
```
pio lib install
```

Run tests:
```
# All tests at once
make test

# Run all tests on every change
tests-watch

# Run specific tests only on every change
tests-watch --gtest_filter=atResponder*
```
