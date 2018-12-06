#include "execution/operator/scan/physical_index_scan.hpp"

#include "main/client_context.hpp"

using namespace duckdb;
using namespace std;

void PhysicalIndexScan::_GetChunk(ClientContext &context, DataChunk &chunk, PhysicalOperatorState *state_) {
	auto state = reinterpret_cast<PhysicalIndexScanOperatorState *>(state_);
	if (column_ids.size() == 0)
		return;

	if (!state->scan_state) {
		// initialize the scan state of the index
		if (low_expression)
			state->scan_state = index.InitializeScan(context.ActiveTransaction(), column_ids, low_expression.get(),
			                                         low_expression_type);
		else
			state->scan_state = index.InitializeScan(context.ActiveTransaction(), column_ids, high_expression.get(),
			                                         high_expression_type);
	}

	//! Continue the scan of the index
	index.Scan(context.ActiveTransaction(), state->scan_state.get(), chunk);
}

string PhysicalIndexScan::ExtraRenderInformation() {
	return tableref.name + "[" + low_expression->ToString() + "]";
}

unique_ptr<PhysicalOperatorState> PhysicalIndexScan::GetOperatorState(ExpressionExecutor *parent_executor) {
	return make_unique<PhysicalIndexScanOperatorState>(parent_executor);
}