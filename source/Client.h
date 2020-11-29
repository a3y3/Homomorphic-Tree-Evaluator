//
// Created by soham on 11/8/20.
// Modified by Akshay on 11/28/20
//

#ifndef HOMOMORPHICTREEEVALUATOR_CLIENT_H
#define HOMOMORPHICTREEEVALUATOR_CLIENT_H

#include "Encryptor.h"
#include "Util.h"

class Client {
public:
    static void main();

private:
    static COED::Encryptor createEncryptor();

    static helib::Ctxt send_input_vector(COED::Encryptor &encryptor);

    static double get_decimal_from_binary(const COED::Encryptor &enc, const helib::Ctxt &result);

public:
    static void debugN(const COED::Encryptor &enc, const helib::Ctxt &ctxt, const std::string &msg, int n);
};


#endif //HOMOMORPHICTREEEVALUATOR_CLIENT_H
