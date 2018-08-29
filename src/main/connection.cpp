
#include "main/connection.hpp"

#include "execution/executor.hpp"
#include "execution/physical_plan_generator.hpp"
#include "optimizer/optimizer.hpp"
#include "parser/parser.hpp"
#include "planner/planner.hpp"

using namespace duckdb;
using namespace std;

DuckDBConnection::DuckDBConnection(DuckDB &database)
    : db(database), context(database) {}

DuckDBConnection::~DuckDBConnection() {}

unique_ptr<DuckDBResult> DuckDBConnection::GetQueryResult(std::string query) {
	auto result = make_unique<DuckDBResult>();
	result->success = false;
	try {
		// parse the query and transform it into a set of statements
		Parser parser;
		if (!parser.ParseQuery(query.c_str())) {
			return make_unique<DuckDBResult>(parser.GetErrorMessage());
		}

		Planner planner;
		if (!planner.CreatePlan(context, move(parser.statements.back()))) {
			return make_unique<DuckDBResult>(planner.GetErrorMessage());
		}
		if (!planner.plan) {
			return make_unique<DuckDBResult>();
		}

		auto plan = move(planner.plan);

		Optimizer optimizer;
		plan = optimizer.Optimize(move(plan));
		if (!optimizer.GetSuccess()) {
			// failed to optimize
			return make_unique<DuckDBResult>(optimizer.GetErrorMessage());
		}
		if (!plan) {
			return make_unique<DuckDBResult>();
		}

		// now convert logical query plan into a physical query plan
		PhysicalPlanGenerator physical_planner(context);
		if (!physical_planner.CreatePlan(move(plan))) {
			return make_unique<DuckDBResult>(
			    physical_planner.GetErrorMessage());
		}

		// finally execute the plan and return the result
		Executor executor;
		result->collection = executor.Execute(move(physical_planner.plan));
		result->success = true;
	} catch (Exception ex) {
		result->error = ex.GetMessage();
	} catch (...) {
		result->error = "UNHANDLED EXCEPTION TYPE THROWN IN PLANNER!";
	}
	return move(result);
}

unique_ptr<DuckDBResult> DuckDBConnection::Query(std::string query) {
	if (context.transaction.IsAutoCommit()) {
		context.transaction.BeginTransaction();
	}

	auto result = GetQueryResult(query);

	if (context.transaction.IsAutoCommit() &&
	    context.transaction.HasActiveTransaction()) {
		if (result->GetSuccess()) {
			context.transaction.Commit();
		} else {
			context.transaction.Rollback();
		}
	}
	return move(result);
}