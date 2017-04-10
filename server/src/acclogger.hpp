#pragma once

#include <cassert>
#include <iostream>
#include <string>

#include "tools.hpp"

namespace Log {

    class AccLogger {
    private:
        FILE *_out;

        AccLogger() : _out(stdout) {
        }

    public:
        ~AccLogger() {
            fclose(_out);
        }

        static AccLogger& getInstance() {
            static AccLogger instance;
            return instance;
        }

        AccLogger(const AccLogger&) = delete;

        void operator=(const AccLogger&) = delete;

        void log(const std::string& ip,
                 const std::string& uident,
                 const std::string& uid,
                 const std::string& type,
                 const std::string& url,
                 const std::string& proto,
                 const std::string& version,
                 int rescode,
                 std::size_t ressize) {

            auto check = [](const std::string &s) {
                return (s.empty() ? "-": s);
            };
            std::string request = "";
            request += check(type) + " " + check(url) + " " + check(proto) + "/" + check(version);
            const char *c_ip = check(ip).c_str();
            const char *c_uident = check(uident).c_str();
            const char *c_uid = check(uid).c_str();

            std::string msg = "";
            msg += check(ip) + " " + check(uident) + " " + check(uid);
            msg += " [" + getFormatedTime() + "]";
            msg += " \"" + request + "\"";
            msg += " " + (rescode < 0 ? "-" : std::to_string(rescode)) + " " + std::to_string(ressize);
            msg += "\n";

            fputs(msg.c_str(), _out); 
        }
    };

    void acclog(const std::string& ip,
                const std::string& uident,
                const std::string& uid,
                const std::string& type,
                const std::string& url,
                const std::string& proto,
                const std::string& version,
                int rescode,
                std::size_t ressize) {
        AccLogger::getInstance().log(ip, uident, uid, type, url, proto, version, rescode, ressize);
    }

    void acclog(const std::string& ip,
                const std::string& uident,
                const std::string& uid,
                const std::string& type,
                const std::string& url,
                const std::string& proto,
                int version,
                int rescode,
                std::size_t ressize) {
        char buff[100];
        snprintf(buff, sizeof(buff), "%.1f", ((double) version) / 10.);
        acclog(ip, uident, uid, type, url, proto, buff, rescode, ressize);
    }
};
