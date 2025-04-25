
#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

#include "automat.h"
#include "conf_parser.h"

namespace atmt {

class Matrix {
	std::vector<std::uint32_t> data_;
	std::size_t r_ = 0;
	std::size_t c_ = 0;
	std::uint32_t q_ = 0;

 public:
	Matrix() {}

	Matrix(const std::vector<std::uint32_t>& data,
			std::size_t r, std::size_t c, std::uint32_t q):
			data_(data), r_(r), c_(c), q_(q) {}

	Matrix(std::vector<std::uint32_t>&& data,
				std::size_t r, std::size_t c, std::uint32_t q):
				data_(std::move(data)), r_(r), c_(c), q_(q) {}

	Matrix(const Matrix& m): Matrix(m.data_, m.r_, m.c_, m.q_) {}

	Matrix(Matrix&& m): Matrix(std::move(m.data_), m.r_, m.c_, m.q_) {}

	Matrix& operator=(const Matrix& m) {
		if (this != &m) {
			data_ = m.data_;
			r_ = m.r_;
			c_ = m.c_;
			q_ = m.q_;
		}
		return *this;
	}

	Matrix& operator=(Matrix&& m) {
		if (this != &m) {
			data_ = std::move(m.data_);
			r_ = m.r_;
			c_ = m.c_;
			q_ = m.q_;
		}
		return *this;
	}

	std::size_t Row() const { return r_; }
	std::size_t Column() const { return c_; }
	const auto& Data() const { return data_; }
	Matrix Copy() const { return Matrix(*this); }
	std::uint32_t Get(const std::size_t r, const std::size_t c) const {
		assert(r < r_);
		assert(c < c_);
		return data_[r * c_ + c];
	}

	void Print() const;

	friend
	Matrix operator+(const Matrix& n1, const Matrix& n2) {
		assert(n1.q_ == n2.q_);
		assert(n1.r_ == n2.r_);
		assert(n1.c_ == n2.c_);
		std::vector<std::uint32_t> new_data;
		std::size_t count = n1.r_ * n1.c_;
		new_data.resize(count);
		for (std::size_t i = 0; i < count; ++i)
			new_data[i] = (n1.data_[i] + n2.data_[i]) % n1.q_;
		return Matrix(std::move(new_data), n1.r_, n1.c_, n1.q_);
	}

	friend
	Matrix operator+(Matrix&& n1, const Matrix& n2) {
		assert(n1.q_ == n2.q_);
		assert(n1.r_ == n2.r_);
		assert(n1.c_ == n2.c_);
		std::size_t count = n1.r_ * n1.c_;
		for (std::size_t i = 0; i < count; ++i)
			n1.data_[i] = (n1.data_[i] + n2.data_[i]) % n1.q_;
		return n1;
	}

	friend
	Matrix operator+(const Matrix& n1, Matrix&& n2) {
		assert(n1.q_ == n2.q_);
		assert(n1.r_ == n2.r_);
		assert(n1.c_ == n2.c_);
		std::size_t count = n1.r_ * n1.c_;
		for (std::size_t i = 0; i < count; ++i)
			n2.data_[i] = (n1.data_[i] + n2.data_[i]) % n1.q_;
		return n2;
	}

	friend
	Matrix operator+(Matrix&& n1, Matrix&& n2) {
		return n1 + const_cast<const Matrix&>(n2);
	}

	friend
	Matrix operator-(const Matrix& n1, const Matrix& n2) {
		assert(n1.q_ == n2.q_);
		assert(n1.r_ == n2.r_);
		assert(n1.c_ == n2.c_);
		std::vector<std::uint32_t> new_data;
		std::size_t count = n1.r_ * n1.c_;
		new_data.resize(count);
		for (std::size_t i = 0; i < count; ++i)
			new_data[i] = (n1.data_[i] - n2.data_[i]) % n1.q_;
		return Matrix(std::move(new_data), n1.r_, n1.c_, n1.q_);
	}

	friend
	Matrix operator-(Matrix&& n1, const Matrix& n2) {
		assert(n1.q_ == n2.q_);
		assert(n1.r_ == n2.r_);
		assert(n1.c_ == n2.c_);
		std::size_t count = n1.r_ * n1.c_;
		for (std::size_t i = 0; i < count; ++i)
			n1.data_[i] = (n1.data_[i] - n2.data_[i]) % n1.q_;
		return n1;
	}

	friend
	Matrix operator-(const Matrix& n1, Matrix&& n2) {
		assert(n1.q_ == n2.q_);
		assert(n1.r_ == n2.r_);
		assert(n1.c_ == n2.c_);
		std::size_t count = n1.r_ * n1.c_;
		for (std::size_t i = 0; i < count; ++i)
			n2.data_[i] = (n1.data_[i] - n2.data_[i]) % n1.q_;
		return n2;
	}

	friend
	Matrix operator*(const Matrix& n1, const Matrix& n2) {
		assert(n1.q_ == n2.q_);
		assert(n1.c_ == n2.r_);
		std::vector<std::uint32_t> new_data;
		std::size_t count = n1.r_ * n2.c_;
		new_data.resize(count);
		for (std::size_t r = 0; r < n1.r_; ++r) {
			for (std::size_t c = 0; c < n2.c_; ++c) {
				auto & new_v = new_data[r * n2.c_ + c] = 0;
				for (std::size_t i = 0; i < n1.c_; ++i)
					new_v = (new_v + n1.Get(r, i) * n2.Get(i, c)) % n1.q_;
			}
		}
		return Matrix(std::move(new_data), n1.r_, n2.c_, n1.q_);
	}
};

// struct Num {
// 	std::uint32_t n = 0;
// 	std::uint32_t q = 0;

//  public:
// 	Num(std::uint32_t value, std::uint32_t q): n(value % q), q(q) {}
// 	Num(const Num& num): Num(num.n, num.q) {}
// 	Num(Num&& num): Num(num.n, num.q) {}
// 	Num& operator=(const Num& num) {
// 		n = num.n;
// 		q = num.q;
// 		return *this;
// 	}
// 	Num& operator=(Num& num) {
// 		n = num.n;
// 		q = num.q;
// 		return *this;
// 	}

// 	friend std::ostream& operator<<(std::ostream& os, const Num& num) {
// 		os << num.n;
// 		return os;
// 	}
// };

// inline
// Num operator+(const Num& n1, const Num& n2) {
// 	assert(n1.q == n2.q);
// 	return {(n1.n + n2.n) % n1.q, n1.q};
// }

// inline
// Num operator*(const Num& n1, const Num& n2) {
// 	assert(n1.q == n2.q);
// 	return {(n1.n * n2.n) % n1.q, n1.q};
// }

// inline
// Num operator/(const Num& n1, const Num& n2) {
// 	assert(n1.q == n2.q);
// 	return {(n1.n / n2.n) % n1.q, n1.q};
// }

// inline
// Num operator%(const Num& n1, const Num& n2) {
// 	assert(n1.q == n2.q);
// 	return {(n1.n % n2.n) % n1.q, n1.q};
// }

// inline
// bool operator==(const Num& n1, const Num& n2) {
// 	assert(n1.q == n2.q);
// 	return (n1.n == n2.n);
// }

struct LinData {
	std::uint32_t q;
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

	// void Next(In& in);

	void PrintElm();

	void PrintOut();
};

}  // namespace atmt
