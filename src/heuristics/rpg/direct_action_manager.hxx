
#pragma once

#include <heuristics/rpg/base_action_manager.hxx>
#include <constraints/direct/csp_handler.hxx>
#include <constraints/direct/effect.hxx>

namespace fs0 {

class GroundAction;


/**
 * A constraint manager capable only of dealing with actions whose constraints are at most unary,
 * in which case the handling is much simpler and more efficient.
 * Note that this restriction in particular excludes nested-fluent terms.
 */
class DirectActionManager : public BaseActionManager
{
public:
	//! Constructs a manager handling the given set of constraints
	DirectActionManager(const GroundAction& action);
	~DirectActionManager();

	virtual void process(unsigned actionIdx, const RelaxedState& layer, RPGData& rpg);

	//!
	virtual bool checkPreconditionApplicability(const DomainMap& domains) const;

protected:
	
	//!
	void processEffects(unsigned actionIdx, const DomainMap& actionProjection, RPGData& rpg) const;
	
	//!
	const GroundAction& _action;
	
	const std::vector<DirectConstraint::cptr> _constraints;
	
	const std::vector<DirectEffect::cptr> _effects;
	
	const DirectCSPHandler _handler;
	
	//!
	void completeAtomSupport(const VariableIdxVector& actionScope, const DomainMap& actionProjection, const VariableIdxVector& effectScope, Atom::vctrp support) const;
	
	std::ostream& print(std::ostream& os) const;
};


} // namespaces
