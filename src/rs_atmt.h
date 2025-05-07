
#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "automat.h"
#include "conf_parser.h"
#include "mmap_file.h"

namespace atmt {

// class TMatrix {
// 	std::size_t size_ = 0;
// 	std::uint32_t* addr_ = nullptr;

//  public:
// 	TMatrix() {}

// 	~TMatrix() {
// 		assert(!addr_);
// 	}

// 	void Init(const std::filesystem::path& file);

// 	void Delete();

// 	std::uint32_t Get(std::uint32_t elm, bool in);
// };


//class EqClasses {
//	std::size_t size_ = 0;
//	std::tuple<void*, void*, void*> file_;
//	std::uint32_t* addr_ = nullptr;
//
// public:
//	EqClasses() {}
//
//	~EqClasses() {
//		assert(!addr_);
//	}
//
//	void Init(const std::filesystem::path& file);
//
//	void Delete();
//
//	std::uint32_t Get(std::uint32_t elm);
//};

class RsFunction : public MmapFile {
	std::size_t size_b_ = 0;

 public:
	RsFunction(const std::filesystem::path& file):
			MmapFile(file), size_b_(size_ << 3) {}

	std::uint8_t operator[](std::size_t offset);
};

struct RsData {
	std::size_t n = 0;
	RsFunction y;
	RsFunction g;

	RsData(std::size_t n, const std::filesystem::path& path_y, const std::filesystem::path& path_g):
			n(n), y(path_y), g(path_g) {}
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

	//void PrintEquivalenceInfo(const std::filesystem::path& dir);

 private:
	// std::shared_ptr<TMatrix> CreateTMatrix(const std::filesystem::path& dir);
	
	// std::shared_ptr<EqClasses> CreateEqClasses(const std::filesystem::path& dir);
};

}  // namespace atmt
