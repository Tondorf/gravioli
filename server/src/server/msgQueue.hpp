#pragma once

#include <atomic>
#include <chrono>
#include <thread>

#include "SimpleLogger/logger.hpp"

#include "planet_generated.h"


namespace server {
    class Publisher {
    private:
        int _lastAllocSize; 

    public:
        static constexpr std::size_t
        SIMULTANEOUSLY_ALLOCATED_INSTANCES_THRESHOLD = 10;

        using BinaryData = flatbuffers::FlatBufferBuilder;

        static std::atomic<std::size_t> currentlyAllocatedInstances;


        Publisher() : _lastAllocSize(-1) {
        }


        virtual ~Publisher() {
            using namespace std::chrono_literals;

            while (currentlyAllocatedInstances.load() > 0) {
                Log::info("Waiting until all allocated instances of Publisher \
                           are deleted: %d instances",
                          currentlyAllocatedInstances.load());

                std::this_thread::sleep_for(1s);
            }

            Log::info("All allocated instances of Publisher are deleted.");
        }


        std::vector<BinaryData *> pop() {
            if (currentlyAllocatedInstances.load() 
                > SIMULTANEOUSLY_ALLOCATED_INSTANCES_THRESHOLD) {
                Log::error("Number of simultaneously allocated instances\
                            is above threshold: %d instances",
                           currentlyAllocatedInstances.load());
            }

            flatbuffers::FlatBufferBuilder *builder;
            if (_lastAllocSize > 0) {
                builder = new flatbuffers::FlatBufferBuilder(_lastAllocSize);
            } else {
                builder = new flatbuffers::FlatBufferBuilder();
            }

            ++currentlyAllocatedInstances;

            auto game = game::CreatePlanet(*builder, 1.f, 2.f);
            builder->Finish(game);

            _lastAllocSize = static_cast<int>(builder->GetSize());

            std::vector<BinaryData *> data;
            data.push_back(builder);

            return data;
        }
    };

    std::atomic<std::size_t> Publisher::currentlyAllocatedInstances(0);


    void customFree(void * /*data*/, void *hint) {
        --Publisher::currentlyAllocatedInstances;

        delete static_cast<flatbuffers::FlatBufferBuilder *>(hint);
    }
}

