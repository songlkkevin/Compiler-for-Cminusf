#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "IRBuilder.hpp"
#include "Module.hpp"
#include "Type.hpp"
#include "Value.hpp"

#include <iostream>
#include <memory>
int main() {
    auto module = new Module();
    auto builder = new IRBuilder(nullptr, module);

    Type *Int32Type = module->get_int32_type();
    Type *FloatType = module->get_float_type();

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),"main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    auto truebb = BasicBlock::create(module, "TrueBlock", mainFun);
    auto falsebb = BasicBlock::create(module, "FasleBlock", mainFun);
    builder->set_insert_point(bb);
    auto aAlloca = builder->create_alloca(FloatType);
    builder->create_store(ConstantFP::get(5.555,module), aAlloca);
    auto aLoad = builder->create_load(aAlloca);
    auto check = builder->create_fcmp_gt(aLoad, ConstantFP::get(1.0, module));
    builder->create_cond_br(check, truebb, falsebb);

    builder->set_insert_point(truebb);
    builder->create_ret(ConstantInt::get(233, module));

    builder->set_insert_point(falsebb);
    builder->create_ret(ConstantInt::get(0, module));

    std::cout << module->print();
    delete module;
    return 0;
}