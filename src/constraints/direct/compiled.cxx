
#include <constraints/direct/compiled.hxx>
#include <utils/projections.hxx>
#include <utils/utils.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 {

CompiledUnaryConstraint::CompiledUnaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension) : 
	UnaryDirectConstraint(scope, parameters), _extension(extension)
{}

CompiledUnaryConstraint::CompiledUnaryConstraint(const UnaryDirectConstraint& constraint) :
	CompiledUnaryConstraint(constraint.getScope(), constraint.getParameters(), _compile(constraint))
{}

CompiledUnaryConstraint::CompiledUnaryConstraint(const VariableIdxVector& scope, const Tester& tester) 
	: CompiledUnaryConstraint(scope, {}, compile(scope, tester))
{}

std::unordered_set<CompiledUnaryConstraint::ElementT> CompiledUnaryConstraint::compile(const VariableIdxVector& scope, const CompiledUnaryConstraint::Tester& tester) {
	assert(scope.size() == 1);
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	
	VariableIdx relevant = scope[0];
	
	std::unordered_set<ElementT> ordered;
	for(ObjectIdx value:info.getVariableObjects(relevant)) {
		if (tester(value)) ordered.insert(value);
	}
	return ordered;
}


CompiledUnaryConstraint::ExtensionT CompiledUnaryConstraint::_compile(const UnaryDirectConstraint& constraint) {
	// Depending on the type of representation, we can directly return the compiled data structure or not.
	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
		auto ordered = compile(constraint);
		return ExtensionT(ordered.begin(), ordered.end());
		
	#else
		return compile(constraint);
	#endif
}

bool CompiledUnaryConstraint::isSatisfied(ObjectIdx o) const {
	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
	return std::binary_search(_extension.begin(), _extension.end(), o); // TODO - Change for a O(1) lookup in a std::unordered_set ?
	#else
	return _extension.find(o) != _extension.end();
	#endif
}

#ifndef EXTENSIONAL_REPRESENTATION_USES_VECTORS
namespace detail{

template <typename ExtensionType>
class	IntersectionTest
{
public:
	IntersectionTest( const ExtensionType& ext, Domain& dst ) 
	: _ext( ext ), _dst( dst ) {}

	void operator()( ObjectIdx o )  {
		if ( _ext.find( o ) == _ext.end() ) return;
		_dst.insert(o);
	}

	const ExtensionType& _ext;
	Domain& _dst;
};

}
#endif

FilteringOutput CompiledUnaryConstraint::filter(const DomainMap& domains) const {
	
	DomainVector projection = Projections::project(domains, _scope);
	assert(projection.size() == 1);
	Domain& domain = *(projection[0]);
	Domain new_domain;

	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
	std::set_intersection(domain.begin(), domain.end(), _extension.begin(), _extension.end(), std::inserter(new_domain, new_domain.end()));
	#else
	std::for_each( domain.begin(), domain.end(), detail::IntersectionTest<ExtensionT>( _extension, new_domain ) ); 
	#endif
	
	if (new_domain.size() == domain.size()) return FilteringOutput::Unpruned;
	if (new_domain.size() == 0) return FilteringOutput::Failure;
	
	// Otherwise the domain has necessarily been pruned
	domain = new_domain;  // Update the domain with the new values using the assignment operator
	return FilteringOutput::Pruned;
}


CompiledBinaryConstraint::CompiledBinaryConstraint(const BinaryDirectConstraint& constraint, const ProblemInfo& problemInfo) :
	CompiledBinaryConstraint(constraint.getScope(), constraint.getParameters(), compile(constraint))
{}

CompiledBinaryConstraint::CompiledBinaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, const CompiledBinaryConstraint::TupleExtension& extension) 
	: BinaryDirectConstraint(scope, parameters), _extension1(index(extension, 0)),  _extension2(index(extension, 1))
{}

CompiledBinaryConstraint::CompiledBinaryConstraint(const VariableIdxVector& scope, const CompiledBinaryConstraint::Tester& tester) 
	: CompiledBinaryConstraint(scope, {}, compile(scope, tester))
{}


bool CompiledBinaryConstraint::isSatisfied(ObjectIdx o1, ObjectIdx o2) const {
	auto iter = _extension1.find(o1);
	assert(iter != _extension1.end());
	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
	const ObjectIdxVector& D_y = iter->second; // iter->second contains all the elements y of the domain of the second variable such that <x, y> satisfies the constraint
	return std::binary_search(D_y.begin(), D_y.end(), o2); // TODO - Change for a O(1) lookup in a std::unordered_set ?
	#else
	const ObjectIdxHash& D_y = iter->second;
	return D_y.find( o2 ) != D_y.end();
	#endif
}

CompiledBinaryConstraint::TupleExtension CompiledBinaryConstraint::compile(const VariableIdxVector& scope, const CompiledBinaryConstraint::Tester& tester) {
	assert(scope.size()==2);
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	
	TupleExtension extension;
	
	for(ObjectIdx x:info.getVariableObjects(scope[0])) {
		for(ObjectIdx y:info.getVariableObjects(scope[1])) {
			if (tester(x, y)) {
				extension.insert(std::make_tuple(x, y));
			}
		}
	}
	return extension;
}


