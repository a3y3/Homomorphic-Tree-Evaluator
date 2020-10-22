//
// Copyright SpiRITlab - Computations on Encrypted Data
// https://gitlab.com/SpiRITlab/coed
//

#include "Encryptor.h"

#include <iostream>
#include <helib/binaryArith.h>
#include "FileSystem.h"
#include "assert.h"

COED::Encryptor::Encryptor(const std::string &secret_key_file_path, const std::string &public_key_file_path,
                           long plaintextModulus, long phiM, long lifting, long numOfBitsOfModulusChain,
                           long numOfColOfKeySwitchingMatrix)
        : plaintextModulus(plaintextModulus), phiM(phiM), lifting(lifting),
          numOfBitsOfModulusChain(numOfBitsOfModulusChain),
          numOfColOfKeySwitchingMatrix(numOfColOfKeySwitchingMatrix) {

    std::cout << "Initialising context object..." << std::endl;
    context = new helib::Context(phiM, plaintextModulus, lifting);

    std::cout << "Building modulus chain..." << std::endl;
    buildModChain(*context, numOfBitsOfModulusChain, numOfColOfKeySwitchingMatrix);

    COED::FileSystem sk_fs(secret_key_file_path);
    sk_fs.open_output_stream(std::fstream::trunc);
    std::ofstream &sk_fs_of = sk_fs.get_output_stream();

    COED::FileSystem pk_fs(public_key_file_path);
    pk_fs.open_output_stream(std::fstream::trunc);
    std::ofstream &pk_fs_of = pk_fs.get_output_stream();

    writeContextBase(sk_fs_of, *(this->context));
    writeContextBase(pk_fs_of, *(this->context));
    sk_fs_of << *(this->context) << std::endl;
    pk_fs_of << *(this->context) << std::endl;

    // Print the context
    context->zMStar.printout();
    std::cout << std::endl;

    // Print the security level
    std::cout << "Security: " << context->securityLevel() << std::endl;

    // Secret key management
    std::cout << "Creating secret key..." << std::endl;
    // Create a secret key associated with the context
    secret_key = new helib::SecKey(*context);
    // Generate the secret key
    secret_key->GenSecKey();
    std::cout << "Generating key-switching matrices..." << std::endl;
    // Compute key-switching matrices that we need
    helib::addSome1DMatrices(*secret_key);

    // Public key management
    // Set the secret key (upcast: SecKey is a subclass of PubKey)
    public_key = secret_key;
    sk_fs_of << *(secret_key) << std::endl;
    pk_fs_of << *(public_key) << std::endl;

    // Get the EncryptedArray of the context
    encrypted_array = new helib::EncryptedArray(*context);
    sk_fs.close_output_stream();
    pk_fs.close_output_stream();
}

COED::Encryptor::Encryptor(const std::string &private_key_file_path, const std::string &public_key_file_path,
                           long plaintextModulus, long lifting, long numOfBitsOfModulusChain,
                           long numOfColOfKeySwitchingMatrix, long desiredSlotCount, long securityLevel)
        : plaintextModulus(plaintextModulus), lifting(lifting), numOfBitsOfModulusChain(numOfBitsOfModulusChain),
          numOfColOfKeySwitchingMatrix(numOfColOfKeySwitchingMatrix), desiredSlotCount(desiredSlotCount),
          securityLevel(securityLevel) {
    /**
    * @brief Returns smallest parameter m satisfying various constraints:
    * @param k security parameter
    * @param L number of levels
    * @param c number of columns in key switching matrices
    * @param p characteristic of plaintext space
    * @param d embedding degree (d ==0 or d==1 => no constraint)
    * @param s at least that many plaintext slots
    * @param chosen_m preselected value of m (0 => not preselected)
    * Fails with an error message if no suitable m is found
    * prints an informative message if verbose == true
    **/
    //long FindM(long k, long nBits, long c, long p, long d, long s, long chosen_m, bool verbose=false);
    phiM = helib::FindM(securityLevel, numOfBitsOfModulusChain, numOfColOfKeySwitchingMatrix, plaintextModulus, 0,
                        desiredSlotCount, 0, false);
    Encryptor(private_key_file_path, public_key_file_path, plaintextModulus, phiM, lifting, numOfBitsOfModulusChain,
              numOfColOfKeySwitchingMatrix);
}

