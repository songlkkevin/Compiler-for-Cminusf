#pragma once

#include "BasicBlock.hpp"
#include "Function.hpp"
#include "PassManager.hpp"

#include <map>
#include <set>
#include <vector>

class Dominators : public Pass {
  public:
    using BBSet = std::set<BasicBlock *>;

    explicit Dominators(Module *m) : Pass(m) {}
    ~Dominators() = default;
    void run() override;

    BasicBlock *get_idom(BasicBlock *bb) { return idom_.at(bb); }
    const BBSet &get_dominance_frontier(BasicBlock *bb) {
        return dom_frontier_.at(bb);
    }
    const BBSet &get_dom_tree_succ_blocks(BasicBlock *bb) {
        return dom_tree_succ_blocks_.at(bb);
    }

  private:
    void create_idom(Function *f);
    void create_dominance_frontier(Function *f);
    void create_dom_tree_succ(Function *f);
    void create_postorder(Function *f);
    void dfs(BasicBlock*);
    BasicBlock* intersect(BasicBlock* b1, BasicBlock* b2);
    void print_idom(Function *f);
    void print_dominance_frontier(Function *f);

    // TODO 补充需要的函数

    std::map<BasicBlock *, BasicBlock *> idom_;  // 直接支配
    std::map<BasicBlock *, BBSet> dom_frontier_{}; // 支配边界集合
    std::map<BasicBlock *, BBSet> dom_tree_succ_blocks_{}; // 支配树中的后继节点
    std::vector<BasicBlock*> postorder_vector_;
    std::map<BasicBlock*, int> postorder_;
};
