//===----------------------------------------------------------------------===//
//                         DuckDB
//
// planner/operator/logical_comparison_join.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "common/constants.hpp"
#include "common/unordered_set.hpp"
#include "planner/joinside.hpp"
#include "planner/operator/logical_join.hpp"

namespace duckdb {

//! LogicalComparisonJoin represents a join that involves comparisons between the LHS and RHS
class LogicalComparisonJoin : public LogicalJoin {
public:
	LogicalComparisonJoin(JoinType type, LogicalOperatorType logical_type = LogicalOperatorType::COMPARISON_JOIN);

	//! The conditions of the join
	vector<JoinCondition> conditions;

public:
	string ParamsToString() const override;

public:
	static unique_ptr<LogicalOperator> CreateJoin(JoinType type, unique_ptr<LogicalOperator> left_child,
	                                              unique_ptr<LogicalOperator> right_child,
	                                              unordered_set<index_t> &left_bindings,
	                                              unordered_set<index_t> &right_bindings,
	                                              vector<unique_ptr<Expression>> &expressions);
};

} // namespace duckdb
