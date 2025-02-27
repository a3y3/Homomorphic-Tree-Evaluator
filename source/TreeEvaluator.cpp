//
// Created by soham on 10/29/20.
//

#include "TreeEvaluator.h"

/**
 * Given an x, stores the binary representation of x in bin. Not that bin[0] contains the MSB and bin[n] contains the
 * LSB of the binary number.
 * @param x An input number whose binary representation is to be found.
 * @param bin The array that stores the binary representation.
 */
void getBin(int x, int *bin) {
    int flag = 0;
    if (x < 0) {
        flag = 1;
        x *= -1;
    }

    for (int i = 15; i >= 0; i--) {
        bin[i] = x % 2;
        x /= 2;
    }

    int carry = 0;
    if (bin[15] == 0) {
        carry = 1;
    }

    if (flag == 1) {
        for (int i = 14; i >= 0; i--) {
            bin[i] = (1 - bin[i]) + carry;
            carry = 0;
            if (bin[i] > 1) {
                bin[i] = 0;
                carry = 1;
            }
        }
    }
}

/**
 * Depending upon the choice provided, returns one of the following ciphertexts:
 *      1. a mask of 100...0 (used for selecting only the first slot of a ciphertext)
 *      2. a ciphertext for storing either a sum or a carry.
 *      3. a ciphertext that contains the binary representation of {@code val}.
 * @param i Used as a switch. Can be either 1, 2, 3, or 4.
 * @param context the address of the helib::context object
 * @param pubkey the address client's public key.
 * @param val the value which is to be encoded as binary in a ciphertext.
 * @return the created ciphertext.
 */
helib::Ctxt TreeEvaluator::getCtxt(int i, helib::Context &context, helib::PubKey &pubkey, int val) {
    // Create a Mask
    if (i == 0) {
        helib::Ptxt<helib::BGV> ptxt_mask(context);
        helib::Ctxt mask = helib::Ctxt(pubkey);

        ptxt_mask[0] = 1;

        (&pubkey)->Encrypt(mask, ptxt_mask);
        return mask;

    } else if (i == 1 || i == 2) {
        // Create a ciphertext for carry/sum.
        helib::Ptxt<helib::BGV> ptxt(context);
        helib::Ctxt ctxt = helib::Ctxt(pubkey);

        (&pubkey)->Encrypt(ctxt, ptxt);
        return ctxt;
    } else {
        int y[16];
        getBin(val, y);

        helib::Ptxt<helib::BGV> yPtxt(context);
        helib::Ctxt yCtxt = helib::Ctxt(pubkey);

        for (int index = 0; index < 16; index++) {
            yPtxt[index] = y[index];
        }

        (&pubkey)->Encrypt(yCtxt, yPtxt);
        return yCtxt;
    }
}

/**
 * Given an array of values (in @code *val), this function stores the ciphertexts formed from the array elements in
 * {@code *nodes}.
 *
 * @param context An object of helib::context
 * @param pubkey the address of the client's public key.
 * @param nodes An array of ciphertext which will store the encrypted array elements of {@code *val}
 * @param val An array of integer values, which is to be stored in binary form in a ciphertext.
 */
void TreeEvaluator::getCtxtList(helib::Context &context, helib::PubKey &pubkey, helib::Ctxt *nodes, int *val) {
    nodes[0] = TreeEvaluator::getCtxt(3, context, pubkey, val[0]);
    nodes[1] = TreeEvaluator::getCtxt(3, context, pubkey, val[1]);
    nodes[2] = TreeEvaluator::getCtxt(3, context, pubkey, val[2]);
}


/**
 * In a client-server setting, the client will call this function without any knowledge about the decision tree.
 * All the client does is supply homomorphically encrypted input vectors, and the server evaluates and sends back the
 * encrypted result.
 * This function is merely a wrapper that hides the actual functionality by delegating it to another function.
 *
 * @param input_vector encrypted input vector.
 * @return an encrypted result obtained after the evaluation of the tree.
 */
