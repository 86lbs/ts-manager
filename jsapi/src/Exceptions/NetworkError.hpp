#pragma once
#include "Exception.hpp"

class NetworkError : public Exception
{
public:
    NetworkError(const char *file, int line, int statusCode)
        : Exception(file, line, "Network error " + std::to_string(statusCode)) {}
};
#define THROW_NETWORK_ERROR(statusCode) throw NetworkError(__FILE__, __LINE__, statusCode)