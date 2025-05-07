
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
		if (!std::filesystem::exists(file))
			throw std::runtime_error(file.string() + "don't exist");
		in_.open(file);
		if (in_.fail())
			throw std::runtime_error("error open config");
	}

	~ConfigParser() {
		in_.close();
	}
};

}  // namespace atmt
