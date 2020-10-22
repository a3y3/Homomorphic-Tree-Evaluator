//
// Copyright SpiRITlab - Computations on Encrypted Data
// https://gitlab.com/SpiRITlab/coed
//


#include <iostream>
#include "BasicExamples.h"

int main() {
    std::cout << "Program Start!!!" << std::endl;

    //BasicExamples::determine_plaintext_slot();

    //BasicExamples::decimal_arithmetic_example();

    //BasicExamples::packed_binary_arithmetic_example();

    //BasicExamples::packed_decimal_arithmetic_example();

    BasicExamples::packed_decimal_arithmetic_scalar_product_example();

    std::cout << "Program Finished!!!" << std::endl;
    return 0;
}
