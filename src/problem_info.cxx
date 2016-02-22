
#include <iostream>
#include <fstream>

#include <problem_info.hxx>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <atom.hxx>

namespace fs0 {

ProblemInfo::ProblemInfo(const rapidjson::Document& data) {
	
	std::cout << "\t Loading Type index..." << std::endl;
	loadTypeIndex(data["types"]); // Order matters
	
	std::cout << "\t Loading Object index..." << std::endl;
	loadObjectIndex(data["objects"]);

	std::cout << "\t Loading Symbol index..." << std::endl;
	loadSymbolIndex(data["symbols"]);
	
	std::cout << "\t Loading Variable index..." << std::endl;
	loadVariableIndex(data["variables"]);
	
	loadProblemMetadata(data["problem"]);
	
	std::cout << "\t All indexes loaded!" << std::endl;
}

const std::string& ProblemInfo::getVariableName(VariableIdx index) const { return variableNames.at(index); }

unsigned ProblemInfo::getNumVariables() const { return variableNames.size(); }

const std::string ProblemInfo::getObjectName(VariableIdx varIdx, ObjectIdx objIdx) const {
	const ObjectType generictype = variableGenericTypes.at(varIdx);
	if (generictype == ObjectType::OBJECT) return getCustomObjectName(objIdx);
	else if (generictype == ObjectType::INT) return std::to_string(objIdx);
	else if (generictype == ObjectType::BOOL) return std::string((objIdx ? "true" : "false"));
	throw std::runtime_error("Should never get here.");
}

const std::string ProblemInfo::deduceObjectName(ObjectIdx object, TypeIdx type) const {
	const ObjectType generictype = getGenericType(type);
	if (generictype == ObjectType::OBJECT) return getCustomObjectName(object);
	else if (generictype == ObjectType::INT) return std::to_string(object);
	else if (generictype == ObjectType::BOOL) return std::string((object ? "true" : "false"));
	throw std::runtime_error("Should never get here.");
}

const std::string& ProblemInfo::getCustomObjectName(ObjectIdx objIdx) const { return objectNames.at(objIdx); }

unsigned ProblemInfo::getNumObjects() const { return objectNames.size(); }


void ProblemInfo::loadVariableIndex(const rapidjson::Value& data) {
	assert(variableNames.empty());
	
	for (unsigned i = 0; i < data.Size(); ++i) {
		unsigned id = variableNames.size();
		assert(data[i]["id"].GetInt() == id); // Check values are decoded in the proper order
		
		const std::string type(data[i]["type"].GetString());
		const std::string name(data[i]["name"].GetString());
		variableNames.push_back(name);
		variableIds.insert(std::make_pair(name, id));
		
		variableGenericTypes.push_back(getGenericType(getTypeId(type)));
		try {
			variableTypes.push_back(name_to_type.at(type));
		} catch( std::out_of_range& ex ) {
			throw std::runtime_error("Unknown type " + type);
		}
		
		// Load the info necessary to resolve state variables dynamically
		const auto& var_data = data[i]["data"];
		unsigned symbol_id = var_data[0].GetInt();
		 std::vector<ObjectIdx> constants;
		for (unsigned j = 0; j < var_data[1].Size(); ++j) {
			constants.push_back(var_data[1][j].GetInt());
		}
		
		variableDataToId.insert(std::make_pair(std::make_pair(symbol_id, constants),  id));
	}
}

std::vector<const ObjectIdxVector*> ProblemInfo::getSignatureValues(const Signature& signature) const {
	std::vector<const ObjectIdxVector*> res;
	for (TypeIdx type:signature) {
		res.push_back(&getTypeObjects(type));
	}
	return res;
}

VariableIdx ProblemInfo::getVariableId(unsigned symbol_id, const std::vector<ObjectIdx>& subterms) const {
	return variableDataToId.at(std::make_pair(symbol_id, subterms));
}

void ProblemInfo::loadSymbolIndex(const rapidjson::Value& data) {
	assert(symbolIds.empty());
	

	// Symbol data is stored as: # <symbol_id, symbol_name, symbol_type, <function_domain>, function_codomain, state_variables, static?>
	for (unsigned i = 0; i < data.Size(); ++i) {
		const unsigned id = data[i][0].GetInt();
		const std::string name(data[i][1].GetString());
		assert(id == symbolIds.size()); // Check values are decoded in the proper order
		symbolIds.insert(std::make_pair(name, id));
		symbolNames.push_back(name);

		// Parse the symbol type: function or predicate
		const std::string symbol_type = data[i][2].GetString();
		assert (symbol_type == "function" || symbol_type == "predicate");
		const SymbolData::Type type = (symbol_type == "function") ? SymbolData::Type::FUNCTION : SymbolData::Type::PREDICATE;
		
		// Parse the domain IDs
		const auto& domains = data[i][3];
		Signature domain;
		for (unsigned j = 0; j < domains.Size(); ++j) {
			domain.push_back(getTypeId(domains[j].GetString()));
		}
		
		// Parse the codomain ID
		TypeIdx codomain = getTypeId(data[i][4].GetString());
		
		// Parse the function variables
		std::vector<VariableIdx> variables;
		const auto& list = data[i][5];
		for (unsigned j = 0; j < list.Size(); ++j) {
			variables.push_back(list[j][0].GetInt());
		}
		
		bool is_static = data[i][6].GetBool();
		functionData.push_back(SymbolData(type, domain, codomain, variables, is_static));
	}
}


ProblemInfo::ObjectType ProblemInfo::getGenericType(TypeIdx typeId) const {
	if (isTypeBounded[typeId]) return ObjectType::INT;
// 	else if (type == "_bool_" || type == "bool") return ObjectType::BOOL;
	else return ObjectType::OBJECT;
}

//! Load the names of the problem objects from the specified file.
void ProblemInfo::loadObjectIndex(const rapidjson::Value& data) {
	assert(objectNames.empty());
	
	for (unsigned i = 0; i < data.Size(); ++i) {
		assert(data[i]["id"].GetInt() == objectNames.size()); // Check values are decoded in the proper order
		const std::string& name = data[i]["name"].GetString();
		objectIds.insert(std::make_pair(name, objectNames.size()));
		objectNames.push_back(name);
	}
}


void ProblemInfo::loadTypeIndex(const rapidjson::Value& data) {
	assert(type_to_name.empty());
	unsigned num_types = data.Size();
	
	typeObjects.resize(num_types); // Resize the vector to the number of types that we have
	isTypeBounded.resize(num_types);
	typeBounds.resize(num_types);
	
	for (unsigned i = 0; i < data.Size(); ++i) {
		TypeIdx type_id = data[i][0].GetInt();
		std::string type_name(data[i][1].GetString());
		assert(type_id == type_to_name.size()); // Check values are decoded in the proper order
		
		name_to_type.insert(std::make_pair(type_name, type_id));
		type_to_name.push_back(type_name);
		
		// We read and convert to integer type the vector of Object indexes
		if (data[i][2].IsString()) {
			assert(std::string(data[i][2].GetString()) == "int" && data[i].Size() == 4);
			int lower = data[i][3][0].GetInt();
			int upper = data[i][3][1].GetInt();
			if (lower > upper) throw std::runtime_error("Incorrect bounded integer expression: [" + std::to_string(lower) + ", " + std::to_string(upper) + "]");
			
			typeBounds[type_id] = std::make_pair(lower, upper);
			isTypeBounded[type_id] = true;
			
			// Unfold the range
			typeObjects[type_id].reserve(upper - lower + 1);
			for (int v = lower; v <= upper; ++v) typeObjects[type_id].push_back(v);
		} else { // We have an enumeration of object IDs
			typeObjects[type_id].reserve(data[i][2].Size());
			for (unsigned j = 0; j < data[i][2].Size(); ++j) {
				typeObjects[type_id].push_back(boost::lexical_cast<ObjectIdx>(data[i][2][j].GetString()));
			}
		}
	}
}

bool ProblemInfo::checkValueIsValid(const Atom& atom) const {
	return checkValueIsValid(atom.getVariable(), atom.getValue());
}

bool ProblemInfo::checkValueIsValid(VariableIdx variable, ObjectIdx value) const {
	TypeIdx type = getVariableType(variable);
	if (!isTypeBounded[type]) return true;
	const auto& bounds = typeBounds[type];
	return value >= bounds.first && value <= bounds.second;
}


void ProblemInfo::loadProblemMetadata(const rapidjson::Value& data) {
	setDomainName(data["domain"].GetString());
	setInstanceName(data["instance"].GetString());
}

} // namespaces
