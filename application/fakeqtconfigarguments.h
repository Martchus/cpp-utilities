#ifndef APPLICATIONUTILITIES_FAKEQTCONFIGARGUMENTS_H
#define APPLICATIONUTILITIES_FAKEQTCONFIGARGUMENTS_H

#include "argumentparser.h"

namespace ApplicationUtilities {

class LIB_EXPORT FakeQtConfigArguments
{
public:
    FakeQtConfigArguments();

    Argument &qtWidgetsGuiArg();
    Argument &qtQuickGuiArg();

    bool areQtGuiArgsPresent() const;

private:
    Argument m_qtWidgetsGuiArg;
    Argument m_qtQuickGuiArg;
};

inline Argument &FakeQtConfigArguments::qtWidgetsGuiArg()
{
    return m_qtWidgetsGuiArg;
}

inline Argument &FakeQtConfigArguments::qtQuickGuiArg()
{
    return m_qtQuickGuiArg;
}

inline bool FakeQtConfigArguments::areQtGuiArgsPresent() const
{
    return m_qtWidgetsGuiArg.isPresent() || m_qtQuickGuiArg.isPresent();
}

} // namespace ApplicationUtilities

#ifndef QT_CONFIG_ARGUMENTS
#define QT_CONFIG_ARGUMENTS ApplicationUtilities::FakeQtConfigArguments
#endif

#endif // APPLICATIONUTILITIES_FAKEQTCONFIGARGUMENTS_H
