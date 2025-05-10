
#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include <iostream>

#include <Eigen/Dense>

#include "automat.h"
#include "conf_parser.h"

namespace atmt {

using Matrix =
		Eigen::Matrix<std::uint64_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

struct LinData {
	std::uint32_t q = 0;
	Matrix A;
	Matrix B;
	Matrix C;
	Matrix D;
};

struct LinConfigParser : public ConfigParser {
	LinConfigParser(const std::filesystem::path& file):
			ConfigParser(file) {}

	std::shared_ptr<LinData> Parse();
};

class LinAutomat : public Automat<Matrix, Matrix, Matrix> {
	std::shared_ptr<LinData> data_;

 public:
 LinAutomat() {}

	~LinAutomat() {
		assert(!data_);
	}

	void Init(std::shared_ptr<LinData>& data, Elm start) {
		data_ = data;
		elm_ = start;
	}

	void Delete() {
		data_.reset();
	}

	void Next(const In& in);

	void PrintElm();

	void PrintOut();

	void PrintEquivalenceInfo();
};

}  // namespace atmt
