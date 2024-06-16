#ifndef VS_ASSERT_H
#define VS_ASSERT_H

#include <sstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstdlib>

#define ASSERTION(expr, except_type, msg)    \
    {                                           \
        if (!static_cast<bool>(expr)) {                    \
            std::stringstream ss;   \
            ss << __FILE__ ":" << __LINE__ << ": " << msg;  \
            throw except_type(ss.str());                \
        }                               \
    }

#define PANIC(expr)    \
    {                                           \
        if (!static_cast<bool>(expr)) {                    \
            std::stringstream ss;   \
            ss << "PANIC: " << __FILE__ ":" << __LINE__;  \
            std::cerr << ss.str() << std::endl; \
            std::abort();   \
        }                               \
    }

#endif //! VS_ASSERT_H
