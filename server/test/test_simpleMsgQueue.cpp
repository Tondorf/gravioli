#include <gtest/gtest.h>

#include "simpleMsgQueue.hpp"


::testing::AssertionResult equalAndNotNull(server::IMessage *a,
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


::testing::AssertionResult equalAndNotEmpty(
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


constexpr crypto::Key TESTKEY {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f
};


TEST(SimpleMsgQueue, popsMessagesAsFIFOQueue) { 
    byte *data1 = new byte[3];
    data1[0] = 0x01;
    data1[1] = 0x02;
    data1[2] = 0x03;

    byte *data2 = new byte[5];
    data2[0] = 0x01;
    data2[1] = 0x02;
    data2[2] = 0x03;
    data2[3] = 0x04;
    data2[4] = 0x05;

    server::Message *msg1 = new server::Message(TESTKEY, data1, 1);
    server::Message *msg2 = new server::Message(TESTKEY, data2, 1);

    std::vector<server::IMsgQueue::Messages::msg_t> msgs1;
    std::vector<server::IMsgQueue::Messages::msg_t> msgs2;

    msgs1.push_back(std::make_pair(msg1, nullptr));
    msgs2.push_back(std::make_pair(msg2, nullptr));

    auto msgs1_copy(msgs1);
    auto msgs2_copy(msgs2);

    std::size_t topicID1 = 1;
    std::size_t topicID2 = 2;

    server::SimpleMsgQueue msgQueue;
    msgQueue.push(server::IMsgQueue::Messages(topicID1, std::move(msgs1)));
    msgQueue.push(server::IMsgQueue::Messages(topicID2, std::move(msgs2)));

    bool success;
    server::IMsgQueue::Messages poppedMsgs;

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