CompiledBinaryConstraint::ExtensionT CompiledBinaryConstraint::index(const CompiledBinaryConstraint::TupleExtension& extension, unsigned variable) {
	assert(variable == 0 || variable == 1);
	std::map<ObjectIdx, std::set<ObjectIdx>> values;
	
	// We perform two passes to make sure that elements are sorted, although depending on the compilation flags this is unnecessary
	
	for (const auto& tuple:extension) {
		ObjectIdx x = (variable == 0) ? std::get<0>(tuple) : std::get<1>(tuple);
		ObjectIdx y = (variable == 0) ? std::get<1>(tuple) : std::get<0>(tuple);
		values[x].insert(y);
	}

	ExtensionT res;
	for (const auto& element:values) {
#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
		res.insert(std::make_pair(element.first, ObjectIdxVector(element.second.begin(), element.second.end()) ));
#else
		res.insert( std::make_pair(element.first, ObjectIdxHash(element.second.begin(), element.second.end() ) ));
#endif
	}
	return res;
}


FilteringOutput CompiledBinaryConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	unsigned other = (variable == 0) ? 1 : 0;
	const ExtensionT& extension_map = (variable == 0) ? _extension1 : _extension2;
	
	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);
	Domain new_domain;
	
	for (ObjectIdx x:domain) {
		auto iter = extension_map.find(x);
		assert(iter != extension_map.end());
		#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
		const ObjectIdxVector& D_y = iter->second; // iter->second contains all the elements y of the domain of the second variable such that <x, y> satisfies the constraint
		#else
		const ObjectIdxHash& D_y = iter->second;
		#endif
		if (!Utils::empty_intersection(other_domain.begin(), other_domain.end(), D_y.begin(), D_y.end())) {
			new_domain.insert(new_domain.cend(), x); //  x is an arc-consistent value. We will insert on the end of the container, as it is already sorted.
		}
	}
	if (new_domain.size() == domain.size()) return FilteringOutput::Unpruned;
	if (new_domain.size() == 0) return FilteringOutput::Failure;

	// Otherwise the domain has necessarily been pruned
	domain = new_domain; // Update the domain by using the assignment operator.
	return FilteringOutput::Pruned;
}

	
CompiledUnaryEffect::CompiledUnaryEffect(VariableIdx relevant, VariableIdx affected, ExtensionT&& extension)
	: UnaryDirectEffect(relevant, affected, {}), _extension(extension)
{}
	
CompiledUnaryEffect::CompiledUnaryEffect(VariableIdx relevant, VariableIdx affected, const Term& term) 
	: CompiledUnaryEffect(relevant, affected, compile(term, Problem::getCurrentProblem()->getProblemInfo()))
{}
	

Atom CompiledUnaryEffect::apply(ObjectIdx value) const {
	return Atom(_affected, _extension.at(value));
}

CompiledUnaryEffect::ExtensionT CompiledUnaryEffect::compile(const UnaryDirectEffect& effect, const fs0::ProblemInfo& info) {
	VariableIdx relevant = effect.getScope()[0];
	const ObjectIdxVector& all_values = info.getVariableObjects(relevant);
	
	ExtensionT map;
	for(ObjectIdx value:all_values) {
		try {
			auto atom = effect.apply(value);
			assert(atom.getVariable() == effect.getAffected());
			map.insert(std::make_pair(value, atom.getValue()));
		} catch(const std::out_of_range& e) {  // TODO - Refactor this, too hacky...
			// If the effect produces an exception, we simply consider it non-applicable and go on.
		}
	}
	return map;
}

CompiledUnaryEffect::ExtensionT CompiledUnaryEffect::compile(const Term& term, const ProblemInfo& info) {
	VariableIdxVector scope = term.computeScope();
	assert(scope.size() == 1);
	ExtensionT map;
	
	for(ObjectIdx value:info.getVariableObjects(scope[0])) {
		ObjectIdx out = term.interpret(Projections::zip(scope, {value}));
		map.insert(std::make_pair(value, out));
	}
	return map;
}

CompiledBinaryEffect::CompiledBinaryEffect(const VariableIdxVector& scope, VariableIdx affected, ExtensionT&& extension) 
	: BinaryDirectEffect(scope, affected, {}), _extension(extension)
{}

CompiledBinaryEffect::CompiledBinaryEffect(const VariableIdxVector& scope, VariableIdx affected, const fs::Term& term)
	: CompiledBinaryEffect(scope, affected, compile(term, Problem::getCurrentProblem()->getProblemInfo()))
{}

CompiledBinaryEffect::ExtensionT CompiledBinaryEffect::compile(const fs::Term& term, const ProblemInfo& info) {
	VariableIdxVector scope = term.computeScope();
	assert(scope.size() == 2);
	ExtensionT map;
	
	for(ObjectIdx x:info.getVariableObjects(scope[0])) {
		for(ObjectIdx y:info.getVariableObjects(scope[1])) {
			ObjectIdx out = term.interpret(Projections::zip(scope, {x, y}));
			map.insert(std::make_pair(std::make_pair(x, y), out));
		}
	}
	return map;
}


Atom CompiledBinaryEffect::apply(ObjectIdx v1, ObjectIdx v2) const {
	return Atom(_affected, _extension.at({v1, v2}));
}


unsigned ConstraintCompiler::compileConstraints(std::vector<DirectConstraint::cptr>& constraints) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	
	unsigned num_compiled = 0;
	for (unsigned i = 0; i < constraints.size(); ++i) {
		DirectConstraint::cptr compiled = constraints[i]->compile(info);
		if (compiled) { // The constraint type requires pre-compilation
			delete constraints[i];
			constraints[i] = compiled;
			++num_compiled;
		}
	}
	return num_compiled;	
}


} // namespaces
