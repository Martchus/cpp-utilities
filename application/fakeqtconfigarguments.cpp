#include "./fakeqtconfigarguments.h"

namespace ApplicationUtilities {

FakeQtConfigArguments::FakeQtConfigArguments() :
    m_qtWidgetsGuiArg("qt-widgets-gui", "g", "shows a Qt widgets based graphical user interface (the application has not been built with Qt widgets support)"),
    m_qtQuickGuiArg("qt-quick-gui", "q", "shows a Qt quick based graphical user interface (the application has not been built with Qt quick support)")
{}

} // namespace ApplicationUtilities

