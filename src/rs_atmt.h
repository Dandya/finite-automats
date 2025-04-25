
#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "automat.h"
#include "conf_parser.h"

namespace atmt {

class RsFunction {
	std::size_t size_ = 0;
	std::size_t size_b_ = 0;
	std::uint8_t* addr_ = nullptr;

 public:
	RsFunction() {}

	~RsFunction() {
		assert(!addr_);
	}

	void Init(const std::filesystem::path& file);

	void Delete();

	std::uint8_t operator[](std::size_t offset);
};

struct RsData {
	std::size_t n;
	RsFunction y;
	RsFunction g;

	~RsData() {
		y.Delete();
		g.Delete();
	}
};

struct RsConfigParser : public ConfigParser {
	RsConfigParser(const std::filesystem::path& file):
			ConfigParser(file) {}

	std::shared_ptr<RsData> Parse(const std::filesystem::path& dir);
};

class RsAutomat : public Automat<std::uint32_t, bool, bool> {
	std::shared_ptr<RsData> data_;
	std::size_t mask_ = 0;

 public:
	RsAutomat() {}

	~RsAutomat() {
		assert(!data_);
	}

	void Init(std::shared_ptr<RsData>& data, Elm start) {
		data_ = data;
		elm_ = start;
		mask_ = 0xFFFFFFFF >> (32 - data->n);
	}

	void Delete() {
		data_.reset();
	}

	void Next(const In& in);

	void PrintElm();

	void PrintOut();
};

}  // namespace atmt
