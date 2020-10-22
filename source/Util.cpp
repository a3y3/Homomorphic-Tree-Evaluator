//
// Copyright SpiRITlab - Computations on Encrypted Data
// https://gitlab.com/SpiRITlab/coed
//

#include "Util.h"

void COED::Util::debug(const std::string &s) {
    std::cout << "[DEBUG] " << s << std::endl;
}


void COED::Util::info(const std::string &s) {
    std::cout << "[INFO] " << s << std::endl;
}

void COED::Util::error(const std::string &s) {
    std::cerr << "[ERROR] " << s << std::endl;
}