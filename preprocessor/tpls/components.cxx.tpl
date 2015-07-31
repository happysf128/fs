
#include "components.hxx"
#include <utils/loader.hxx>

std::unique_ptr<External> external;

void generate(const rapidjson::Document& data, const std::string& data_dir, Problem& problem) {
	external = std::unique_ptr<External>(new External(data_dir));
	ComponentFactory factory;
	Loader::loadProblem(data, factory, problem);
}