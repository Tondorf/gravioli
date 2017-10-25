#pragma once


namespace utils {
    class Runnable {
    protected:
        bool _stopped;

    public:
        Runnable();

        virtual ~Runnable() = default;

        virtual bool run();

        virtual void stop();

        virtual bool loop() = 0;
    };
}
