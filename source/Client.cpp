//
// Created by soham on 11/8/20.
//

#include "Client.h"
#include "TreeEvaluator.h"

void Client::main() {
    COED::Encryptor encryptor = Client::createEncryptor();

    helib::Ctxt ctxt_result = Client::send_input_vector(encryptor);

    debugN(encryptor, ctxt_result, ">> Result :", 16);

    std::cout << ">> Decimal result: " << get_decimal_from_binary(encryptor, ctxt_result) << "\n";
}

/**
 * Creates an input vector for the client and sends it to the server.
 * For demonstration purposes, currently this function only calls the server's method since they're both on the same
 * machine. However, this  method can be just as easily changed to pass the input vector over a network.
 *
 * @param encryptor an address of the encryptor object used to encrypt/decrypt a ciphertext.
 * @return The value that the server sent.
 */
helib::Ctxt Client::send_input_vector(COED::Encryptor &encryptor) {
    helib::Context *context = encryptor.getContext();
    helib::PubKey *pubKey = encryptor.getPublicKey();

    int inputs[3];
    std::cout << "Enter 3 feature vectors. Hit enter after each." << std::endl;
    for (int &input : inputs) {
        std::cin >> input;
    }

    std::cout << "Calculating result..." << std::endl;

    helib::Ctxt ctxt_input_vector[3] = {helib::Ctxt(*pubKey), helib::Ctxt(*pubKey), helib::Ctxt(*pubKey)};
    TreeEvaluator::getCtxtList(*context, *pubKey, ctxt_input_vector, inputs);

    helib::Ctxt ctxt_result = TreeEvaluator::evaluate_decision_tree(ctxt_input_vector, *(encryptor.getPublicKey()),
                                                                    *encryptor.getContext());

    return ctxt_result;
}

/**
 * Decrypts a ciphertext and prints the output. Use only for debugging/demonstrations.
 * @param enc A COED::Encryptor object that can encrypt and decrypt ciphertexts.
 * @param ctxt The ciphertext to be decrypted and printed.
 * @param msg An optional message that is prepended to the output.
 * @param n The number of 'slots' of the ciphertext to be printed.
 *
 */
void Client::debugN(const COED::Encryptor &enc, const helib::Ctxt &ctxt, const std::string &msg, int n) {
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

/**
 * Creates an encryptor object can be used to either encrypt or decrypt a plaintext.
 * @return the created object.
 */
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

/**
 * Given a ciphertext representing some number in binary format, decrypts it and calculates the decimal result.
 * @param enc the encryptor object used to decrypt the ciphertext.
 * @param result The evaluation result sent by the server.
 * @return A decimal representation of the result.
 */
double Client::get_decimal_from_binary(const COED::Encryptor &enc, const helib::Ctxt &result) {
    std::vector<long> plaintext(enc.getEncryptedArray()->size());
    enc.getEncryptedArray()->decrypt(result, *enc.getSecretKey(), plaintext);
    double decimal_result = 0;

    for (int i = 0; i < 16; ++i) {
        double power = pow(2, 15 - i);
        decimal_result += power * plaintext[i];
    }
    return decimal_result;

}
