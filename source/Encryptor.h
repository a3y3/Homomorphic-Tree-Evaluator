//
// Copyright SpiRITlab - Computations on Encrypted Data
// https://gitlab.com/SpiRITlab/coed
//

#ifndef HELIBEXAMPLES_ENCRYPTOR_H
#define HELIBEXAMPLES_ENCRYPTOR_H

#include <iostream>
#include <helib/helib.h>

namespace COED {
    class Encryptor {
    public:
        Encryptor(const std::string &, const std::string &, long, long, long, long, long);

        Encryptor(const std::string &, const std::string &, long, long, long, long, long, long);

        Encryptor(const std::string &, const std::string &);

        ~Encryptor();

        void testEncryption();

        static void fill_plaintext(helib::Ptxt<helib::BGV> &, const std::vector<bool> &);

        helib::Context *getContext() const;

        helib::SecKey *getSecretKey() const;

        helib::PubKey *getPublicKey() const;

        helib::EncryptedArray *getEncryptedArray() const;

        int getSlotCount();

    private:
        // Plaintext prime modulus.
        long plaintextModulus = 2;
        // Cyclotomic polynomial - defines phi(m).
        long phiM = 0;
        // Hensel lifting (default = 1).
        long lifting = 1;
        // Number of bits of the modulus chain.
        long numOfBitsOfModulusChain = 500;
        // Number of columns of Key-Switching matrix (typically 2 or 3).
        long numOfColOfKeySwitchingMatrix = 2;
        // slot count
        long desiredSlotCount = 3000;
        // security level
        long securityLevel = 80;

        helib::Context *context;
        helib::SecKey *secret_key;
        helib::PubKey *public_key;
        helib::EncryptedArray *encrypted_array;
    };
}

#endif //HELIBEXAMPLES_ENCRYPTOR_H
