#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "IRBuilder.hpp"
#include "Module.hpp"
#include "Type.hpp"

#include <iostream>
#include <memory>
int main() {
    auto module = new Module();
    auto builder = new IRBuilder(nullptr, module);

    Type *Int32Type = module->get_int32_type();
    auto *arrayType_10 = ArrayType::get(Int32Type, 10);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),"main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    auto aAlloc = builder->create_alloca(arrayType_10);
    auto a0GEP = builder->create_gep(aAlloc, {ConstantInt::get(0, module),ConstantInt::get(0,module)});
    builder->create_store(ConstantInt::get(10,module), a0GEP);
    auto a0Load = builder->create_load(a0GEP);
    auto a1mul = builder->create_imul(a0Load, ConstantInt::get(2,module));
    auto a1GEP = builder->create_gep(aAlloc, {ConstantInt::get(0, module),ConstantInt::get(1,module)});
    builder->create_store(a1mul, a1GEP);
    auto a1Load = builder->create_load(a1GEP);
    builder->create_ret(a1Load);
    std::cout << module->print();
    delete module;
    return 0;
}
