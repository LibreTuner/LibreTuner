#include <iostream>
#include <memory>
#include <thread>
#include <future>
#include <chrono>
#include <algorithm>

#include "gtest/gtest.h"
#include "protocols/mockcaninterface.h"
#include "protocols/isotpinterface.h"

class IsoTp : public ::testing::Test {
protected:
  std::shared_ptr<MockCanInterface> can = std::make_shared<MockCanInterface>();
  std::shared_ptr<IsoTpInterface> isotp = IsoTpInterface::get(can);
};

TEST_F(IsoTp, Short) {
  uint8_t message[] = {'t', 'e', 's', 't'};
  
  std::promise<CanMessage> promise;
  
  auto conn = can->connect([&promise](const CanMessage &msg) {
    promise.set_value(std::move(msg));
  });
  
  std::future<CanMessage> fut = promise.get_future();
  
  isotp->request(message, sizeof(message), IsoTpOptions(0x7e0, 0x7e8));
  
  std::future_status status = fut.wait_for(std::chrono::milliseconds(100));
  ASSERT_EQ(status, std::future_status::ready);
  
  uint8_t expected[] = {0x04, 't', 'e', 's', 't', 0, 0, 0};
  
  CanMessage msg = fut.get();
  ASSERT_EQ(msg.length(), sizeof(expected));
  EXPECT_TRUE(std::equal(msg.message(), msg.message() + sizeof(expected), expected));
}
