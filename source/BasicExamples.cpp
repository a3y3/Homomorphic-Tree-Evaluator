//
// Copyright SpiRITlab - Computations on Encrypted Data
// https://gitlab.com/SpiRITlab/coed
//

#include "BasicExamples.h"
#include <iostream>
#include "Util.h"

void
BasicExamples::determine_plaintext_slot() {
    /*  Example of BGV scheme  */
    // Plaintext prime modulus
    unsigned long p = 53;
    // Cyclotomic polynomial - defines phi(m)
    unsigned long m = 0;
    // Hensel lifting (default = 1)
    unsigned long r = 1;
    // Number of bits of the modulus chain
    unsigned long bits =512; //4096;
    // Number of columns of Key-Switching matrix (default = 2 or 3)
    unsigned long c = 2;


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
    m = helib::FindM(80, bits, c, p, 0, 1, 0, false);

    std::cout << "Initialising context object..." << std::endl;
    // Intialise context
    helib::Context context(m, p, r);

    // Modify the context, adding primes to the modulus chain
    std::cout << "Building modulus chain..." << std::endl;
    buildModChain(context, bits, c);

    // Print the context
    context.zMStar.printout();
    std::cout << std::endl;

    // Print the security level
    std::cout << "Security: " << context.securityLevel() << std::endl;

    // Secret key management
    std::cout << "Creating secret key..." << std::endl;
    // Create a secret key associated with the context
    helib::SecKey secret_key(context);
    // Generate the secret key
    secret_key.GenSecKey();
    std::cout << "Generating key-switching matrices..." << std::endl;
    // Compute key-switching matrices that we need
    helib::addSome1DMatrices(secret_key);

    // Public key management
    // Set the secret key (upcast: SecKey is a subclass of PubKey)
    const helib::PubKey &public_key = secret_key;

    // Get the EncryptedArray of the context
    const helib::EncryptedArray &ea = *(context.ea);

    // Get the number of slot (phi(m))
    long nslots = ea.size();
    std::cout << "Number of slots: " << nslots << std::endl;

    std::cout << "getM()= " << ea.getContext().zMStar.getM() << std::endl;
}


void
BasicExamples::fill_plaintext_with_bits(helib::Ptxt<helib::BGV> &plaintext, const long data) {
    for(int i=0; i<BIT_SIZE; i++) {
        plaintext[i] = (data >> i) & 1;
    }
}

void
BasicExamples::print_vector(const std::string &msg, const std::vector<long> &vec, bool space) {
    std::cout << msg;
    for (auto &slot : vec) {
        std::cout << slot;
        if (space)
            std::cout << " ";
    }
    std::cout << std::endl;
}

void
BasicExamples::debug(const COED::Encryptor &encryptor, const helib::Ctxt &ctxt, const std::string &msg, bool space) {
    std::vector<long> plaintext(encryptor.getEncryptedArray()->size());
    encryptor.getEncryptedArray()->decrypt(ctxt, *encryptor.getSecretKey(), plaintext);
    print_vector(msg, plaintext, space);
}

