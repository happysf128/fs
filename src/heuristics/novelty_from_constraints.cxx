#include <heuristics/novelty_from_constraints.hxx>
#include <iostream>

namespace fs0 {

  NoveltyFromConstraintsAdapter::NoveltyFromConstraintsAdapter( const GenericState& s, const NoveltyFromConstraints& featureMap )
		: _adapted( s ), _featureMap( featureMap) {}

	NoveltyFromConstraintsAdapter::~NoveltyFromConstraintsAdapter() { }

	NoveltyFromConstraints::~NoveltyFromConstraints() { }

  void NoveltyFromConstraints::selectFeatures( const Problem& theProblem, bool useGoal ) {
    // MRJ: add to the set of features global and goal constraints
    for ( ScopedConstraint::cptr c : theProblem.getConstraints() )
      _features.push_back(c);

    if ( useGoal ) {
      for ( ScopedConstraint::cptr c : theProblem.getGoalConstraints() )
        _features.push_back(c);
    }

    for ( Action::cptr a : theProblem.getAllActions() )
      for ( ScopedConstraint::cptr c : a->getConstraints() ) {
        // Leave out bounds constraints!
        if ( dynamic_cast< const UnaryDomainBoundsConstraint*>(c) != nullptr ) continue;
        if ( dynamic_cast< const BinaryDomainBoundsConstraint*>(c) != nullptr ) continue;
        _features.push_back(c);
      }

    std::cout << "Novelty From Constraints: # features: " << numFeatures() << std::endl;
  }

}
