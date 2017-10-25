#include "runnable.hpp"


namespace utils {
    Runnable::Runnable(): _stopped(false) {
    }


    bool Runnable::run() {
        while (!_stopped) {
            bool rc = loop();
            if (rc == false) {
                return rc;
            }
        }

        return true;
    }


    void Runnable::stop() {
        _stopped = true;
    }
}