void
BasicExamples::decimal_arithmetic_example() {
    unsigned long plaintext_prime_modulus = 53;
    unsigned long m = 26651;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 512;
    unsigned long numOfColOfKeySwitchingMatrix = 2;

    COED::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              m,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
    COED::Util::info("Finished creating encryptor.");

    // Create a vector of long with nslots elements
    helib::Ptxt<helib::BGV> ptxt(*(encryptor.getContext()));
    // Set it with numbers 0..nslots - 1
    for (int i = 0; i < ptxt.size(); ++i) {
        ptxt[i] = i;
    }

    // Print the plaintext
    std::cout << "Initial Plaintext: " << ptxt << std::endl;

    // Create a ciphertext
    helib::Ctxt ctxt(*(encryptor.getPublicKey()));
    // Encrypt the plaintext using the public_key
    encryptor.getPublicKey()->Encrypt(ctxt, ptxt);

    // Square the ciphertext
    ctxt.multiplyBy(ctxt);
    debug(encryptor,ctxt,"Ctxt: Square itself: ", true);
    // Plaintext version
    ptxt.multiplyBy(ptxt);
    std::cout << "Ptxt: Square itself: " << ptxt << std::endl;

    // Divide the ciphertext by itself
    // To do this we must calculate the multiplicative inverse using Fermat's
    // Little Theorem.  We calculate a^{-1} = a^{p-2} mod p, where a is non-zero
    // and p is our plaintext prime.
    // First make a copy of the ctxt using copy constructor
    helib::Ctxt ctxt_divisor(ctxt);
    // Raise the copy to the exponenet p-2
    ctxt_divisor.power(plaintext_prime_modulus - 2);
    // a^{p-2}*a = a^{-1}*a = a / a = 1;
    ctxt.multiplyBy(ctxt_divisor);
    debug(encryptor,ctxt,"Ctxt: Divide by itself: ", true);

    // Plaintext version
    helib::Ptxt<helib::BGV> ptxt_divisor(ptxt);
    ptxt_divisor.power(plaintext_prime_modulus - 2);
    ptxt.multiplyBy(ptxt_divisor);
    std::cout << "Ptxt: Divide by itself: " << ptxt << std::endl;

    // Double it (using additions)
    ctxt += ctxt;
    debug(encryptor,ctxt,"Ctxt: addition: ", true);
    // Plaintext version
    ptxt += ptxt;
    std::cout << "Ptxt: addition: " << ptxt << std::endl;

    // Subtract it from itself (result should be 0)
    ctxt -= ctxt;
    debug(encryptor,ctxt,"Ctxt: subtraction: ", true);
    // Plaintext version
    ptxt -= ptxt;
    std::cout << "Ptxt: substraction: " << ptxt << std::endl;

    // We can also add constants
    ctxt.addConstant(NTL::ZZX(1l));
    debug(encryptor,ctxt,"Ctxt: add constant(1): ", true);
    // Plaintext version
    ptxt.addConstant(NTL::ZZX(1l));
    std::cout << "Ptxt: add constant(1): " << ptxt << std::endl;

    // And multiply by constants
    ctxt *= NTL::ZZX(1l);
    debug(encryptor,ctxt,"Ctxt: multiply by constant(1): ", true);
    // Plaintext version
    ptxt *= NTL::ZZX(1l);
    std::cout << "Ptxt: multiply by constant(1): " << ptxt << std::endl;

    // We can also perform ciphertext-plaintext operations
    ctxt += ptxt;
    debug(encryptor,ctxt,"Ctxt: add ptxt: ", true);
}

