#ifndef TESTUTILS_CPPUNIT_H
#define TESTUTILS_CPPUNIT_H

#include "./testutils.h"

#include "../application/commandlineutils.h"
#include "../io/ansiescapecodes.h"

#include <cppunit/TestPath.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <iostream>

using namespace CppUtilities;

/*!
 * \brief Prints the names of all child tests of the specified \a test.
 */
void printTestNames(CPPUNIT_NS::Test *test, Indentation indentation)
{
    for (int index = 0, count = test->getChildTestCount(); index != count; ++index) {
        const auto childTest = test->getChildTestAt(index);
        std::cerr << '\n' << indentation << " - " << childTest->getName();
        printTestNames(childTest, indentation + 4);
    }
}

/*!
 * \brief Performs unit tests using cppunit.
 */
int main(int argc, char **argv)
{
    TestApplication testApp(argc, argv);
    if (!testApp) {
        return -1;
    }

    // list tests
    CPPUNIT_NS::TestFactoryRegistry &registry = CPPUNIT_NS::TestFactoryRegistry::getRegistry();
    if (testApp.onlyListUnits()) {
        std::cerr << "Available tests:";
        printTestNames(registry.makeTest(), Indentation(0));
        std::cerr << '\n';
        return 0;
    }

    // run tests
    CPPUNIT_NS::TextUi::TestRunner runner;
    if (!testApp.unitsSpecified() || testApp.units().empty()) {
        // no units specified -> test all
        runner.addTest(registry.makeTest());
    } else {
        // pick specified units from overall test
        CPPUNIT_NS::Test *overallTest = registry.makeTest();
        std::vector<const char *> unavailableUnits;
        for (const char *unit : testApp.units()) {
            try {
                runner.addTest(overallTest->findTest(unit));
            } catch (const std::invalid_argument &) {
                unavailableUnits.emplace_back(unit);
            }
        }
        if (!unavailableUnits.empty()) {
            std::cerr << "The following tests specified via --unit are not available:";
            for (const char *unitName : unavailableUnits) {
                std::cerr << "\n - " << unitName;
            }
            std::cerr << "\nAvailable tests:";
            printTestNames(overallTest, Indentation(0));
            std::cerr << '\n';
            return -1;
        }
    }
    std::cerr << EscapeCodes::TextAttribute::Bold << "Executing test cases ..." << EscapeCodes::Phrases::EndFlush;
    const auto ok = runner.run(std::string(), false);
    std::cerr << (ok ? "Tests successful\n" : "Tests failed\n");
    return !ok;
}

#endif // TESTUTILS_CPPUNIT_H
