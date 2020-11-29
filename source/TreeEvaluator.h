//
// Created by soham on 10/29/20.
//

#ifndef HOMOMORPHICTREEEVALUATOR_TREEEVALUATOR_H
#define HOMOMORPHICTREEEVALUATOR_TREEEVALUATOR_H
#define BIT_SIZE 16

#include "Encryptor.h"
#include "Util.h"

class TreeEvaluator {
public:    
    static helib::Ctxt getCtxt(int i, helib::Context &context, helib::PubKey &pubkey, int val);

    static helib::Ctxt evaluate_decision_tree(helib::Ctxt input_vector, helib::PubKey pubkey, helib::Context &context);

    static helib::Ctxt calculate_result(helib::Ctxt decisions[], helib::Ctxt leaf_nodes[], helib:: Ctxt ctxt_1);

    static helib::Ctxt compareCtxt(helib::Ctxt xCtxt, helib::Ctxt yCtxt, helib::Context &context, helib::PubKey &pubkey);

    static void getCtxtList(helib::Context &context, helib::PubKey &pubkey, helib::Ctxt* nodes, int* val);

};


#endif //HOMOMORPHICTREEEVALUATOR_TREEEVALUATOR_H
