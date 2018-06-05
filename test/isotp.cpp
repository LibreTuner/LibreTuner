#include <iostream>
#include <memory>
#include <thread>
#include <future>
#include <chrono>
#include <algorithm>

#include "gtest/gtest.h"
#include "protocols/mockcaninterface.h"
#include "protocols/isotpprotocol.h"

class IsoTp : public ::testing::Test {
protected:
  std::shared_ptr<MockCanInterface> can = std::make_shared<MockCanInterface>();
  isotp::Protocol isotp = isotp::Protocol(can);
};

TEST_F(IsoTp, Short) {
  uint8_t message[] = {'t', 'e', 's', 't'};
  
  std::promise<CanMessage> promise;
  
  auto conn = can->connect([&promise](const CanMessage &msg) {
    promise.set_value(std::move(msg));
  });
  
  std::future<CanMessage> fut = promise.get_future();

  isotp::Packet packet(gsl::make_span(message, 4));
  isotp.send(std::move(packet)).get();
  
  std::future_status status = fut.wait_for(std::chrono::milliseconds(100));
  ASSERT_EQ(status, std::future_status::ready);
  
  uint8_t expected[] = {0x04, 't', 'e', 's', 't', 0, 0, 0};
  
  CanMessage msg = fut.get();
  ASSERT_EQ(msg.length(), sizeof(expected));
  EXPECT_TRUE(std::equal(msg.message(), msg.message() + sizeof(expected), expected));
}



TEST_F(IsoTp, Long) {
  std::vector<CanMessage> messages;
  
  std::mutex mutex;
  std::condition_variable cv;
  
  uint8_t message[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

  isotp.recvPacketAsync([&message](isotp::Error err, isotp::Packet &&rPacket) {
    ASSERT_EQ(err, isotp::Error::Success);
    std::vector<uint8_t> data;
    rPacket.moveAll(data);
    EXPECT_TRUE(std::equal(message, message + sizeof(message), std::begin(data)));
  });

  isotp::Packet packet(gsl::make_span(message, 26));
  ASSERT_EQ(isotp.send(std::move(packet)).get(), isotp::Error::Success);
}
