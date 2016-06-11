#ifndef APPLICATION_UTILITIES_FAILURE_H
#define APPLICATION_UTILITIES_FAILURE_H

#include "./global.h"

#include <exception>
#include <string>

namespace ApplicationUtilities {

class LIB_EXPORT Failure : public std::exception
{
public:
    Failure();
    Failure(const std::string &what);
    ~Failure() USE_NOTHROW;

    virtual const char *what() const USE_NOTHROW;

private:
    std::string m_what;
};

}

#endif // APPLICATION_UTILITIES_FAILURE_H
