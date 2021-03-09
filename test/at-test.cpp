#include <gtest/gtest.h>
#include "test-lib/test-text-stream.h"
#include "test-lib/test-at-handler.h"
#include "test-lib/random.h"
#include "at.h"

namespace
{

TEST(atResponder, writeOk)
{
    TestTextStream testStream;
    at::Responder responder(&testStream);
    responder.writeOk();

    ASSERT_EQ("OK\n", testStream.writeBuffer);
}

TEST(atResponder, writeError)
{
    TestTextStream testStream;
    at::Responder responder(&testStream);
    responder.writeError();

    ASSERT_EQ("ERROR\n", testStream.writeBuffer);
}

TEST(atResponder, writeLine)
{
    TestTextStream testStream;
    at::Responder responder(&testStream);
    const char line[] = "SOME DATA LINE";
    responder.writeLine(line);

    char want[20] = "";
    strcat(want, line);
    strcat(want, "\n");

    ASSERT_EQ(want, testStream.writeBuffer);
}

TEST(atResponder, write_int)
{
    TestTextStream testStream;
    at::Responder responder(&testStream);

    testStream.reset();
    const int intVal = test::randomNumber(-20000, 20000);
    responder.write(intVal);
    ASSERT_EQ(std::to_string(intVal).c_str(), testStream.writeBuffer);

    testStream.reset();
    const unsigned uintVal = test::randomNumber(0, 20000);
    responder.write(uintVal);
    ASSERT_EQ(std::to_string(uintVal).c_str(), testStream.writeBuffer);

    testStream.reset();
    const long longVal = test::randomNumber(-20000, 20000);
    responder.write(longVal);
    ASSERT_EQ(std::to_string(longVal).c_str(), testStream.writeBuffer);

    testStream.reset();
    const unsigned long unsignedLongVal = test::randomNumber(0, 20000);
    responder.write(unsignedLongVal);
    ASSERT_EQ(std::to_string(unsignedLongVal).c_str(), testStream.writeBuffer);
}

TEST(atResponder, write_single_char)
{
    const char value = test::randomNumber(0, 255);
    TestTextStream testStream;
    at::Responder responder(&testStream);
    responder.write(value);
    const char want[] = { value, 0 };
    ASSERT_EQ(want, testStream.writeBuffer);
}

TEST(atResponder, write_c_str)
{
    std::string want = "some response string";
    TestTextStream testStream;
    at::Responder responder(&testStream);
    responder.write(want.c_str());
    ASSERT_EQ(want, testStream.writeBuffer);
}

TEST(atResponder, writeLineWithLength)
{
    TestTextStream testStream;
    at::Responder responder(&testStream);
    const char line[] = "SOME DATA LINE HELLO";
    responder.writeLine(line, strlen("SOME DATA LINE"));

    std::string want = "SOME DATA LINE\n";
    ASSERT_EQ(want, testStream.writeBuffer);
}

TEST(atEngine, handleAT)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);
    engine.setup();

    testStream.readBuffer.assign("AT\n");

    engine.loop();

    std::string want = "OK\n";

    ASSERT_EQ(want, testStream.writeBuffer);
}

TEST(atEngine, handleUnknownCommand)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);
    engine.setup();
    std::string want = "ERROR\n";

    testStream.readBuffer.assign("UNKNOWN\n");
    engine.loop();
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();

    testStream.readBuffer.assign("AT1\n");
    engine.loop();
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();

    testStream.readBuffer.assign("0AT\n");
    engine.loop();
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();
}

TEST(atEngine, handleCommandNoInput)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);
    
    TestATHandler cmd1("AT+CMD1", "CMD1-RESPONSE");
    TestATHandler cmd2("AT+CMD2", "CMD2-RESPONSE");
    TestATHandler cmd3("AT+CMD3", "CMD3-RESPONSE");

    engine.addCommandHandler(&cmd1);
    engine.addCommandHandler(&cmd2);
    engine.addCommandHandler(&cmd3);
    engine.setup();

    testStream.readBuffer.assign("AT+CMD1\n");
    engine.loop();

    ASSERT_EQ("", cmd1.gotInput);
    ASSERT_TRUE(cmd1.called) << "cmd1 not called";
    std::string want = "CMD1-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();

    testStream.readBuffer.assign("AT+CMD2\n");
    engine.loop();

    ASSERT_EQ("", cmd2.gotInput);
    ASSERT_TRUE(cmd2.called) << "cmd2 not called";
    want = "CMD2-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();

    testStream.readBuffer.assign("AT+CMD3\n");
    engine.loop();

    ASSERT_EQ("", cmd3.gotInput);
    ASSERT_TRUE(cmd3.called) << "cmd3 not called";
    want = "CMD3-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
}

