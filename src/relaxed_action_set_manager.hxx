
#pragma once

#include <iosfwd>
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_types.hxx>
#include <fact.hxx>
#include <constraints/constraints.hxx>
#include <utils/utils.hxx>

namespace aptk { namespace core {

/**
 * An action manager operating on a delete-free relaxation of the problem.
 */
class RelaxedActionSetManager
{
protected:
	//! The original state.
	const State* seed;

	//! The state constraints
	const ProblemConstraint::vctr& _constraints;

public:
	RelaxedActionSetManager(const ProblemConstraint::vctr& constraints)
		: RelaxedActionSetManager(NULL, constraints) {}
	
	RelaxedActionSetManager(const State* originalState, const ProblemConstraint::vctr& constraints)
		: seed(originalState), _constraints(constraints) {}

	RelaxedActionSetManager(const RelaxedActionSetManager& other)
		: seed(other.seed), _constraints(other._constraints)  {}
	
	//!
	std::pair<bool, FactSetPtr> isApplicable(const ApplicableEntity& entity, const DomainMap& domains) const;
	
	//!
	void computeChangeset(const CoreAction& action, const DomainMap& domains, Changeset& changeset) const;
	
protected:
	bool isProcedureApplicable(const ApplicableEntity& entity, const DomainMap& domains, unsigned procedureIdx, FactSetPtr causes) const;
	
	void computeProcedureChangeset(unsigned procedureIdx, const CoreAction& action, const DomainMap& domains, Changeset& changeset) const;
	void computeProcedurePointChangeset(unsigned procedureIdx, const CoreAction& action,
										const VariableIdxVector& relevant, const ProcedurePoint& point, Changeset& changeset) const;
};

} } // namespaces
