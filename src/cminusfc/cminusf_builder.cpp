#include "cminusf_builder.hpp"

#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_INT(num) ConstantInt::get(num, module.get())

// types
Type *VOID_T;
Type *INT1_T;
Type *INT32_T;
Type *INT32PTR_T;
Type *FLOAT_T;
Type *FLOATPTR_T;

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

Value* CminusfBuilder::visit(ASTProgram &node) {
    VOID_T = module->get_void_type();
    INT1_T = module->get_int1_type();
    INT32_T = module->get_int32_type();
    INT32PTR_T = module->get_int32_ptr_type();
    FLOAT_T = module->get_float_type();
    FLOATPTR_T = module->get_float_ptr_type();

    Value *ret_val = nullptr;
    for (auto &decl : node.declarations) {
        ret_val = decl->accept(*this);
    }
    return ret_val;
}

Value* CminusfBuilder::visit(ASTNum &node) {
    // TODO: This function is empty now. DONE
    // Add some code here.
    if(node.type == TYPE_INT)
        return CONST_INT(node.i_val);
    else
        return CONST_FP(node.f_val);
    return nullptr;
}

Value* CminusfBuilder::visit(ASTVarDeclaration &node) {
    // TODO: This function is empty now. DONE
    // Add some code here.
    Type *type;
    if(node.type == TYPE_INT)
        type = INT32_T;
    else
        type = FLOAT_T;
    if(node.num != nullptr)
    {
        type = ArrayType::get(type, node.num->i_val);
    }
    if(scope.in_global())
    {
        auto initializer = ConstantZero::get(type, module.get());
        auto var = GlobalVariable::create(node.id, module.get(), type, false, initializer);
        scope.push(node.id, var);
    }
    else
    {
        auto var = builder->create_alloca(type);
        scope.push(node.id, var);
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTFunDeclaration &node) {
    FunctionType *fun_type;
    Type *ret_type;
    std::vector<Type *> param_types;
    if (node.type == TYPE_INT)
        ret_type = INT32_T;
    else if (node.type == TYPE_FLOAT)
        ret_type = FLOAT_T;
    else
        ret_type = VOID_T;

    for (auto &param : node.params) {
        // TODO: Please accomplish param_types. DONE
        if(param->type == TYPE_VOID)
            param_types.push_back(VOID_T);
        if(param->type == TYPE_INT)
        {
            if(param->isarray)
                param_types.push_back(INT32PTR_T);
            else
                param_types.push_back(INT32_T);
        }
        if(param->type == TYPE_FLOAT)
        {
            if(param->isarray)
                param_types.push_back(FLOATPTR_T);
            else
                param_types.push_back(FLOAT_T);
        }
    }

    fun_type = FunctionType::get(ret_type, param_types);
    auto func = Function::create(fun_type, node.id, module.get());
    scope.push(node.id, func);
    context.func = func;
    auto funBB = BasicBlock::create(module.get(), "entry", func);
    builder->set_insert_point(funBB);
    scope.enter();
    std::vector<Value *> args;
    for (auto &arg : func->get_args()) {
        args.push_back(&arg);
    }
    for (size_t i = 0; i < node.params.size(); ++i) {
        // TODO: You need to deal with params and store them in the scope. DONE
        if(node.params[i]->type == TYPE_VOID) continue;
        if(node.params[i]->type == TYPE_INT)
        {
            if(node.params[i]->isarray)
            {
                auto arr_ptr = builder->create_alloca(INT32PTR_T);
                builder->create_store(args[i], arr_ptr);
                scope.push(node.params[i]->id, arr_ptr);
            }
            else 
            {
                auto ptr = builder->create_alloca(INT32_T);
                builder->create_store(args[i], ptr);
                scope.push(node.params[i]->id, ptr);
            }
        }
        else 
        {
            if(node.params[i]->isarray)
            {
                auto arr_ptr = builder->create_alloca(FLOATPTR_T);
                builder->create_store(args[i], arr_ptr);
                scope.push(node.params[i]->id, arr_ptr);
            }
            else 
            {
                auto ptr = builder->create_alloca(FLOAT_T);
                builder->create_store(args[i], ptr);
                scope.push(node.params[i]->id, ptr);
            }
        }

    }
    node.compound_stmt->accept(*this);
    if (not builder->get_insert_block()->is_terminated())
    {
        if (context.func->get_return_type()->is_void_type())
            builder->create_void_ret();
        else if (context.func->get_return_type()->is_float_type())
            builder->create_ret(CONST_FP(0.));
        else
            builder->create_ret(CONST_INT(0));
    }
    scope.exit();
    return nullptr;
}

Value* CminusfBuilder::visit(ASTParam &node) {
    // TODO: This function is empty now. DONE in FunDeclaration
    // Add some code here.
    return nullptr;
}

Value* CminusfBuilder::visit(ASTCompoundStmt &node) {
    // TODO: This function is not complete. DONE?
    // You may need to add some code here
    // to deal with complex statements.
    scope.enter();
    for (auto &decl : node.local_declarations) {
        decl->accept(*this);
    }

    for (auto &stmt : node.statement_list) {
        stmt->accept(*this);
        if (builder->get_insert_block()->is_terminated())
            break;
    }
    scope.exit();
    return nullptr;
}

Value* CminusfBuilder::visit(ASTExpressionStmt &node) {
    // TODO: This function is empty now. DONE
    // Add some code here.
    if(node.expression != nullptr)
        return node.expression->accept(*this);
    return nullptr;
}

Value* CminusfBuilder::visit(ASTSelectionStmt &node) {
    // TODO: This function is empty now.
    // Add some code here.
    auto expr = node.expression->accept(*this);
    Value *cond;
    auto next_bb = BasicBlock::create(module.get(), "", context.func);
    auto true_bb = BasicBlock::create(module.get(), "", context.func);
    if(expr->get_type()->is_integer_type())
        cond = builder->create_icmp_ne(expr, CONST_INT(0));
    else
        cond = builder->create_fcmp_ne(expr, CONST_FP(0.0));
    if(node.else_statement != nullptr)
    {
        auto false_bb = BasicBlock::create(module.get(), "", context.func);
        builder->create_cond_br(cond, true_bb, false_bb);
        builder->set_insert_point(true_bb);
        node.if_statement->accept(*this);
        if(builder->get_insert_block()->is_terminated() == false)
            builder->create_br(next_bb);
        builder->set_insert_point(false_bb);
        node.else_statement->accept(*this);
        if(builder->get_insert_block()->is_terminated() == false)
            builder->create_br(next_bb);
        builder->set_insert_point(next_bb);
    }
    else 
    {
        builder->create_cond_br(cond, true_bb, next_bb);
        builder->set_insert_point(true_bb);
        node.if_statement->accept(*this);
        if(builder->get_insert_block()->is_terminated() == false)
            builder->create_br(next_bb);
        builder->set_insert_point(next_bb);
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTIterationStmt &node) {
    // TODO: This function is empty now.
    // Add some code here.
    auto cond_bb = BasicBlock::create(module.get(), "", context.func);
    auto body_bb = BasicBlock::create(module.get(), "", context.func);
    auto next_bb = BasicBlock::create(module.get(), "", context.func);
    Value* cond;
    builder->create_br(cond_bb);
    builder->set_insert_point(cond_bb);
    auto expr = node.expression->accept(*this);
    if(expr->get_type()->is_integer_type())
        cond = builder->create_icmp_ne(expr, CONST_INT(0));
    else
        cond = builder->create_fcmp_ne(expr, CONST_FP(0.0));
    builder->create_cond_br(cond, body_bb, next_bb);
    builder->set_insert_point(body_bb);
    node.statement->accept(*this);
    if(builder->get_insert_block()->is_terminated() == false)
        builder->create_br(cond_bb);
    builder->set_insert_point(next_bb);
    return nullptr;
}

Value* CminusfBuilder::visit(ASTVar &node) {
    // TODO: This function is empty now.DONE
    // Add some code here.
    auto VarValue = scope.find(node.id);
    auto IsLeftValue = context.IsLeftValue;
    context.IsLeftValue = false;
    if(node.expression == nullptr)
    {
        if(IsLeftValue)
        {
            return VarValue;
        }
        else 
        {
            if(VarValue->get_type()->get_pointer_element_type()->is_integer_type() || VarValue->get_type()->get_pointer_element_type()->is_float_type() || VarValue->get_type()->get_pointer_element_type()->is_pointer_type())
                VarValue = builder->create_load(VarValue);
            else
                VarValue = builder->create_gep(VarValue, {CONST_INT(0), CONST_INT(0)});
            return VarValue;
        }
    }
    else
    {
        auto expr = node.expression->accept(*this);
        auto is_neg = BasicBlock::create(module.get(), "", context.func);
        auto not_neg = BasicBlock::create(module.get(), "", context.func);
        if(expr->get_type()->is_float_type())
            expr = builder->create_fptosi(expr, INT32_T);
        auto cmp = builder->create_icmp_ge(expr, CONST_INT(0));
        builder->create_cond_br(cmp, not_neg, is_neg);
        
        builder->set_insert_point(is_neg);
        auto neg_func = static_cast<Function *>(scope.find("neg_idx_except"));
        builder->create_call(neg_func, {});
        if(builder->get_insert_block()->is_terminated() == false)
        {
            if(context.func->get_return_type()->is_float_type())
                builder->create_ret(CONST_FP(0.0));
            if(context.func->get_return_type()->is_integer_type())
                builder->create_ret(CONST_INT(0));
            if(context.func->get_return_type()->is_void_type())
                builder->create_void_ret();
        }

        builder->set_insert_point(not_neg);
        Value* gep;
        if(VarValue->get_type()->get_pointer_element_type()->is_integer_type() || VarValue->get_type()->get_pointer_element_type()->is_float_type())
        {
            gep = builder->create_gep(VarValue, {expr});
        }
        else
        {
            if(VarValue->get_type()->get_pointer_element_type()->is_pointer_type())
            {
                gep = builder->create_load(VarValue);
                gep = builder->create_gep(gep, {expr});
            }
            else
            {
                gep = builder->create_gep(VarValue, {CONST_INT(0), expr});
            }
        }
        if(IsLeftValue)
        {
            return gep;
        }
        else 
        {
            VarValue = builder->create_load(gep);
            return VarValue;
        }
    }
    return nullptr;
}

Value* CminusfBuilder::visit(ASTReturnStmt &node) {
    if (node.expression == nullptr) {
        builder->create_void_ret();
        return nullptr;
    } 
    // TODO: The given code is incomplete. DONE
    // You need to solve other return cases (e.g. return an integer).
    auto ret_val = node.expression->accept(*this);
    auto ret_type = context.func->get_return_type();
    if(ret_val->get_type() != ret_type)
    {
        if(ret_type->is_integer_type())
            ret_val = builder->create_fptosi(ret_val, INT32_T);
        if(ret_type->is_float_type())
            ret_val = builder->create_sitofp(ret_val, FLOAT_T);
    }
    builder->create_ret(ret_val);
    return nullptr;
}

Value* CminusfBuilder::visit(ASTAssignExpression &node) {
    // TODO: This function is empty now. DONE
    // Add some code here.
    context.IsLeftValue = true;
    auto LeftValue = node.var->accept(*this);
    auto RightValue = node.expression->accept(*this);
    if(LeftValue->get_type()->get_pointer_element_type() != RightValue->get_type())
    {
        if(RightValue->get_type()->is_integer_type())
            RightValue = builder->create_sitofp(RightValue, FLOAT_T);
        else
            RightValue = builder->create_fptosi(RightValue, INT32_T);
    }
    builder->create_store(RightValue, LeftValue);
    return RightValue;
}

Value* CminusfBuilder::visit(ASTSimpleExpression &node) {
    // TODO: This function is empty now.
    // Add some code here.
    if(node.additive_expression_r == nullptr)
        return node.additive_expression_l->accept(*this);
    Value *Result;
    auto FirstValue = node.additive_expression_l->accept(*this);
    auto SecondValue = node.additive_expression_r->accept(*this);
    if(FirstValue->get_type() != SecondValue->get_type())
    {
        if(FirstValue->get_type()->is_integer_type())
            FirstValue = builder->create_sitofp(FirstValue, FLOAT_T);
        if(SecondValue->get_type()->is_integer_type())
            SecondValue = builder->create_sitofp(SecondValue, FLOAT_T);
    }
    if(FirstValue->get_type()->is_integer_type())
    {
        if(node.op == OP_LE)
            Result = builder->create_icmp_le(FirstValue, SecondValue);
        if(node.op == OP_LT)
            Result = builder->create_icmp_lt(FirstValue, SecondValue);
        if(node.op == OP_GT)
            Result = builder->create_icmp_gt(FirstValue, SecondValue);
        if(node.op == OP_GE)
            Result = builder->create_icmp_ge(FirstValue, SecondValue);
        if(node.op == OP_EQ)
            Result = builder->create_icmp_eq(FirstValue, SecondValue);
        if(node.op == OP_NEQ)
            Result = builder->create_icmp_ne(FirstValue, SecondValue);
    }
    else
    {
        if(node.op == OP_LE)
            Result = builder->create_fcmp_le(FirstValue, SecondValue);
        if(node.op == OP_LT)
            Result = builder->create_fcmp_lt(FirstValue, SecondValue);
        if(node.op == OP_GT)
            Result = builder->create_fcmp_gt(FirstValue, SecondValue);
        if(node.op == OP_GE)
            Result = builder->create_fcmp_ge(FirstValue, SecondValue);
        if(node.op == OP_EQ)
            Result = builder->create_fcmp_eq(FirstValue, SecondValue);
        if(node.op == OP_NEQ)
            Result = builder->create_fcmp_ne(FirstValue, SecondValue);
    }
    return builder->create_zext(Result, INT32_T);
}

Value* CminusfBuilder::visit(ASTAdditiveExpression &node) {
    // TODO: This function is empty now. DONE
    // Add some code here.
    if(node.additive_expression == nullptr)
        return node.term->accept(*this);
    auto FirstValue = node.additive_expression->accept(*this);
    auto SecondValue = node.term->accept(*this);
    Value* Result = nullptr;
    if(FirstValue->get_type()->is_float_type() && SecondValue->get_type()->is_integer_type())
        SecondValue = builder->create_sitofp(SecondValue, FLOAT_T);
    if(FirstValue->get_type()->is_integer_type() && SecondValue->get_type()->is_float_type())
        FirstValue = builder->create_sitofp(FirstValue, FLOAT_T);
    if(FirstValue->get_type()->is_integer_type() && SecondValue->get_type()->is_integer_type())
    {   
        if(node.op == OP_PLUS)
        {
            Result = builder->create_iadd(FirstValue, SecondValue);
        }
        else
        {
            Result = builder->create_isub(FirstValue, SecondValue);
        }
    }
    else
    {
        if(node.op == OP_PLUS)
        {
            Result = builder->create_fadd(FirstValue, SecondValue);
        }
        else
        {
            Result = builder->create_fsub(FirstValue, SecondValue);
        }
    }
    return Result;
}

Value* CminusfBuilder::visit(ASTCall &node) {
    // TODO: This function is empty now.DONE
    // Add some code here.
    auto CallFunc = static_cast<Function *>(scope.find(node.id));
    std::vector<Value *> args;
    auto func_type = static_cast<FunctionType *>(CallFunc->get_type());
    for (size_t i = 0; i < node.args.size(); i++)
    {
        auto ArgValue = node.args[i]->accept(*this);
        if(func_type->get_param_type(i) != ArgValue->get_type() && !func_type->get_param_type(i)->is_pointer_type())
        {
            if(ArgValue->get_type()->is_float_type())
                ArgValue = builder->create_fptosi(ArgValue, INT32_T);
            else
                ArgValue = builder->create_sitofp(ArgValue, FLOAT_T);
        }
        args.push_back(ArgValue);
    }
    auto CallValue = builder->create_call(CallFunc, args);
    return CallValue;
}

Value* CminusfBuilder::visit(ASTTerm &node) {
    // TODO: This function is empty now. DONE
    // Add some code here.
    if(node.term == nullptr)
        return node.factor->accept(*this);
    auto FirstValue = node.term->accept(*this);
    auto SecondValue = node.factor->accept(*this);
    Value* Result = nullptr;
    if(FirstValue->get_type()->is_float_type() && SecondValue->get_type()->is_integer_type())
        SecondValue = builder->create_sitofp(SecondValue, FLOAT_T);
    if(FirstValue->get_type()->is_integer_type() && SecondValue->get_type()->is_float_type())
        FirstValue = builder->create_sitofp(FirstValue, FLOAT_T);
    if(FirstValue->get_type()->is_integer_type() && SecondValue->get_type()->is_integer_type())
    {   
        if(node.op == OP_MUL)
        {
            Result = builder->create_imul(FirstValue, SecondValue);
        }
        else
        {
            Result = builder->create_isdiv(FirstValue, SecondValue);
        }
    }
    else
    {
        if(node.op == OP_MUL)
        {
            Result = builder->create_fmul(FirstValue, SecondValue);
        }
        else
        {
            Result = builder->create_fdiv(FirstValue, SecondValue);
        }
    }
    return Result;
}
