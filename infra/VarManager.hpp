#ifndef ANALYSISTREEQA_SRC_VARMANAGER_H_
#define ANALYSISTREEQA_SRC_VARMANAGER_H_

#include "vector"

#include "BranchReader.hpp"
#include "FillTask.hpp"
#include "Variable.hpp"
#include "Matching.hpp"
#include "VarManagerEntry.hpp"

namespace AnalysisTree {

class VarManager : public FillTask {

 public:
  VarManager() = default;

  size_t AddEntry(const VarManagerEntry& vars) { vars_.emplace_back(vars); return vars_.size()-1; }

  void Init(std::map<std::string, void *> &pointers_map) override;
  void Exec() override;
  void Finish() override{};

  void SetCutsMap(std::map<std::string, Cuts*> map) { cuts_map_ = std::move(map); }

  [[nodiscard]] const std::vector<std::vector<double>> &GetValues(int i_var) const {
    return vars_.at(i_var).GetValues();
  }

  std::vector<BranchReader> &Branches() { return branches_; }
  BranchReader* GetBranch(const std::string &name);

  [[nodiscard]] const std::vector<BranchReader> &GetBranches() const { return branches_; }
  void FillBranchNames();

 private:
  std::vector<VarManagerEntry> vars_{};

  std::vector<BranchReader> branches_{};
  std::vector<Matching *> matching_{};
  std::map<std::string, Cuts *> cuts_map_{};
};

}// namespace AnalysisTree
#endif//ANALYSISTREEQA_SRC_VARMANAGER_H_