TEST(atEngine, handleCommandWithInput)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);

    TestATHandler cmd1("AT+CMD1", "CMD1-RESPONSE");
    TestATHandler cmd2("AT+CMD2", "CMD2-RESPONSE");

    engine.addCommandHandler(&cmd1);
    engine.addCommandHandler(&cmd2);
    engine.setup();

    testStream.readBuffer.assign("AT+CMD1=CMD1-INPUT\n");
    engine.loop();

    ASSERT_TRUE(cmd1.called) << "cmd1 not called";
    ASSERT_EQ("CMD1-INPUT", cmd1.gotInput);
    std::string want = "CMD1-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();

    testStream.readBuffer.assign("AT+CMD2=CMD2-INPUT\r\n");
    engine.loop();

    ASSERT_EQ("CMD2-INPUT", cmd2.gotInput);
    ASSERT_TRUE(cmd2.called) << "cmd2 not called";
    want = "CMD2-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();
}

TEST(atEngine, handleChunkedWrites)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);

    TestATHandler cmd1("AT+CMD1", "CMD1-RESPONSE");

    engine.addCommandHandler(&cmd1);
    engine.setup();

    testStream.readBuffer.assign("A");
    engine.loop();
    testStream.readBuffer.assign("T");
    engine.loop();
    testStream.readBuffer.assign("\n");
    engine.loop();

    std::string want = "OK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();

    testStream.readBuffer.assign("AT");
    engine.loop();
    testStream.readBuffer.assign("+CMD1");
    engine.loop();
    testStream.readBuffer.assign("\n");
    engine.loop();

    ASSERT_EQ(cmd1.gotInput, "");
    ASSERT_TRUE(cmd1.called) << "cmd1 not called";
    want = "CMD1-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();
}

TEST(atEngine, handleBackspace)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);

    TestATHandler cmd1("AT+CMD1", "CMD1-RESPONSE");

    engine.addCommandHandler(&cmd1);
    engine.setup();

    testStream.readBuffer.assign("AT+cmd2");
    engine.loop();
    testStream.readBuffer.assign("\b");
    engine.loop();
    testStream.readBuffer.assign("\b");
    engine.loop();
    testStream.readBuffer.assign("\b");
    engine.loop();
    testStream.readBuffer.assign("\b");
    engine.loop();
    testStream.readBuffer.assign("CMD1");
    engine.loop();
    testStream.readBuffer.assign("\n");
    engine.loop();

    ASSERT_EQ(cmd1.gotInput, "");
    ASSERT_TRUE(cmd1.called) << "cmd1 not called";
    std::string want = "CMD1-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();
}

TEST(atEngine, handleBackspaceWithInput)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);

    TestATHandler cmd1("AT+CMD1", "CMD1-RESPONSE");

    engine.addCommandHandler(&cmd1);
    engine.setup();

    testStream.readBuffer.assign("AT+cmd2=");
    engine.loop();
    testStream.readBuffer.assign("\b\b\b\b\b");
    engine.loop();
    testStream.readBuffer.assign("CMD1=hello");
    engine.loop();
    testStream.readBuffer.assign("\n");
    engine.loop();

    ASSERT_TRUE(cmd1.called) << "cmd1 not called";
    ASSERT_EQ("hello", cmd1.gotInput);
    std::string want = "CMD1-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();
}

TEST(atEngine, handleChunkedWritesWithInput)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);

    TestATHandler cmd1("AT+CMD1", "CMD1-RESPONSE");

    engine.addCommandHandler(&cmd1);
    engine.setup();

    testStream.readBuffer.assign("AT+CMD1=");
    engine.loop();
    testStream.readBuffer.assign("CMD1-INPUT\n");
    engine.loop();

    ASSERT_TRUE(cmd1.called) << "cmd1 not called";
    ASSERT_EQ("CMD1-INPUT", cmd1.gotInput);
    std::string want = "CMD1-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();
}

TEST(atEngine, ignoreCR)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);

    TestATHandler cmd1("AT+CMD1", "CMD1-RESPONSE");

    engine.addCommandHandler(&cmd1);
    engine.setup();

    testStream.readBuffer.assign("AT\r\n");
    engine.loop();

    std::string want = "OK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();

    testStream.readBuffer.assign("AT+CMD1\r\n");
    engine.loop();

    ASSERT_EQ(cmd1.gotInput, "");
    ASSERT_TRUE(cmd1.called) << "cmd1 not called";
    want = "CMD1-RESPONSE\nOK\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();
}

TEST(atEngine, errorIfLargeCommand)
{
    TestTextStream testStream;
    at::Engine engine(&testStream);

    TestATHandler cmd1("AT+CMD1", "CMD1-RESPONSE");

    engine.addCommandHandler(&cmd1);
    engine.setup();

    testStream.readBuffer.assign("012345678901234567890123456789");
    engine.loop();
    testStream.readBuffer.assign("0123456789");
    engine.loop();

    std::string want = "ERROR\n";
    ASSERT_EQ(want, testStream.writeBuffer);
    testStream.reset();
}

} // namespace
