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

    auto calleeFun = Function::create(FunctionType::get(Int32Type, {Int32Type}), "callee", module);
    auto bb = BasicBlock::create(module, "callee", calleeFun);
    builder->set_insert_point(bb);
    std:: vector<Value *> args;
    for (auto &arg : calleeFun->get_args())
    {
        args.push_back(&arg);
    }
    auto a2 = builder->create_imul(args[0], ConstantInt::get(2,module));
    builder->create_ret(a2);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),"main", module);
    bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    auto call = builder->create_call(calleeFun, {ConstantInt::get(110,module)});
    builder->create_ret(call);

    std::cout << module->print();
    delete module;
    return 0;
}