
#pragma once

#include <filesystem>
#include <fstream>
#include <memory>

namespace atmt {

class ConfigParser {
 protected:	
	std::ifstream in_;

 public:
	ConfigParser(const std::filesystem::path& file) {
		in_.open(file.filename());
	}

	~ConfigParser() {
		in_.close();
	}
};

}  // namespace atmt
