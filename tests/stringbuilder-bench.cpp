#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"
#include "../chrono/datetime.h"

#include <vector>
#include <iostream>
#include <sstream>

using namespace std;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

int main()
{
    cout << "Benchmarking string builder vs. stringstream" << endl;

    constexpr unsigned int iterations = 500000;
    constexpr unsigned int iterations2 = 50;

    vector<string> v1, v2;
    v1.reserve(iterations);
    v2.reserve(iterations);

    DateTime t1 = DateTime::now();
    for(unsigned int r = 0; r < iterations2; ++r) {
        for(unsigned int i = 0; i < iterations; ++i) {
            stringstream ss;
            v1.emplace_back("left. " + numberToString(i + 1) + "; right: " + numberToString(i + 2) + "; top: " + numberToString(i + 3) + "; bottom: " + numberToString(i + 4) + ';');
        }
        v1.clear();
    }
    DateTime t2 = DateTime::now();
    cout << "plus operator: " << (t2 - t1).toString(TimeSpanOutputFormat::Normal, true) << endl;

    t1 = DateTime::now();
    for(unsigned int r = 0; r < iterations2; ++r) {
        for(unsigned int i = 0; i < iterations; ++i) {
            stringstream ss;
            ss << "left: " << (i + 1) << "; right: " << (i + 2) << "; top: " << (i + 3) << "; bottom: " << (i + 4) << ';';
            v1.emplace_back(ss.str());
        }
        v1.clear();
    }
    t2 = DateTime::now();
    const TimeSpan diff1 = t2 - t1;
    cout << "stringstream: " << diff1.toString(TimeSpanOutputFormat::Normal, true) << endl;

    t1 = DateTime::now();
    for(unsigned int r = 0; r < iterations2; ++r) {
        for(unsigned int i = 0; i < iterations; ++i) {
            v2.emplace_back("left. " % numberToString(i + 1) % "; right: " % numberToString(i + 2) % "; top: " % numberToString(i + 3) % "; bottom: " % numberToString(i + 4) + ';');
        }
        v2.clear();
    }
    t2 = DateTime::now();
    const TimeSpan diff2 = t2 - t1;
    cout << "string builder: " << diff2.toString(TimeSpanOutputFormat::Normal, true) << endl;

    v1.swap(v2);

    cout << "diff (stringstream minus string builder): " << (diff1 - diff2).toString(TimeSpanOutputFormat::Normal, true) << endl;
    cout << "factor (stringstream / string builder): " << (static_cast<double>(diff1.totalTicks()) / diff2.totalTicks()) << endl;

    return 0;
}
