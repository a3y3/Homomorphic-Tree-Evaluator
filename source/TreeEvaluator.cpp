//
// Created by soham on 10/29/20.
//

#include "TreeEvaluator.h"

/**
 * In a client-server setting, the client will call this function without any knowledge about the decision tree.
 * All the client does is supply homomorphically encrypted input vectors, and the server evaluates and sends back the
 * encrypted result.
 * This function is merely a wrapper that hides the actual functionality by delegating it to another function.
 *
 * @param input_vector encrypted input vector.
 * @return an encrypted result obtained after the evaluation of the tree.
 */
helib::Ctxt TreeEvaluator::evaluate_decision_tree(helib::Ctxt input_vector) {
    helib::Ctxt decisions[] = {}; // TODO this should be a function call to secComp that returns an array of encrypted decisions.
    helib::Ctxt leaf_nodes[] = {}; // TODO this should be a function call that returns encrypted leaf nodes.
    helib::Ctxt ctxt_1 = helib::Ctxt(helib::PubKey()); // TODO this should be an encryption of 1.

    return TreeEvaluator::calculate_result(decisions, leaf_nodes, ctxt_1);
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
helib::Ctxt TreeEvaluator::calculate_result(helib::Ctxt decisions[], helib::Ctxt leaf_nodes[], helib:: Ctxt ctxt_1) {
    // calculate decision[0]*(decision[2]*leaf_nodes[0]) (call it term0)
    helib::Ctxt temp(decisions[2]);
    temp.multiplyBy(leaf_nodes[0]);
    helib::Ctxt term0(decisions[0]);
    term0.multiplyBy(temp);


    // calculate 1-decision[0] (call it term1)
    helib::Ctxt one_minus_ctxt_0(ctxt_1);
    one_minus_ctxt_0.addCtxt(decisions[0], true);
    helib::Ctxt term1(one_minus_ctxt_0); // the only point of this variable is for readability's sake

    // calculate (1-decision[1])*leaf_nodes[1] (call it term2)
    helib::Ctxt one_minus_ctxt_1(ctxt_1);
    one_minus_ctxt_1.addCtxt(decisions[1], true);
    one_minus_ctxt_1.multiplyBy(leaf_nodes[1]);
    helib::Ctxt term2(one_minus_ctxt_1);

    // calculate decision[1]*leaf_nodes[2] (call it term3)
    helib::Ctxt term3(decisions[1]);
    term3.multiplyBy(leaf_nodes[2]);


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
