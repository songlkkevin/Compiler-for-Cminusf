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

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),"main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    auto While = BasicBlock::create(module, "While", mainFun);
    auto Do = BasicBlock::create(module, "Do", mainFun);
    auto AfterWhile = BasicBlock::create(module, "AfterWhile", mainFun);
    
    builder->set_insert_point(bb);
    auto aAlloca = builder->create_alloca(Int32Type);
    auto iAlloca = builder->create_alloca(Int32Type);
    builder->create_store(ConstantInt::get(10, module), aAlloca);
    builder->create_store(ConstantInt::get(0, module), iAlloca);
    builder->create_br(While);

    builder->set_insert_point(While);
    auto iLoad = builder->create_load(iAlloca);
    auto iCmp = builder->create_icmp_lt(iLoad, ConstantInt::get(10, module));
    builder->create_cond_br(iCmp, Do, AfterWhile);

    builder->set_insert_point(Do);
    iLoad = builder->create_load(iAlloca);
    auto iAdd = builder->create_iadd(iLoad, ConstantInt::get(1, module));
    builder->create_store(iAdd, iAlloca);
    auto aLoad = builder->create_load(aAlloca);
    auto aAdd = builder->create_iadd(aLoad, iAdd);
    builder->create_store(aAdd, aAlloca);
    builder->create_br(While);

    builder->set_insert_point(AfterWhile);
    aLoad = builder->create_load(aAlloca);
    builder->create_ret(aLoad);

    std::cout << module->print();
    delete module;
    return 0;
}