#include "gtest/gtest.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>

using namespace std::placeholders;

TEST(TestFramedPacket, test1) {
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
