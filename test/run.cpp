#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <iostream>
#include <fstream>

int main(int argc, char **argv)
{
  std::srand(std::time(nullptr));
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::InitGoogleMock(&argc, argv);

  return RUN_ALL_TESTS();
}