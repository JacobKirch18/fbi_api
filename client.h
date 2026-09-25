#pragma once

#include <string>

class ApiClient {
public:
    struct Response {
        unsigned int statusCode = 0;
        std::string body;

        bool ok() const noexcept {
            return statusCode >= 200 && statusCode < 300;
        }
    };

    Response request(const std::string& query) const;
};