//
// Created by soham on 11/8/20.
//

#include "Client.h"
#include "TreeEvaluator.h"

void Client::main() {
    COED::Encryptor encryptor = Client::createEncryptor();

    helib::Ctxt ctxt_result = Client::send_input_vector(encryptor);

    Client::print_result(ctxt_result, encryptor);
}

helib::Ctxt Client::send_input_vector(const COED::Encryptor &encryptor) {
    helib::Ptxt<helib::BGV> ptxt_input_vector(*(encryptor.getContext()));
    helib::Ctxt ctxt_input_vector(*(encryptor.getPublicKey()));
    ptxt_input_vector[0] = 10;
    ptxt_input_vector[1] = 12;
    ptxt_input_vector[2] = 18;
    ptxt_input_vector[3] = 15;

    encryptor.getPublicKey()->Encrypt(ctxt_input_vector, ptxt_input_vector);

    return TreeEvaluator::evaluate_decision_tree(ctxt_input_vector);
}

void Client::print_result(const helib::Ctxt &result, const COED::Encryptor &encryptor) {
    std::vector<long> ptxt_result(encryptor.getEncryptedArray()->size());
    encryptor.getEncryptedArray()->decrypt(result, *encryptor.getSecretKey(), ptxt_result);
    std::cout << "Result: " << ptxt_result[0] << std::endl;
}

COED::Encryptor Client::createEncryptor() {
    int plaintext_prime_modulus = 53;
    int phiM = 2665;
    int lifting = 1;
    int numOfBitsOfModulusChain = 512;
    int numOfColOfKeySwitchingMatrix = 2;
    COED::Util::info("Creating encryptor ...");

    COED::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              phiM,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
    COED::Util::info("Finished creating encryptor.");
    return encryptor;
}

