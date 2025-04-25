
#include "lin_atmt.h"

#include <iostream>
#include <iterator>

namespace atmt {

void
Matrix::Print() const {
	for (std::size_t i = 0; i < r_; ++i) {
		for (std::size_t j = 0; j < c_; ++j)
			std::cout << data_[i * c_ + j] << " ";
		std::cout << "\n";
	}
}

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
		std::vector<std::uint32_t> mtx_data;
		std::size_t count = r * c;
		mtx_data.resize(count);
		for (std::size_t i = 0; i < count; ++i) {
			mtx_data[i] = *values % q;
			values++;
		}
		mtx = Matrix(mtx_data, r, c, q);
	};
	read_matrix(n, n, data->q, data->A);
	read_matrix(m, n, data->q, data->B);
	read_matrix(n, k, data->q, data->C);
	read_matrix(m, k, data->q, data->D);
	return data;
}

void
LinAutomat::Next(const In& in) {
	auto print_size = [](const char* text, const Matrix& m) {
		std::cout << text << m.Row() << "x" << m.Column() << "\n";
	};
	print_size("elm: ", elm_);
	print_size("in: ", in);
	print_size("A: ", data_->A);
	print_size("B: ", data_->B);
	print_size("C: ", data_->C);
	print_size("D: ", data_->D);
	auto res = (elm_ * data_->C);
	print_size("res: ", res);
	res = (in * data_->D);
	print_size("res: ", res);
	
	// print_size("elm: ", elm_);
	last_out_ = (elm_ * data_->C) + (in * data_->D);
	elm_ = (elm_ * data_->A) + (in * data_->B);
	last_in_ = in;
}

void
LinAutomat::PrintElm() {
	std::cout << "Element:\n";
	elm_.Print();
}

void
LinAutomat::PrintOut() {
	std::cout << "Out:\n";
	last_out_.Print();
}

}