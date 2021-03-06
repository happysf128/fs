
#pragma once

#include <fs_types.hxx>

namespace fs0 {

//! An interface for the concrete instance generators
class BaseComponentFactory {
public:
	//! To be subclassed if necessary
    virtual std::map<std::string, Function> instantiateFunctions() const { return {}; }
};

} // namespaces