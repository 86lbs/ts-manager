#pragma once
#include "Exception.hpp"
#include <curl/curl.h>

class CurlError : public Exception
{
public:
    CurlError(const char *file, int line, CURLcode errorCode)
        : Exception(file, line, "CURL error " + std::to_string(errorCode) + ": "
                        + std::string(curl_easy_strerror(errorCode))) {}
};
#define THROW_CURL_ERROR(errorCode) throw CurlError(__FILE__, __LINE__, errorCode)
#define ASSERT_CURL_OK(expr)                                                \
    do {                                                                    \
        CURLcode res = (expr);                                              \
        if (res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK)            \
            THROW_CURL_ERROR(res);                                          \
    } while (false)