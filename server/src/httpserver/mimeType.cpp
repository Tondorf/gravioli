#include "httpserver/mimeType.hpp"


namespace httpserver {
    namespace mimeTypes {
        struct Mapping {
            const char *extension;
            const char *mimeType;
        } mappings[] = {
            { "gif", "image/gif" },
            { "htm", "text/html" },
            { "html", "text/html" },
            { "jpg", "image/jpeg" },
            { "png", "image/png" }
        };

        std::string extension2type(const std::string& extension) {
            for (auto m : mappings) {
                if (m.extension == extension) {
                    return m.mimeType;
                }
            }

            return "text/plain";
        }
    }
}
