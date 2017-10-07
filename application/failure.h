#ifndef APPLICATION_UTILITIES_FAILURE_H
#define APPLICATION_UTILITIES_FAILURE_H

#include "../global.h"

#include <exception>
#include <iosfwd>
#include <string>

namespace ApplicationUtilities {

class CPP_UTILITIES_EXPORT Failure : public std::exception {
public:
    Failure();
    Failure(const std::string &what);
    ~Failure() USE_NOTHROW;

    virtual const char *what() const USE_NOTHROW;

private:
    std::string m_what;
};

CPP_UTILITIES_EXPORT std::ostream &operator<<(std::ostream &o, const Failure &failure);

} // namespace ApplicationUtilities

#endif // APPLICATION_UTILITIES_FAILURE_H
