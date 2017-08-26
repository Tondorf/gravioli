#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include <tuple>

#include "SimpleLogger/logger.hpp"

#include "server.hpp"

#include "planet_generated.h"


namespace server {
    class Message: public IMessage, 
                   public flatbuffers::FlatBufferBuilder {
    public:
        Message():
            flatbuffers::FlatBufferBuilder() {
        }

        Message(std::size_t allocSize):
            flatbuffers::FlatBufferBuilder(allocSize) {
        }


        virtual ~Message() = default;


        virtual byte *getBufferPointer() const {
            return static_cast<byte *>(GetBufferPointer());
        }


        virtual std::size_t getSize() const {
            return static_cast<std::size_t>(GetSize());
        }


        virtual flatbuffers::FlatBufferBuilder *asFlatBufferBuilder() {
            return static_cast<FlatBufferBuilder *>(this);
        };
    };


    class MsgQueue: public IMsgQueue {
    private:
        int _lastAllocSize; 

    public:
        static constexpr std::size_t
        SIMULTANEOUSLY_ALLOCATED_INSTANCES_THRESHOLD = 10;

        static std::atomic<std::size_t> currentlyAllocatedInstances;


        MsgQueue() : _lastAllocSize(-1) {
        }


        virtual ~MsgQueue() {
            using namespace std::chrono_literals;

            while (currentlyAllocatedInstances.load() > 0) {
                Log::info("Waiting until all allocated instances of MsgQueue"
                          "are deleted: %d instances",
                          currentlyAllocatedInstances.load());

                std::this_thread::sleep_for(1s);
            }

            Log::info("All allocated instances of MsgQueue are deleted.");
        }


        Messages pop() {
            if (currentlyAllocatedInstances.load() 
                > SIMULTANEOUSLY_ALLOCATED_INSTANCES_THRESHOLD) {
                Log::error("Number of simultaneously allocated instances "
                           "is above threshold: %d instances",
                           currentlyAllocatedInstances.load());
            }

            Message *builder = nullptr;
            if (_lastAllocSize > 0) {
                builder = new Message(_lastAllocSize);
            } else {
                builder = new Message();
            }

            ++currentlyAllocatedInstances;

            auto game = game::CreatePlanet(*(builder->asFlatBufferBuilder()),
                                           1.f, 2.f);
            builder->Finish(game);

            _lastAllocSize = static_cast<int>(builder->GetSize());

            const std::size_t ID = 0;
            std::vector<Messages::msg_t> msgs;
            msgs.push_back(std::make_pair(builder, true));

            return Messages(ID, std::move(msgs));
        }
    };

    std::atomic<std::size_t> MsgQueue::currentlyAllocatedInstances(0);


    void customFree(void * /*data*/, void *hint) {
        --MsgQueue::currentlyAllocatedInstances;

        delete static_cast<Message *>(hint);
    }
}

