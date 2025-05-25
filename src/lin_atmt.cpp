
#include "lin_atmt.h"

#include <iostream>
#include <iterator>
#include <cmath>
#include <stack>
#include <unordered_map>

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
			tmp.data()[i] = static_cast<double>(k.data()[i]);
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
			//std::cout << "K_" << degree << ": \n" << k << "\n";
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
	auto count = std::powl(static_cast<double>(data_->q), rang_k);
	auto all = std::powl(static_cast<double>(data_->q), data_->A.rows());
	std::cout << "Degree of distinctness: " << degree  << '\n';
	std::cout << "Weight: " << count << '\n';
	std::cout << "Is minimal: " << (count == all) << '\n';
}

std::shared_ptr<LinAutomat::AdjacencyMatrix>
LinAutomat::CreateNotDirectedAdjacencyMatrix() {
	auto pmtrx = std::make_shared<AdjacencyMatrix>();
	auto& mtrx = *pmtrx;
	Matrix elm(1, data_->A.rows());
	Matrix x(1, data_->B.rows());
	std::memset(elm.data(), 0, data_->A.rows() * sizeof(*elm.data()));
	std::uint64_t q = data_->q;
	auto mod = [&q](std::uint64_t val) {
		return val % q;
	};
	auto next = [&q](Matrix& mtrx) -> bool {
		std::size_t cols = mtrx.cols();
		for (std::size_t i = 0; i < cols; i++) {
			mtrx.data()[i] += 1;
			if (mtrx.data()[i] == q) [[unlikely]] {
				mtrx.data()[i] = 0;
				if (i != cols-1) [[likely]]
					continue;
				else [[unlikely]]
					break;
			} else { [[likely]]
				return true;
			}
		}
		return false;
	};
	do {
		std::memset(x.data(), 0, data_->B.rows() * sizeof(*x.data()));
		do {
			SetElm(elm);
			Next(x);
			mtrx[elm][elm_] = true;
			mtrx[elm_][elm] = true;
		} while (next(x));
	} while (next(elm));
	return pmtrx;
}

bool
LinAutomat::IsConnectivityAutomat(AdjacencyMatrix& ndir_mtrx) {
	std::size_t size = data_->A.rows();
	std::uint64_t q = data_->q;
	std::unordered_map<Matrix, bool, MatrixHash> visited;
	std::stack<Matrix> stack;
	auto next = [&q](Matrix& mtrx) -> bool {
		std::size_t cols = mtrx.cols();
		for (std::size_t i = 0; i < cols; i++) {
			mtrx.data()[i] += 1;
			if (mtrx.data()[i] == q) [[unlikely]] {
				mtrx.data()[i] = 0;
				if (i != cols-1) [[likely]]
					continue;
				else [[unlikely]]
					break;
			} else { [[likely]]
				return true;
			}
		}
		return false;
		};
	auto dfs = [&visited, &stack, &ndir_mtrx, &size, &q, &next](const Matrix& elm, std::vector<Matrix>& comp) {
		comp.push_back(elm);
		visited[elm] = true;
		Matrix dst(1, size);
		std::memset(dst.data(), 0, size * sizeof(*elm.data()));
		do {
			if (ndir_mtrx[elm][dst] && !visited[dst])
				stack.push(dst);
		} while (next(dst));
	};

	std::vector<std::vector<Elm>> components;
	Matrix src(1, size);
	std::memset(src.data(), 0, size * sizeof(*src.data()));
	do {
		if (!visited[src]) {
			auto& comp = components.emplace_back();
			dfs(src, comp);
			while (!stack.empty()) {
				Elm elm = stack.top();
				stack.pop();
				if (!visited[elm])
					dfs(elm, comp);
			}
		}
	} while (next(src));

	std::cout << "Count of connectivity component: " << components.size() << "\n";

	return components.size() == 1;
}

bool
LinAutomat::IsHighConnectivityAutomat() {
	using MatrixF = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
	std::uint32_t q = data_->q;
	std::size_t rang_m = 0;
	std::size_t step = 0;
	std::size_t n = data_->A.rows();
	Matrix m = data_->B;
	Matrix new_part = data_->B;
	const std::size_t cols_count = data_->A.rows();
	auto mod = [&q](std::uint64_t val) {
		return val % q;
		};
	auto convert_to_double = [&m]() -> MatrixF {
		MatrixF tmp(m.rows(), m.cols());
		for (std::size_t i = 0; i < m.size(); ++i)
			tmp.data()[i] = static_cast<double>(m.data()[i]);
		return tmp;
		};
	if (!m.isZero()) {
		for (; step < n; step++) {
			rang_m = Eigen::ColPivHouseholderQR<MatrixF>(convert_to_double()).rank();
			if (rang_m == n) {
				return true;
			}
			new_part = (new_part * data_->A).unaryExpr(mod);
			Matrix tmp = m;
			std::size_t m_rows_count = m.rows();
			m.resize(m_rows_count + new_part.rows(), cols_count);
			m.block(0, 0, new_part.rows(), cols_count) = new_part;
			m.block(new_part.rows(), 0, m_rows_count, cols_count) = tmp;
		}
	}
	return false;
}

void
LinAutomat::Print—onnectivityInfo() {
	bool is_high_connectivity = IsHighConnectivityAutomat();
	bool is_connectivity = is_high_connectivity;
	if (!is_connectivity) {
		auto p_ndir_mtrx = CreateNotDirectedAdjacencyMatrix();
		is_connectivity = IsConnectivityAutomat(*p_ndir_mtrx);
	}
	std::cout << "Connectivity: " << is_connectivity << "\n";
	std::cout << "High connectivity: " << is_high_connectivity << "\n";
}

}