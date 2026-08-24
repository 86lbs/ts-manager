#pragma once

#include "Exception.hpp"

#define ASSERT(condition) \
    if (!(condition))     \
        THROW_ASSERT_FAILED("Assertion failed: " #condition);

#define THROW_ASSERT_FAILED(message) throw AssertFailedException(__FILE__, __LINE__, message)

class AssertFailedException : public Exception
{
public:
    AssertFailedException(const char *file, int line, const std::string &message)
        : Exception(file, line, "Assertion failed: " + message) {}
};
