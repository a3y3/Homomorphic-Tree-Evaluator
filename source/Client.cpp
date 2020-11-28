//
// Created by soham on 11/8/20.
//

#include "Client.h"
#include "TreeEvaluator.h"

void Client::main() {
    COED::Encryptor encryptor = Client::createEncryptor();

    helib::Ctxt ctxt_result = Client::send_input_vector(encryptor);

    debugN(encryptor, ctxt_result, ">> Result :", 16);
    // debugN(encryptor, ctxt_test, ">> Test :", 16);
}


helib::Ctxt Client::send_input_vector(COED::Encryptor &encryptor) {

    helib::Ptxt<helib::BGV> ptxt_input_vector(*(encryptor.getContext()));
    helib::Ctxt ctxt_input_vector(*(encryptor.getPublicKey()));
    ptxt_input_vector[0] = 10;
    ptxt_input_vector[1] = 12;
    ptxt_input_vector[2] = 18;
    ptxt_input_vector[3] = 15;

    encryptor.getPublicKey()->Encrypt(ctxt_input_vector, ptxt_input_vector);
    Client::print_result(ctxt_input_vector, encryptor);

    helib::Ctxt ctxt_result = TreeEvaluator::evaluate_decision_tree(ctxt_input_vector, *(encryptor.getPublicKey()), *encryptor.getContext());
    // debugN(encryptor, ctxt_result, ">> Result :", 1);
    return ctxt_result;
}


void Client::print_result(const helib::Ctxt &result, const COED::Encryptor &encryptor) {
    std::vector<long> ptxt_result(encryptor.getEncryptedArray()->size());
    encryptor.getEncryptedArray()->decrypt(result, *encryptor.getSecretKey(), ptxt_result);
    std::cout << "Result: " << ptxt_result[0] << std::endl;
}


void Client::debugN(COED::Encryptor enc, helib::Ctxt ctxt, const std::string &msg, int n){
    // std::cout<<">>>>> 3333" <<std::endl;
    std::vector<long> plaintext(enc.getEncryptedArray()->size());
    // decrypts the ctxt
    
    enc.getEncryptedArray()->decrypt(ctxt, *enc.getSecretKey(), plaintext);
    std::cout << msg;
    // Prints plain text values
    
    for (size_t i = 0; i < n; i++)    
        std::cout << plaintext[i] << " ";
    std::cout << std::endl;
}


COED::Encryptor Client::createEncryptor() {
    int plaintext_prime_modulus = 2;
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
