#include "Mem2Reg.hpp"
#include "BasicBlock.hpp"
#include "IRBuilder.hpp"
#include "Instruction.hpp"
#include "Value.hpp"

#include <memory>
#include <set>

void Mem2Reg::run() {
    // 创建支配树分析 Pass 的实例
    dominators_ = std::make_unique<Dominators>(m_);
    // 建立支配树
    dominators_->run();
    // 以函数为单元遍历实现 Mem2Reg 算法
    for (auto &f : m_->get_functions()) {
        if (f.is_declaration())
            continue;
        func_ = &f;
        if (func_->get_basic_blocks().size() >= 1) {
            // 对应伪代码中 phi 指令插入的阶段
            generate_phi();
            // 对应伪代码中重命名阶段
            rename(func_->get_entry_block());
        }
        // 后续 DeadCode 将移除冗余的局部变量的分配空间
    }
}

void Mem2Reg::generate_phi() {
    // TODO:
    // 步骤一：找到活跃在多个 block 的全局名字集合，以及它们所属的 bb 块
    std::set<Value*> active_var;
    std::map<Value *, std::set<BasicBlock *>> define_var_blocks;
    for(auto &bb : func_->get_basic_blocks()) {
        for(auto &inst : bb.get_instructions()) {
            if(inst.is_store()) {
                auto inst_store = static_cast<StoreInst *>(&inst);
                auto l_val = inst_store->get_lval();
                auto r_val = inst_store->get_rval();
                if(is_global_variable(l_val)) continue;
                if(is_gep_instr(r_val)) continue;
                active_var.insert(l_val);
                define_var_blocks[l_val].insert(&bb);
            }
        }
    }
    // 步骤二：从支配树获取支配边界信息，并在对应位置插入 phi 指令
    for(auto &v : active_var) {
        if(define_var_blocks[v].size() <= 1) continue;
        std::set<BasicBlock*> f_set;
        std::set<BasicBlock*> w_set = define_var_blocks[v];
        while(!w_set.empty()) {
            auto x = w_set.begin();
            w_set.erase(x);
            for(auto &y : dominators_->get_dominance_frontier(*x)) {
                if(f_set.find(y) == f_set.end()) {
                    f_set.insert(y);
                    if(define_var_blocks[v].find(y) == define_var_blocks[v].end()) 
                        w_set.insert(y);
                    auto phi = PhiInst::create_phi(v->get_type()->get_pointer_element_type(), y, {}, {});
                    y->add_instr_begin( phi );
                }
            }
        }
    }
}

void Mem2Reg::rename(BasicBlock *bb) {
    // TODO:
    // 步骤三：将 phi 指令作为 lval 的最新定值，lval 即是为局部变量 alloca 出的地址空间
    // 步骤四：用 lval 最新的定值替代对应的load指令
    // 步骤五：将 store 指令的 rval，也即被存入内存的值，作为 lval 的最新定值
    // 步骤六：为 lval 对应的 phi 指令参数补充完整
    // 步骤七：对 bb 在支配树上的所有后继节点，递归执行 re_name 操作
    // 步骤八：pop出 lval 的最新定值
    // 步骤九：清除冗余的指令
    for (auto &inst : bb->get_instructions()) {
        if(inst.is_phi()) {
            
        }
    }
}
