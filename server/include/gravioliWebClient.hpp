#pragma once

#include <string>
#include "playerProvider.hpp"


namespace gravioli {
    class GravioliWebClient : public simulation::PlayerProvider::WebClient {
    public:
        GravioliWebClient();

        virtual ~GravioliWebClient() = default;

        virtual Response get(const std::string& url) override;
    };
}
