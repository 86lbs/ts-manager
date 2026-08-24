#pragma once
#include "Exception.hpp"
#include <sqlite3.h>     // 从 jsapi/include/sqlite3/sqlite3.h 来

class DatabaseError : public Exception
{
public:
    DatabaseError(const char *file, int line, sqlite3 *conn)
        : Exception(file, line, "Database error: " + std::string(sqlite3_errmsg(conn))) {}
};
#define THROW_DATABASE_ERROR(conn) throw DatabaseError(__FILE__, __LINE__, conn)
#define ASSERT_DATABASE_OK(expr)                                            \
    do {                                                                    \
        int res = (expr);                                                   \
        if (res != SQLITE_OK && res != SQLITE_DONE) THROW_DATABASE_ERROR(conn); \
    } while (0)