void
BasicExamples::packed_binary_arithmetic_example() {
    unsigned long plaintext_prime_modulus = 2;
    unsigned long m = 21845;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 1024;
    unsigned long numOfColOfKeySwitchingMatrix = 2;

    // Note, Approx.RunningTime is primarily for generating context, modChain, secret key, key-switching matrix
    // PlaintextModulus: 2, Security: 128 (actual: 319.16), ModChainBits: 128, M: 21845,, Slots: 1024, Crypto Element Generation Time:: 4.051s  (overall time: )
    // PlaintextModulus: 2, Security: 128 (actual: 160.525), ModChainBits: 256, M: 21845, Slots: 1024, Crypto Element Generation Time:: 4.906s  (overall time: 5:57.59)
    // PlaintextModulus: 2, Security: 128 (actual: 128.945), ModChainBits: 512, M: 32767, Slots: 1800, Crypto Element Generation Time:: 11.372s (overall time: 11:47.79)
    // PlaintextModulus: 2, Security: 128 (actual: 141.383), ModChainBits: 768, M: 43691, Slots: 1285, Crypto Element Generation Time:: 30.793s (overall time: 8:41.00)
    // PlaintextModulus: 2, Security: 128 (actual: 137.705), ModChainBits: 1024, M: 55831, Slots: 2160, Crypto Element Generation Time:: 32.191s (overall time: 20:51.58)
    // PlaintextModulus: 2, Security: 128 (actual: 127.755), ModChainBits: 1280, M: 65537, Slots: 2048, Crypto Element Generation Time:: 1:09.06 (overall time: )
    // PlaintextModulus: 2, Security: 128 (actual: 144.998), ModChainBits: 1536, M: 92837, Slots: 1512, Crypto Element Generation Time:: 1:13.47 (overall time: )
    // PlaintextModulus: 2, Security: 128 (actual: 123.285), ModChainBits: 1792, M: 90751, Slots: 1200, Crypto Element Generation Time:: 1:38.51
    // PlaintextModulus: 2, Security: 128 (actual: 121.315), ModChainBits: 2048, M: 104353, Slots: 1440, Crypto Element Generation Time:: 2:17.11
    COED::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              m,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);

    // Get the number of slot (phi(m))
    long nslots = encryptor.getEncryptedArray()->size();
    std::cout << "Number of slots: " << nslots << std::endl;

    // =======================================================
    // Packing multiple numbers into one plaintext and encrypt
    // =======================================================
    // generate a random number with a specified bitsize
    long a_data = 8;
    long b_data = 3;
    long c_data = 4;

    // create a ciphertext
    helib::Ctxt ciphertext(*encryptor.getPublicKey());

    // create a vector with a set slot count for plaintexts
    std::vector<long> plaintext(nslots);

    // put a_data into the available slots and 0 padding
    long i = 0;
    std::cout << "input bit vector: ";
    for (auto &slot : plaintext) {
        if (i < BIT_SIZE) {
            slot = (a_data >> i) & 1;
            i++;
        } else if (i >= BIT_SIZE && i < BIT_SIZE * 2) {
            slot = (b_data >> (i % BIT_SIZE)) & 1;
            i++;
        } else if (i >= BIT_SIZE * 2 && i < BIT_SIZE * 3) {
            slot = (c_data >> (i % BIT_SIZE)) & 1;
            i++;
        } else {
            slot = 0;
        }
        std::cout << slot;
    }
    std::cout << std::endl;

    // packed ciphertext of data
    encryptor.getEncryptedArray()->encrypt(ciphertext, *encryptor.getPublicKey(), plaintext);
    debug(encryptor, ciphertext, "decrypted bit vector: ");

    encryptor.getEncryptedArray()->rotate(ciphertext, 1);
    debug(encryptor, ciphertext, "rotated bit vector: ");

    encryptor.getEncryptedArray()->shift(ciphertext, 1);
    debug(encryptor, ciphertext, "shifted bit vector: ");


    // =======================================================
    // pack binary into two ciphertext and apply arithmetics
    // =======================================================
    helib::Ptxt<helib::BGV> vec_1(*encryptor.getContext()), vec_2(*encryptor.getContext());
    fill_plaintext_with_bits(vec_1, 2);
    fill_plaintext_with_bits(vec_2, 4);

    helib::Ctxt vec_1_ctxt(*encryptor.getPublicKey()), vec_2_ctxt(*encryptor.getPublicKey());
    encryptor.getPublicKey()->Encrypt(vec_1_ctxt, vec_1);
    debug(encryptor, vec_1_ctxt, "vec_1 bit vector ");
    encryptor.getPublicKey()->Encrypt(vec_2_ctxt, vec_2);
    debug(encryptor, vec_2_ctxt, "vec_2 bit vector ");

    helib::Ctxt sum_ctxt(vec_1_ctxt);
    sum_ctxt.addCtxt(vec_2_ctxt);
    debug(encryptor, sum_ctxt, "Sum bit vector ");

    helib::Ctxt remainder_ctxt(vec_1_ctxt);
    remainder_ctxt.addCtxt(vec_2_ctxt, true);
    debug(encryptor, remainder_ctxt, "Remainder bit vector ");

    helib::Ctxt product_ctxt(vec_1_ctxt);
    product_ctxt.multiplyBy(vec_2_ctxt);
    debug(encryptor, product_ctxt, "Product bit vector ");

    encryptor.getEncryptedArray()->rotate(vec_1_ctxt, 1);
    debug(encryptor, vec_1_ctxt, "rotated vec_1 bit vector ");
}



void
BasicExamples::fill_plaintext_with_numbers(std::vector<long> &plaintext, const long maxValue, bool reverse) {
    if (!reverse)
        for(int i=0; i<=maxValue; ++i) {
            plaintext[i] = i;
        }
    else
        for(int i=maxValue-1; i>=0; --i) {
            plaintext[i] = i;
        }
}

