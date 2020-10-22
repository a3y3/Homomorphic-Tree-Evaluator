//
// Copyright SpiRITlab - Computations on Encrypted Data
// https://gitlab.com/SpiRITlab/coed
//

#ifndef HELIBEXAMPLES_UTIL_H
#define HELIBEXAMPLES_UTIL_H

#include <bitset>
#include <iostream>
#include <string>

namespace COED {
    class Util {
    public:
        static void debug(const std::string &s);

        static void info(const std::string &s);

        static void error(const std::string &s);
    };
}

#endif //HELIBEXAMPLES_UTIL_H
