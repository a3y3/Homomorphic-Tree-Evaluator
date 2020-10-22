//
// Copyright SpiRITlab - Computations on Encrypted Data
// https://gitlab.com/SpiRITlab/coed
//

#ifndef HELIBEXAMPLES_BASICEXAMPLES_H
#define HELIBEXAMPLES_BASICEXAMPLES_H

#include "Encryptor.h"

class BasicExamples {
#define BIT_SIZE 16
public:
    static void determine_plaintext_slot();
    static void decimal_arithmetic_example();
    static void packed_binary_arithmetic_example();
    static void packed_decimal_arithmetic_example();
    static void packed_decimal_arithmetic_scalar_product_example();

private:
    static void fill_plaintext_with_bits(helib::Ptxt<helib::BGV> &plaintext, const long data);
    static void print_vector(const std::string &msg, const std::vector<long> &vec, bool space=false);
    static void debug(const COED::Encryptor &encryptor, const helib::Ctxt &ctxt, const std::string &msg, bool space=false);
    static void fill_plaintext_with_numbers(std::vector<long> &plaintext, const long maxValue, bool reverse=false);
};


#endif //HELIBEXAMPLES_BASICEXAMPLES_H
