
#include "lin_atmt.h"

#include <iostream>
#include <iterator>

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

}