helib::Ctxt TreeEvaluator::evaluate_decision_tree(helib::Ctxt input_vector[], helib::PubKey &pubkey, helib::Context
&context) {
    int leaves[3] = {10, 20, 30};
    helib::Ctxt leaf_nodes[3] = {helib::Ctxt(pubkey), helib::Ctxt(pubkey), helib::Ctxt(pubkey)};
    TreeEvaluator::getCtxtList(context, pubkey, leaf_nodes, leaves);

    // Module 2: Stores ctxt versions of thresholds in threshold
    // We use modified dummy node, sepal_length < 999 (always the
    // case), instead of sepal_length > 0 
    int thold[3] = {-27, -17, -999};
    helib::Ctxt thresholds[3] = {helib::Ctxt(pubkey), helib::Ctxt(pubkey), helib::Ctxt(pubkey)};
    TreeEvaluator::getCtxtList(context, pubkey, thresholds, thold);

    helib::Ctxt ctxt_1(pubkey);
    helib::Ptxt<helib::BGV> ptxt_input(context);
    ptxt_input[0] = 1;
    pubkey.Encrypt(ctxt_1, ptxt_input);
    helib::EncryptedArray ea(context);
    helib::totalSums(ea, ctxt_1);

    helib::Ctxt decisions[3] = {helib::Ctxt(pubkey), helib::Ctxt(pubkey), helib::Ctxt(pubkey)};
    int size = sizeof(thresholds) / sizeof(thresholds[0]);
    for (int i = 0; i < size; i++) {
        decisions[i] = TreeEvaluator::compareCtxt(input_vector[i], thresholds[i], context, pubkey);
    }

    return TreeEvaluator::calculate_result(decisions, leaf_nodes, ctxt_1);
}

/**
 * Compares two ciphertexts and returns the result.
 * This method compares using 2's complement. If x<y, x-y has '1' as an MSB. The method subtracts the numbers this
 * way, and returns a ciphertext such that it has all 1s if x<y, and all 0s otherwise.
 * @param xCtxt The first ciphertext to be compared.
 * @param yCtxt The second ciphertext to be compared.
 * @param context An address of helib::context object.
 * @param pubkey Address of the client's public key.
 * @return An encryption of x<y.
 */
helib::Ctxt
TreeEvaluator::compareCtxt(helib::Ctxt xCtxt, helib::Ctxt yCtxt, helib::Context &context, helib::PubKey &pubkey) {

    const int bitLength = 16;

    helib::Ctxt mask = TreeEvaluator::getCtxt(0, context, pubkey, 0);
    helib::Ctxt carry = TreeEvaluator::getCtxt(1, context, pubkey, 0);
    helib::Ctxt sum = TreeEvaluator::getCtxt(2, context, pubkey, 0);

    for (int i = 0; i < bitLength; i++) {

        sum = xCtxt;
        sum += yCtxt;

        carry = xCtxt;
        carry *= yCtxt;

        helib::EncryptedArray ea(context);
        ea.rotate(carry, -1);

        xCtxt = sum;
        yCtxt = carry;
    }
    sum *= mask;
    helib::EncryptedArray ea(context);
    helib::totalSums(ea, sum);
    return sum;
}


/**
 * This is an internal, private function to TreeEvaluator.
 * Note that the structure of the actual decision tree is simply a polynomial formula. There isn't a need to store the
 * actual tree in memory. Unfortunately, this means this particular function is sensitive to the structure of tree,
 * ie. it's not a generic function. It is only designed to work with a particular, pre defined tree (see README.md)
 *
 * @param decisions an array of encrypted decisions. Each element in this array is a ciphertext obtained from SecComp.
 * @return a single ciphertext that is the result of evaluation of the tree.
 */
helib::Ctxt
TreeEvaluator::calculate_result(helib::Ctxt decisions[], helib::Ctxt leaf_nodes[], const helib::Ctxt &ctxt_1) {
    // calculate decision[0]*(decision[2]*leaf_nodes[0]) (call it term0)
    helib::Ctxt temp(decisions[2]);
    temp.multiplyBy(leaf_nodes[0]);
    helib::Ctxt term0(decisions[0]);
    term0.multiplyBy(temp);


    // calculate 1-decision[0] (call it term1)
    helib::Ctxt one_minus_ctxt_0(ctxt_1);
    one_minus_ctxt_0.addCtxt(decisions[0], true);
    helib::Ctxt term1(one_minus_ctxt_0); // the only point of this variable is for readability's sake

    // calculate (1-decision[1])*leaf_nodes[2] (call it term2)
    helib::Ctxt one_minus_ctxt_1(ctxt_1);
    one_minus_ctxt_1.addCtxt(decisions[1], true);
    one_minus_ctxt_1.multiplyBy(leaf_nodes[2]);
    helib::Ctxt term2(one_minus_ctxt_1);

    // calculate decision[1]*leaf_nodes[1] (call it term3)
    helib::Ctxt term3(decisions[1]);
    term3.multiplyBy(leaf_nodes[1]);


    // calculate term2+term3 (call it term4)
    term2.addCtxt(term3);
    helib::Ctxt term4(term2);

    // multiply term4 with term1 (call it term5)
    term4.multiplyBy(term1);
    helib::Ctxt term5(term4);

    /*Finally, add terms 0 and 5, effectively calculating the following expression:
     * ctxt[0]*((1 - ctxt[2]) * 0 + ctxt[2] * ctxt_leaves[0]) + (1-ctxt[0]) * ((1-ctxt[1])*ctxt_leaves[1] + ctxt[1]*ctxt_leaves[2])
     * */
    term5.addCtxt(term0);

    return term5;
}
