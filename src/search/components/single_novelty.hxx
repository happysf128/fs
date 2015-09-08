
#pragma once

#include <fs0_types.hxx>
#include <state_model.hxx>
#include <utils/logging.hxx>
#include <heuristics/novelty/fs0_novelty_evaluator.hxx>
#include <heuristics/unsat_goal_atoms/unsat_goal_atoms.hxx>
#include <search/components/base_novelty_component.hxx>


namespace fs0 { class Problem; class Config; }

namespace fs0 { namespace engines {

//! A novelty evaluator object to be used as an open list acceptor.
//! It accepts a new search node iff its novelty less than or equal to the max novelty bound
template <typename SearchNode>
class SingleNoveltyComponent : public BaseNoveltyComponent<SearchNode> {
protected:
	//! A single novelty evaluator will be in charge of evaluating all nodes
	GenericNoveltyEvaluator _novelty_evaluator;

public:
	typedef BaseNoveltyComponent<SearchNode> Base;
	
	SingleNoveltyComponent(const FS0StateModel& model, unsigned max_novelty, bool useStateVars, bool useGoal, bool useActions)
		: Base(max_novelty), _novelty_evaluator(model.getTask(), max_novelty, useStateVars, useGoal, useActions)
	{}
	
	~SingleNoveltyComponent() {
		for ( unsigned k = 1; k <= Base::novelty_bound(); k++ ) {
			FINFO("heuristic", "# novelty(s)=" << k << " : " << _novelty_evaluator.get_num_states(k));
		}
	}

	GenericNoveltyEvaluator& evaluator(const State& state) { return _novelty_evaluator; }
};

} } // namespaces
