#include "./fakeqtconfigarguments.h"

namespace ApplicationUtilities {

/*!
 * \class FakeQtConfigArguments
 * \brief The FakeQtConfigArguments class provides arguments for the Qt GUI used when
 *        the application hasn't been built with Qt GUI support.
 * \deprecated Get rid of this and simply don't add arguments for Qt GUI when disabled.
 */

/*!
 * \brief Constructs new fake Qt-config arguments.
 */
FakeQtConfigArguments::FakeQtConfigArguments()
    : m_qtWidgetsGuiArg(
          "qt-widgets-gui", 'g', "shows a Qt widgets based graphical user interface (the application has not been built with Qt widgets support)")
    , m_qtQuickGuiArg(
          "qt-quick-gui", 'q', "shows a Qt quick based graphical user interface (the application has not been built with Qt quick support)")
{
}

} // namespace ApplicationUtilities