void
BasicExamples::packed_decimal_arithmetic_example() {
    unsigned long plaintext_prime_modulus = 53;
    unsigned long m = 26651;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 512;
    unsigned long numOfColOfKeySwitchingMatrix = 2;

    COED::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              m,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
    COED::Util::info("Finished creating encryptor.");

    std::vector<long> vec_1(encryptor.getEncryptedArray()->size()), vec_2(encryptor.getEncryptedArray()->size());

    fill_plaintext_with_numbers(vec_1, 5);
    fill_plaintext_with_numbers(vec_2, 8);

    COED::Util::info("Finished creating plaintext.");
    helib::Ctxt vec_1_ctxt(*encryptor.getPublicKey()), vec_2_ctxt(*encryptor.getPublicKey());
    encryptor.getEncryptedArray()->encrypt(vec_1_ctxt, *encryptor.getPublicKey(), vec_1);
    debug(encryptor, vec_1_ctxt, "vec_1 vector: ", true);
    encryptor.getEncryptedArray()->encrypt(vec_2_ctxt, *encryptor.getPublicKey(), vec_2);
    debug(encryptor, vec_2_ctxt, "vec_2 vector: ", true);

    helib::Ctxt sum_ctxt(vec_1_ctxt);
    sum_ctxt.addCtxt(vec_2_ctxt);
    debug(encryptor, sum_ctxt, "Sum vector: ", true);

    helib::Ctxt remainder_ctxt(vec_1_ctxt);
    remainder_ctxt.addCtxt(vec_2_ctxt, true);
    debug(encryptor, remainder_ctxt, "Remainder vector: ", true);

    helib::Ctxt product_ctxt(vec_1_ctxt);
    product_ctxt.multiplyBy(vec_2_ctxt);
    debug(encryptor, product_ctxt, "Product vector: ", true);

    encryptor.getEncryptedArray()->rotate(vec_1_ctxt, 1);
    debug(encryptor, vec_1_ctxt, "rotated vec_1 vector: ", true);

    encryptor.getEncryptedArray()->shift(vec_1_ctxt, 1);
    debug(encryptor, vec_1_ctxt, "shifted vec_1 vector: ", true);
}

void
BasicExamples::packed_decimal_arithmetic_scalar_product_example() {
    unsigned long plaintext_prime_modulus = 53;
    unsigned long m = 26651;
    unsigned long lifting = 1;
    unsigned long numOfBitsOfModulusChain = 512;
    unsigned long numOfColOfKeySwitchingMatrix = 2;

    COED::Encryptor encryptor("/tmp/sk.txt", "/tmp/pk.txt",
                              plaintext_prime_modulus,
                              m,
                              lifting,
                              numOfBitsOfModulusChain,
                              numOfColOfKeySwitchingMatrix);
    COED::Util::info("Finished creating encryptor.");

    int size = 3;
    long Varray[]={1,2,3};
    long Uarray[]={4,5,6};

    NTL::ZZX Vpoly,Upoly;
    Vpoly.SetLength(size);
    Upoly.SetLength(size);

    for (int i = 0; i < size; ++i) {
        NTL::SetCoeff(Vpoly, i, Varray[i]);
        NTL::SetCoeff(Upoly, size-1-i, Uarray[i]);
    }
    std::cout << "Vpoly: " << Vpoly << std::endl;
    std::cout << "Upoly: " << Upoly << std::endl;
    COED::Util::info("Finished creating plaintext.");

    helib::Ctxt Vctxt(*encryptor.getPublicKey()), Uctxt(*encryptor.getPublicKey());
    NTL::ZZX Vpoly_dec, Upoly_dec;
    encryptor.getPublicKey()->Encrypt(Vctxt, Vpoly);
    encryptor.getSecretKey()->Decrypt(Vpoly_dec, Vctxt);
    std::cout << "dec(Vctxt)=" <<Vpoly_dec << std::endl;
    encryptor.getPublicKey()->Encrypt(Uctxt, Upoly);
    encryptor.getSecretKey()->Decrypt(Upoly_dec, Uctxt);
    std::cout << "dec(Uctxt)=" <<Upoly_dec << std::endl;

    helib::Ctxt product_ctxt(Vctxt);
    product_ctxt.multiplyBy(Uctxt);

    NTL::ZZX product_dec;
    encryptor.getSecretKey()->Decrypt(product_dec, product_ctxt);
    std::cout << "dec(product_ctxt)=" <<product_dec << std::endl;
}