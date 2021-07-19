/* Copyright (C) 2019-2021 GSI, MEPhI
   SPDX-License-Identifier: GPL-3.0-only
   Authors: Eugeny Kashirin, Ilya Selyuzhenkov */
#ifndef ANALYSISTREE_INFRA_BRANCH_HPP_
#define ANALYSISTREE_INFRA_BRANCH_HPP_

#include <BranchConfig.hpp>
#include <Configuration.hpp>

#include <cassert>
#include <map>
#include <utility>
#include <vector>

#include "BranchChannel.hpp"

class TTree;

namespace AnalysisTree {

class Branch {
 public:

  /* c-tors */
  explicit Branch(BranchConfig config) : config_(std::move(config)) {
    InitDataPtr();
    UpdateConfigHash();
  }

  template<class T>
  Branch(BranchConfig config, T* data) : config_(std::move(config)), data_(data) {
    UpdateConfigHash();
  }

  Branch(BranchConfig config, BranchPointer ptr) : config_(std::move(config)), data_(ptr) {
    UpdateConfigHash();
  }

  ~Branch();

  struct FieldsMapping {
    std::vector<std::pair<Field /* src */, Field /* dst */>> field_pairs;
  };

  /* Accessors to branch' main parameters, used very often */
  inline auto GetBranchName() const { return config_.GetName(); }
  inline auto GetBranchType() const { return config_.GetType(); }
  inline const AnalysisTree::BranchConfig& GetConfig() const { return config_; }

  void InitDataPtr();
  void ConnectOutputTree(TTree* tree);

  BranchChannel NewChannel();
  void ClearChannels();
  Field NewVariable(const std::string& field_name, AnalysisTree::Types type);
  void CloneVariables(const AnalysisTree::BranchConfig& other);
  void CopyContents(Branch* br);

  //  /* iterating */
  [[nodiscard]] size_t size() const;

  BranchChannel operator[](size_t i_channel) {
    return BranchChannel(this, i_channel);
  }

  [[nodiscard]] size_t GetId() const {
    return std::visit([](auto&& b) { return b->GetId(); }, data_);
  }

  [[nodiscard]] BranchPointer GetData() const { return data_; }
  [[nodiscard]] Field GetField(std::string field_name) const {
    Field field(config_.GetName(), std::move(field_name));
    field.Init(config_);
    return field;
  }

 private:
  AnalysisTree::BranchConfig config_;
  BranchPointer data_;/// owns object
  bool is_mutable_{false};
  bool is_frozen_{false};
  std::size_t config_hash_{0};

 public:
  AnalysisTree::Configuration* parent_config{nullptr};
  bool is_connected_to_input{false};
  bool is_connected_to_output{false};

  std::map<const Branch* /* other branch */, FieldsMapping> copy_fields_mapping;

  /* Modification */
  void Freeze(bool freeze = true) { is_frozen_ = freeze; };
  void SetMutable(bool is_mutable_ = true) { Branch::is_mutable_ = is_mutable_; }
  /* Checks are used very often */
  inline void CheckFrozen(bool expected = true) const {
    if (is_frozen_ != expected)
      throw std::runtime_error("Branch is frozen");
  }
  inline void CheckMutable(bool expected = true) const {
    if (is_mutable_ != expected)
      throw std::runtime_error("Branch is not mutable");
  }

  Field GetFieldVar(const std::string& field_name);
  /**
   * @brief Gets variables according to variable names specified in the arguments.
   * Returns tuple of variables which is suitable for unpacking with std::tie()
   * @tparam Args
   * @param field_name variable names convertible to std::string
   * @return tuple of variables
   */
  template<typename... Args>
  auto GetVars(Args... field_name) {
    return GetVarsImpl(std::array<std::string, sizeof...(Args)>{{std::string(field_name)...}},
                       std::make_index_sequence<sizeof...(Args)>());
  }
  /**
   * @brief Initializes ATI2::Field objects
   * @param vars - vector of pairs with name and reference to the ATI2::Field object
   */
  void UseFields(std::vector<std::pair<std::string, std::reference_wrapper<Field>>>&& vars, bool ignore_missing = false);
  bool HasField(const std::string& field_name) const;
  std::vector<std::string> GetFieldNames() const;

//  /* Getting value */

  /**
   * @brief Copies contents from other branch 'as-is'. Faster than CopyContents() since it creates no mapping
   * @param other
   */
  void CopyContentsRaw(Branch* other);

  void CreateMapping(Branch* other);

  void UpdateConfigHash();

  template<typename EntityPtr>
  constexpr static const bool is_event_header_v =
      std::is_same_v<AnalysisTree::EventHeader, std::remove_const_t<std::remove_pointer_t<EntityPtr>>>;

  template<typename Functor>
  auto ApplyT(Functor&& f) {
    return std::visit(f, data_);
  }

  template<typename Functor>
  auto ApplyT(Functor&& f) const {
    return std::visit(f, data_);
  }

  AnalysisTree::ShortInt_t Hash() const {
    const auto hasher = std::hash<std::string>();
    return AnalysisTree::ShortInt_t(hasher(config_.GetName()));
  }

 private:
  template<size_t... Idx>
  auto GetVarsImpl(std::array<std::string, sizeof...(Idx)>&& field_names, std::index_sequence<Idx...>) {
    return std::make_tuple(GetFieldVar(field_names[Idx])...);
  }
};

}// namespace AnalysisTree

#endif//ANALYSISTREE_INFRA_BRANCH_HPP_