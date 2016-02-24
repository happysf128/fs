
#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/handlers/action_schema_handler.hxx>
#include <actions/action_schema.hxx>
#include <utils/logging.hxx>
#include <actions/action_id.hxx>
#include <utils/binding.hxx>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSPHandler>> ActionSchemaCSPHandler::create(const std::vector<const ActionSchema*>& schemata, bool approximate, bool novelty) {
	// Simply upcast the shared_ptrs
	std::vector<std::shared_ptr<BaseActionCSPHandler>> handlers;
	for (const auto& element:create_derived(schemata, approximate, novelty)) {
		handlers.push_back(std::static_pointer_cast<BaseActionCSPHandler>(element));
	}
	return handlers;
}

std::vector<std::shared_ptr<ActionSchemaCSPHandler>> ActionSchemaCSPHandler::create_derived(const std::vector<const ActionSchema*>& schemata, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<ActionSchemaCSPHandler>> handlers;
	
	for (auto schema:schemata) {
		auto handler = std::make_shared<ActionSchemaCSPHandler>(*schema, approximate, novelty);
		FDEBUG("main", "Generated CSP for action schema" << *schema << std::endl <<  *handler << std::endl);
		handlers.push_back(handler);
	}
	return handlers;
}


ActionSchemaCSPHandler::ActionSchemaCSPHandler(const ActionSchema& action, const std::vector<const fs::ActionEffect*>& effects, bool approximate, bool novelty)
:  BaseActionCSPHandler(action, effects, approximate, novelty)
{
	index_parameters();
}

ActionSchemaCSPHandler::ActionSchemaCSPHandler(const ActionSchema& action, bool approximate, bool novelty)
:  ActionSchemaCSPHandler(action,  action.getEffects(), approximate, novelty)
{}


void ActionSchemaCSPHandler::index_parameters() {
	// Index in '_parameter_variables' the (ordered) CSP variables that correspond to the action parameters
	const ActionSchema& schema = static_cast<const ActionSchema&>(_action);
	const Signature& signature = schema.getSignature();
	for (unsigned i = 0; i< signature.size(); ++i) {
		// We here assume that the parameter IDs are always 0..k-1, where k is the number of parameters
		fs::BoundVariable variable(i, signature[i]);
		_parameter_variables.push_back(_translator.resolveVariableIndex(&variable, CSPVariableType::Input));
	}
}

Binding ActionSchemaCSPHandler::build_binding_from_solution(SimpleCSP* solution) const {
	std::vector<int> values;
	values.reserve(_parameter_variables.size());
	for (unsigned csp_var_idx:_parameter_variables) {
		values.push_back(_translator.resolveValueFromIndex(csp_var_idx, *solution));
	}
	return Binding(values);
}

// Simply forward to the more concrete method
const ActionID* ActionSchemaCSPHandler::get_action_id(SimpleCSP* solution) const {
	return get_lifted_action_id(solution);
}

LiftedActionID* ActionSchemaCSPHandler::get_lifted_action_id(SimpleCSP* solution) const {
	return new LiftedActionID(_action.getId(), build_binding_from_solution(solution));
}

void ActionSchemaCSPHandler::log() const {
	FFDEBUG("heuristic", "Processing action schema #" << _action.getId() << ": " << _action.fullname());
}
} } // namespaces
