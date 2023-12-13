#include "Dominators.hpp"
#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include <cstddef>
#include <cstdlib>
#include <map>
#include <queue>
#include <string>

void Dominators::run() {
    for (auto &f1 : m_->get_functions()) {
        auto f = &f1;
        postorder_vector_.clear();
        postorder_.clear();
        if (f->get_basic_blocks().size() == 0)
            continue;
        for (auto &bb1 : f->get_basic_blocks()) {
            auto bb = &bb1;
            postorder_.insert({bb, -1});
            idom_.insert({bb, {}});
            dom_frontier_.insert({bb, {}});
            dom_tree_succ_blocks_.insert({bb, {}});
        }

        create_postorder(f);
        create_idom(f);
        create_dominance_frontier(f);
        // print_idom(f);
        // print_dominance_frontier(f);
        create_dom_tree_succ(f);
    }
}

void Dominators::dfs(BasicBlock* now_bb) {
    postorder_[now_bb] = 0;
    for(auto &next_bb : now_bb->get_succ_basic_blocks()) {
        if(postorder_[next_bb] != -1)
            continue;
        dfs(next_bb);
    }
    postorder_[now_bb] = postorder_vector_.size();
    postorder_vector_.push_back(now_bb);
    return;
}

void Dominators::create_postorder(Function *f) {
    dfs(f->get_entry_block());
    return;
}

BasicBlock* Dominators::intersect(BasicBlock* bb1, BasicBlock* bb2) {
    if(bb1 == nullptr) return bb2;
    if(bb2 == nullptr) return bb1;
    while(postorder_[bb1] != postorder_[bb2])
    {
        while(postorder_[bb1] < postorder_[bb2])
            bb1 = idom_[bb1];
        while(postorder_[bb1] > postorder_[bb2])
            bb2 = idom_[bb2];
    }
    return bb1;
}

void Dominators::create_idom(Function *f) {
    // TODO 分析得到 f 中各个基本块的 idom
    int changed = true;
    idom_[f->get_entry_block()] = f->get_entry_block();
    while (changed) {
        changed = false;
        for(int i = postorder_vector_.size() - 1; i >= 0; i --) {
            auto now_bb = postorder_vector_[i];
            BasicBlock* new_idom = nullptr;
            if(now_bb == f->get_entry_block()) 
                continue;
            for(auto pre_bb : now_bb->get_pre_basic_blocks()) 
                if(idom_[pre_bb] != nullptr)
                    new_idom = intersect(new_idom, pre_bb);
            if(idom_[now_bb] != new_idom)
            {
                idom_[now_bb] = new_idom;
                changed = true;
            }
            // std::cout << now_bb->get_name();
            // print_idom(f);
        }
    }
}

void Dominators::create_dominance_frontier(Function *f) {
    // TODO 分析得到 f 中各个基本块的支配边界集合
    for(auto &bb1 : f->get_basic_blocks()) {
        auto bb = &bb1;
        if(bb->get_pre_basic_blocks().size() < 2)
            continue;
        for(auto &runner : bb->get_pre_basic_blocks()) {
            while(runner != get_idom(bb)) {
                dom_frontier_[runner].insert(bb);
                runner = get_idom(runner);
            }
        }
    }
}

void Dominators::create_dom_tree_succ(Function *f) {
    // TODO 分析得到 f 中各个基本块的支配树后继
    for (auto &bb1 : f->get_basic_blocks()) {
        auto bb = &bb1;
        if(get_idom(bb) != bb) {
            dom_tree_succ_blocks_[get_idom(bb)].insert(bb);
        }
    }
}


void Dominators::print_idom(Function *f)
{
    int counter = 0;
    std::map<BasicBlock *, std::string> bb_id;
    for (auto &bb1 : f->get_basic_blocks()) {
        auto bb = &bb1;
        if (bb->get_name().empty())
            bb_id[bb] = "bb" + std::to_string(counter);
        else 
            bb_id[bb] = bb->get_name();
        counter++;
    }
    printf("Immediate dominance of function %s:\n", f->get_name().c_str());
    for (auto &bb1 : f->get_basic_blocks()) {
        auto bb= &bb1;
        std::string output;
        output = bb_id[bb] + ": ";
        if (get_idom(bb)) {
            output += bb_id[get_idom(bb)];
        }
        else {
            output += "null";
        }
        output += " :";
        output += std::to_string(postorder_[bb]);
        printf("%s\n", output.c_str());
    }
}

void Dominators::print_dominance_frontier(Function *f)
{
    int counter = 0;
    std::map<BasicBlock *, std::string> bb_id;
    for (auto &bb1 : f->get_basic_blocks()) {
        auto bb = &bb1;
        if (bb->get_name().empty())
            bb_id[bb] = "bb" + std::to_string(counter);
        else 
            bb_id[bb] = bb->get_name();
        counter++;
    }
    printf("Dominance Frontier of function %s:\n", f->get_name().c_str());
    for (auto &bb1 : f->get_basic_blocks()) {
        std::string output;
        auto bb = &bb1;
        output = bb_id[bb] + ": ";
        if (get_dominance_frontier(bb).empty()) {
            output += "null";
        }
        else {
            bool first = true;
            for (auto df : get_dominance_frontier(bb)) {
                if (first) {
                    first = false;
                }
                else {
                    output += ", ";
                }
                output += bb_id[df];
            }
        }
        printf("%s\n", output.c_str());
    }
}