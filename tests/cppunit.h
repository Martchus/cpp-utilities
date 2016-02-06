#ifndef CPPUNIT_H
#define CPPUNIT_H

#include "./testutils.h"

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <iostream>

using namespace std;
using namespace TestUtilities;
using namespace CPPUNIT_NS;

/*!
 * \brief Performs unit tests using cppunit.
 */
int main(int argc, char **argv)
{
    TestApplication testApp(argc, argv);
    if(testApp) {
        // run tests
        TextUi::TestRunner runner;
        TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
        runner.addTest(registry.makeTest());
        return !runner.run(string(), false);
    } else {
        return -1;
    }
}

#endif // CPPUNIT_H
