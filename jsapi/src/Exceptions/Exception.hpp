// Exceptions 基类（file:line 进 message）+ 多个分类子类 + 宏
// 用法：try { ASSERT(x); ASSERT_DATABASE_OK(stmt); ASSERT_CURL_OK(curl_*); ... }
//       catch (const std::exception &e) { info.postError(e.what()) 或 ThrowInternalError(e.what()); }
#pragma once
#include <stdexcept>
#include <string>

class Exception : public std::runtime_error
{
public:
    Exception(const char *file, int line, const std::string &message)
        : std::runtime_error(message + " (in " + file + ":" + std::to_string(line) + ")") {}
};
#define THROW_EXCEPTION(message) throw Exception(__FILE__, __LINE__, message)