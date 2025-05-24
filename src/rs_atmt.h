
#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "automat.h"
#include "conf_parser.h"
#include "mmap_file.h"

namespace atmt {

struct TMatrixData {
	std::array<std::uint32_t, 2> next;
	std::array<bool, 2> out;
};

class TMatrix : public MmapFile {
 public:
	TMatrix(const std::filesystem::path& file):
			MmapFile(file, MmapMode::READ) {}

	TMatrixData Get(std::uint32_t elm) {
		return reinterpret_cast< TMatrixData*>(addr_)[elm];
	}
};


class EqClasses : public MmapFile {
public:
	std::size_t count = 0;

	EqClasses(const std::filesystem::path& file):
			MmapFile(file, MmapMode::READ_WRITE) {}

	std::uint32_t Get(std::uint32_t elm) {
		return reinterpret_cast<std::uint32_t*>(addr_)[elm];
	}
};

class RsFunction : public MmapFile {
	std::size_t size_b_ = 0;

 public:
	RsFunction(const std::filesystem::path& file):
			MmapFile(file, MmapMode::READ), size_b_(size_ << 3) {}

	std::uint8_t operator[](std::size_t offset) {
		return ((reinterpret_cast<std::uint8_t*>(addr_)[(offset >> 3)] >> (offset & 0b111)) & 0b1);
	}
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
	 using AdjacencyMatrix = std::vector<std::vector<bool>>;

	RsAutomat() {}

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

	void PrintEqualClasses(EqClasses& eq);

	void PrintEquivalenceInfo(const std::filesystem::path& dir);

	void Print—onnectivityInfo(const std::filesystem::path& dir);

 private:
	std::shared_ptr<TMatrix> CreateTMatrix(const std::filesystem::path& dir);
	
	std::shared_ptr<EqClasses> CreateEqClasses(const std::filesystem::path& dir, bool fill);

	std::shared_ptr<AdjacencyMatrix> CreateDirectedAdjacencyMatrix();
	std::shared_ptr<AdjacencyMatrix> CreateNotDirectedAdjacencyMatrix(const AdjacencyMatrix& dir_mtrx);
	void TranspositionAdjacencyMatrix(AdjacencyMatrix& mtrx);
	bool IsConnectivityAutomat(const AdjacencyMatrix& dir_mtrx);
	bool IsHighConnectivityAutomat(AdjacencyMatrix& dir_mtrx);

	bool UpdateEqClasses(TMatrix& m, EqClasses& now, EqClasses& next);
};

}  // namespace atmt