COED::Encryptor::Encryptor(const std::string &secret_key_file_path, const std::string &public_key_file_path) {
    COED::FileSystem sk_fs(secret_key_file_path);
    sk_fs.open_input_stream();
    std::ifstream &sk_fs_if = sk_fs.get_input_stream();

    COED::FileSystem pk_fs(public_key_file_path);
    pk_fs.open_input_stream();
    std::ifstream &pk_fs_if = pk_fs.get_input_stream();

    assert(sk_fs_if.is_open());
    assert(pk_fs_if.is_open());

    unsigned long m, p, r;
    std::vector<long> gens, ords;
    helib::readContextBase(sk_fs_if, m, p, r, gens, ords);
    helib::readContextBase(pk_fs_if, m, p, r, gens, ords);
    context = new helib::Context(m, p, r, gens, ords);

    sk_fs_if >> *context;
    secret_key = new helib::SecKey(*context);
    sk_fs_if >> *secret_key;

    pk_fs_if >> *context;
    public_key = new helib::PubKey(*context);
    pk_fs_if >> *public_key;

    sk_fs.close_input_stream();
    pk_fs.close_input_stream();
}

COED::Encryptor::~Encryptor() {
//    if(context != nullptr)
//        delete context;
//    if(secret_key != nullptr)
//        delete secret_key; // don't need to delete public_key because it is the same as secret_key.
}

void
COED::Encryptor::testEncryption() {
    // Get the number of slot (phi(m))
    long nslots = encrypted_array->size();
    std::cout << "Number of slots: " << nslots << std::endl;

    long bitSize = 16;

    // generate a random number with a specified bitsize
    long a_data = 8; //NTL::RandomBits_long(bitSize);
    long b_data = 3;
    long c_data = 4;

    // create a ciphertext
    helib::Ctxt ciphertext(*public_key);

    // create a vector with a set slot count for plaintexts
    std::vector<long> plaintext(encrypted_array->size());

    // put a_data into the available slots and 0 padding
    long i = 0;
    std::cout << "input bit vector: ";
    for (auto &slot : plaintext) {
        if (i < bitSize) {
            slot = (a_data >> i) & 1;
            i++;
        } else if (i >= bitSize && i < bitSize * 2) {
            slot = (b_data >> (i % bitSize)) & 1;
            i++;
        } else if (i >= bitSize * 2 && i < bitSize * 3) {
            slot = (c_data >> (i % bitSize)) & 1;
            i++;
        } else {
            slot = 0;
        }
        std::cout << slot;
    }
    std::cout << std::endl;

    // packed ciphertext of data
    encrypted_array->encrypt(ciphertext, *public_key, plaintext);

    std::vector<long> dec_plaintext(encrypted_array->size());
    encrypted_array->decrypt(ciphertext, *secret_key, dec_plaintext);
    std::cout << "decrypted bit vector: ";
    for (auto &slot : dec_plaintext) {
        std::cout << slot;
    }
    std::cout << std::endl;

    encrypted_array->rotate(ciphertext, 1);
    encrypted_array->decrypt(ciphertext, *secret_key, dec_plaintext);
    std::cout << "rotated bit vector: ";
    for (auto &slot : dec_plaintext) {
        std::cout << slot;
    }
    std::cout << std::endl;
}



helib::Context *
COED::Encryptor::getContext() const {
    return context;
}

helib::SecKey *
COED::Encryptor::getSecretKey() const {
    return secret_key;
}

helib::PubKey *
COED::Encryptor::getPublicKey() const {
    return public_key;
}

helib::EncryptedArray *
COED::Encryptor::getEncryptedArray() const {
    return encrypted_array;
}

int
COED::Encryptor::getSlotCount() {
    return this->encrypted_array->size();
}
