#ifndef FAILURE_H
#define FAILURE_H

#include "global.h"

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

#endif // FAILURE_H
