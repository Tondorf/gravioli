#include <gtest/gtest.h>

#include "simpleMsgQueue.hpp"

#include "helper.hpp"


inline ::testing::AssertionResult equalAndNotNull(server::IMessage *a,
                                                  server::IMessage *b) {
    if (a == nullptr || b == nullptr) {
        return ::testing::AssertionFailure() <<
               "one or both are nullptr";
    }

    auto keyA = a->key();
    auto keyB = b->key();
    auto keyLengthA = keyA.size();
    auto keyLengthB = keyB.size();
    if (keyLengthA != keyLengthB) {
        return ::testing::AssertionFailure() <<
               "keys differ in length;" <<
               "; " << keyLengthA << " != " << keyLengthB;
               
    }
    for (std::size_t i = 0; i < keyLengthA; ++i) {
        if (keyA[i] != keyB[i]) {
            return ::testing::AssertionFailure() << 
                   "key mismatch at postion " << i <<
                   "; " << (int) keyA[i] << " != " << (int) keyB[i];
        }
    }

    auto sizeOfA = a->getSize();
    auto sizeOfB = b->getSize();
    if (sizeOfA != sizeOfB) {
        return ::testing::AssertionFailure() <<
               "stored data differ in size" <<
               "; " << sizeOfA << " != " << sizeOfB;
    }
    byte *dataPtrA = a->getBufferPointer();
    byte *dataPtrB = a->getBufferPointer();
    for (std::size_t i = 0; i < sizeOfA; ++i) {
        if (dataPtrA[i] != dataPtrB[i]) {
            return ::testing::AssertionFailure() << 
                   "stored data mismatch at postion " << i <<
                   "; " << (int) dataPtrA[i] << " != " << (int) dataPtrB[i];
        }
    }

    return ::testing::AssertionSuccess();
}


inline ::testing::AssertionResult equalAndNotEmpty(
    std::vector<server::IMsgQueue::Messages::msg_t> a,
    std::vector<server::IMsgQueue::Messages::msg_t> b) {

    auto sizeOfA = a.size();
    auto sizeOfB = b.size();
    if (sizeOfA == 0 || sizeOfB == 0) {
        return ::testing::AssertionFailure() <<
               "one or both are empty";
    }
    if (sizeOfA != sizeOfB) {
        return ::testing::AssertionFailure() <<
               "amount of stored data differ" <<
               "; " << sizeOfA << " != " << sizeOfB;
    }
    for (std::size_t i = 0; i < sizeOfA; ++i) {
        auto *msgA = std::get<0>(a[i]);
        auto *msgB = std::get<0>(b[i]);
        auto assertionResult = equalAndNotNull(msgA, msgB);
        if (!assertionResult) {
            return assertionResult;
        }
    }

    return ::testing::AssertionSuccess();
}


TEST(SimpleMsgQueue, popsMessagesAsFIFOQueue) { 
    using msg_t = server::IMsgQueue::Messages::msg_t;
    using Messages = server::IMsgQueue::Messages;

    constexpr crypto::Key TESTKEY {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };

    auto *msg1 = createMsg(TESTKEY, {
        0x01, 0x02, 0x03
    });
    auto *msg2 = createMsg(TESTKEY, {
        0x04, 0x05, 0x06, 0x07, 0x08
    });

    std::vector<msg_t> msgs1 {
        std::make_pair(msg1, nullptr)
    };
    std::vector<msg_t> msgs2 {
        std::make_pair(msg2, nullptr)
    };

    auto msgs1_copy(msgs1);
    auto msgs2_copy(msgs2);

    const std::size_t topicID1 = 1;
    const std::size_t topicID2 = 2;

    server::SimpleMsgQueue msgQueue;
    msgQueue.push(Messages(topicID1, std::move(msgs1)));
    msgQueue.push(Messages(topicID2, std::move(msgs2)));

    bool success;
    Messages poppedMsgs;

    success = msgQueue.pop(poppedMsgs);
    ASSERT_TRUE(success);
    ASSERT_EQ(poppedMsgs.topicID, topicID1);
    ASSERT_TRUE(equalAndNotEmpty(poppedMsgs.msgs, msgs1_copy));

    success = msgQueue.pop(poppedMsgs);
    ASSERT_TRUE(success);
    ASSERT_EQ(poppedMsgs.topicID, topicID2);
    ASSERT_TRUE(equalAndNotEmpty(poppedMsgs.msgs, msgs2_copy));

    success = msgQueue.pop(poppedMsgs);
    ASSERT_FALSE(success);

    delete msg1;
    delete msg2;
}
