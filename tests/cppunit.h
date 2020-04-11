#ifndef TESTUTILS_CPPUNIT_H
#define TESTUTILS_CPPUNIT_H

#include "./testutils.h"

#include "../application/commandlineutils.h"
#include "../io/ansiescapecodes.h"

#include <cppunit/TestPath.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <iostream>

using namespace std;
using namespace CppUtilities;
using namespace CPPUNIT_NS;

/*!
 * \brief Prints the names of all child tests of the specified \a test.
 */
void printTestNames(Test *test, Indentation indentation)
{
    for (int index = 0, count = test->getChildTestCount(); index != count; ++index) {
        const auto childTest = test->getChildTestAt(index);
        cerr << '\n' << indentation << " - " << childTest->getName();
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
    TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
    if (testApp.onlyListUnits()) {
        cerr << "Available tests:";
        printTestNames(registry.makeTest(), Indentation(0));
        cerr << '\n';
        return 0;
    }

    // run tests
    TextUi::TestRunner runner;
    if (!testApp.unitsSpecified() || testApp.units().empty()) {
        // no units specified -> test all
        runner.addTest(registry.makeTest());
    } else {
        // pick specified units from overall test
        Test *overallTest = registry.makeTest();
        vector<const char *> unavailableUnits;
        for (const char *unit : testApp.units()) {
            try {
                runner.addTest(overallTest->findTest(unit));
            } catch (const invalid_argument &) {
                unavailableUnits.emplace_back(unit);
            }
        }
        if (!unavailableUnits.empty()) {
            cerr << "The following tests specified via --unit are not available:";
            for (const char *unitName : unavailableUnits) {
                cerr << "\n - " << unitName;
            }
            cerr << "\nAvailable tests:";
            printTestNames(overallTest, Indentation(0));
            cerr << '\n';
            return -1;
        }
    }
    cerr << EscapeCodes::TextAttribute::Bold << "Executing test cases ..." << EscapeCodes::Phrases::EndFlush;
    const auto ok = runner.run(string(), false);
    cerr << (ok ? "Tests successful\n" : "Tests failed\n");
    return !ok;
}

#endif // TESTUTILS_CPPUNIT_H
