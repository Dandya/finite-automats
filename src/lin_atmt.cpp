
#include "lin_atmt.h"

#include <iostream>
#include <iterator>
#include <cmath>

namespace atmt {

// void
// Matrix::Print() const {
// 	for (std::size_t i = 0; i < r_; ++i) {
// 		for (std::size_t j = 0; j < c_; ++j)
// 			std::cout << data_[i * c_ + j] << " ";
// 		std::cout << "\n";
// 	}
// }

std::shared_ptr<LinData>
LinConfigParser::Parse() {
	std::cout << "Creating linear automat\n";
	std::shared_ptr<LinData> data = std::make_shared<LinData>();
	std::istream_iterator<std::uint16_t> values(in_);
	data->q = *values;
	values++;
	std::size_t m, n, k;
	m = *values;
	values++;
	n = *values;
	values++;
	k = *values;
	values++;
	std::cout << "q: " << data->q << "\n";
	std::cout << "m: " << m << "\n";
	std::cout << "n: " << n << "\n";
	std::cout << "k: " << k << "\n";
	auto read_matrix =
			[&values] (std::size_t r, std::size_t c, std::uint32_t q, Matrix& mtx) {
		mtx.resize(r, c);
		std::size_t count = r * c;
		for (std::size_t i = 0; i < count; ++i) {
			mtx.data()[i] = *values % q;
			values++;
		}
	};
	read_matrix(n, n, data->q, data->A);
	read_matrix(m, n, data->q, data->B);
	read_matrix(n, k, data->q, data->C);
	read_matrix(m, k, data->q, data->D);
	return data;
}

void
LinAutomat::Next(const In& in) {
	std::uint32_t q = data_->q;
	auto mod = [&q](std::uint64_t val) {
		return val % q;
	};
	last_out_ = ((elm_ * data_->C) + (in * data_->D)).unaryExpr(mod);
	elm_ = ((elm_ * data_->A) + (in * data_->B)).unaryExpr(mod);
	last_in_ = in;
}

void
LinAutomat::PrintElm() {
	std::cout << "Element:\n" << elm_ << '\n';
}

void
LinAutomat::PrintOut() {
	std::cout << "Out:\n" << last_out_ << '\n';
}

void
LinAutomat::PrintEquivalenceInfo() {
	using MatrixF = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

	/*Matrix k(2, 2);
	k << 1, 2,
		3, 4;
	std::cout << "k_before: \n" << k << '\n';*/
	/*MatrixF tmp =*/
	/*k.resize(2, k.cols() + 2);
	k.block(0, 1, 2, 2) = tmp;*/
	//std::cout << "k_after: \n" << convert_to_double() << '\n';
	std::size_t rang_k = 0;
	std::size_t degree = 0;
	std::uint32_t q = data_->q;
	auto mod = [&q](std::uint64_t val) {
		return val % q;
	};
	Matrix k;
	auto convert_to_double = [&k]() -> MatrixF {
		MatrixF tmp(k.rows(), k.cols());
		for (std::size_t i = 0; i < k.size(); ++i)
			tmp.data()[i] = k.data()[i];
		return tmp;
	};
	if (!data_->C.isZero()) {
		degree = 1;
		k = data_->C;
		rang_k = Eigen::ColPivHouseholderQR<MatrixF>(convert_to_double()).rank();
		Matrix new_part = (data_->A * data_->C).unaryExpr(mod);
		std::size_t offset = data_->C.cols();
		const std::size_t rows_count = data_->A.rows();
		for (; degree < rows_count; ++degree) {
			std::cout << "K_" << degree << ": \n" << k << "\n";
			Matrix tmp = k;
			k.resize(rows_count, k.cols() + data_->C.cols());
			k.block(0, 0, rows_count, tmp.cols()) = tmp;
			k.block(0, offset, rows_count, data_->C.cols()) = new_part;
			std::size_t new_rang = Eigen::ColPivHouseholderQR<MatrixF>(convert_to_double()).rank();
			if (rang_k == new_rang)
				break;
			rang_k = new_rang;
			new_part = (data_->A * new_part).unaryExpr(mod);
			offset += data_->C.cols();
		}
	}
	auto count = std::powl(data_->q, rang_k);
	auto all = std::powl(data_->q, data_->A.rows());
	std::cout << "Degree of distinctness: " << degree  << '\n';
	std::cout << "Weight: " << count << '\n';
	std::cout << "Is minimal: " << (count == all) << '\n';
}

}