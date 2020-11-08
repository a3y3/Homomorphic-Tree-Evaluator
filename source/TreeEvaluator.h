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
    static helib::Ctxt evaluate_decision_tree(helib::Ctxt input_vector);

    static helib::Ctxt calculate_result(helib::Ctxt decisions[], helib::Ctxt leaf_nodes[], helib:: Ctxt ctxt_1);
};


#endif //HOMOMORPHICTREEEVALUATOR_TREEEVALUATOR_H
