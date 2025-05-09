
#include "rs_atmt.h"

#include <fcntl.h>
#ifndef WIN32
#include <sys/mman.h>
#else
#include <Windows.h>
#endif !WIN32

#include <iostream>
#include <iterator>
#include <unordered_map>

namespace atmt {

std::shared_ptr<RsData>
RsConfigParser::Parse(const std::filesystem::path& dir) {
	std::cout << "Creating shift register\n";
	std::size_t n = 0;
	in_ >> n;
	std::cout << "n: " << n << "\n";
	in_.get();
	std::istream_iterator<bool> values(in_);
	std::size_t offset = 0;
	std::size_t len = (1 << n) * 2;
	std::filesystem::path path_y = dir.string() + "/func_y";
	std::filesystem::path path_g = dir.string() + "/func_g";
	std::ofstream func_y(path_y, std::ios::binary);
	std::ofstream func_g(path_g, std::ios::binary);
	char byte = 0;
	while (values != std::istream_iterator<bool>()) {
		byte |= *values << (offset & 0b111);
		if ((offset & 0b111) == 0b111) {
			if (offset < len) {
				func_y.write(&byte, 1);
			} else {
				func_g.write(&byte, 1);
			}
			byte = 0;
		}
		values++;
		offset++;
	}
	func_y.close();
	func_g.close();
	std::shared_ptr<RsData> data = std::make_shared<RsData>(n, path_y, path_g);
	return data;
}


void
RsAutomat::Next(const In& in) {
	auto offset = [](Elm elm, In in) -> std::size_t {
		return static_cast<std::size_t>(elm) * 2 + static_cast<std::size_t>(in);
	};
	bool y_out = data_->y[offset(elm_, in)];
	last_out_ = data_->g[offset(elm_,y_out)];
	elm_ = ((elm_ << 1) | static_cast<std::uint32_t>(y_out)) & mask_;
	last_in_ = in;
}

void
RsAutomat::PrintElm() {
	std::cout << "Element: (";
	for (std::size_t i = 1; i <= data_->n; ++i)
		std::cout << ((elm_ >> (data_->n - i)) & 0b1);
	std::cout << ")\n";
}

void
RsAutomat::PrintOut() {
	std::cout << "Out: (" << last_out_ << ")\n";
}

void
RsAutomat::PrintEqualClasses(EqClasses& eq) {
	for (Elm elm = 0; elm <= mask_; ++elm)
		std::cout << elm << ':' << eq.Get(elm) << ' ';
	std::cout << "Count: " << eq.count << "\n";
}

void
RsAutomat::PrintEquivalenceInfo(const std::filesystem::path& dir) {
	auto t_mtrx = CreateTMatrix(dir);
	auto classes_now = CreateEqClasses(dir, true);
	auto classes_next = CreateEqClasses(dir, false);
	assert(t_mtrx && classes_now && classes_next);

	PrintEqualClasses(*classes_now);
	bool was_changes = true;
	std::uint32_t degree = 0;
	do {
		++degree;
		was_changes = UpdateEqClasses(*t_mtrx, *classes_now, *classes_next);
		PrintEqualClasses(*classes_next);
		if (was_changes)
			std::swap(classes_next, classes_now);
	} while (was_changes);
	std::cout << "Degree of distinctness: " << degree  << '\n';
	std::cout << "Weight: " << classes_now->count << '\n';
	std::cout << "Is minimal: " << (classes_now->count == 1) << '\n';
}

std::shared_ptr<TMatrix>
RsAutomat::CreateTMatrix(const std::filesystem::path& dir) {
	std::filesystem::path mtrx_path = dir.string() + "/tmatrix";
	std::ofstream mtrx(mtrx_path, std::ios::binary);
	TMatrixData data;
	for (Elm elm = 0; elm <= mask_; ++elm) {
		SetElm(elm);
		Next(0);
		data.next[0] = elm_;
		data.out[0] = last_out_;
		SetElm(elm);
		Next(1);
		data.next[1] = elm_;
		data.out[1] = last_out_;
		mtrx.write(reinterpret_cast<const char*>(&data), sizeof(data));
	}

	mtrx.close();

	return std::make_shared<TMatrix>(mtrx_path);
}

std::shared_ptr<EqClasses>
RsAutomat::CreateEqClasses(const std::filesystem::path& dir, bool fill)  {
	if (fill) {
		std::filesystem::path classes_path = dir.string() + "/classes_one";
		std::ofstream classes(classes_path, std::ios::binary);
		std::array<bool, 4> out_group = { false, false, false, false};
		for (Elm elm = 0; elm <= mask_; ++elm) {
			SetElm(elm);
			Next(0);
			std::uint32_t out_class = last_out_;
			SetElm(elm);
			Next(1);
			out_class |= last_out_ << 1;
			out_group[out_class] = true;
			classes.write(
					reinterpret_cast<const char*>(&out_class), sizeof(out_class));
		}

		classes.close();

		auto res = std::make_shared<EqClasses>(classes_path);
		res->count = 0;
		for (auto b : out_group)
			if (b)
				res->count++;

		return res;
	} else {
		std::filesystem::path classes_path = dir.string() + "/classes_two";
		std::ofstream classes(classes_path, std::ios::binary);
		std::uint32_t out_class = 0;
		for (Elm elm = 0; elm <= mask_; ++elm) {
			classes.write(
				reinterpret_cast<const char*>(&out_class), sizeof(out_class));
		}

		classes.close();

		return std::make_shared<EqClasses>(classes_path);
	}
}

bool
RsAutomat::UpdateEqClasses(TMatrix& m, EqClasses& now, EqClasses& next) {
	struct ClassId {
		std::uint32_t id_next_0;
		std::uint32_t id_next_1;
		std::uint16_t out;
	} key;
	struct ClassIdHash {
		std::size_t operator()(const ClassId& id) const {
			return ((static_cast<std::size_t>(id.id_next_0) << 32) + 
					static_cast<std::size_t>(id.id_next_1)) * static_cast<std::size_t>(id.out);
		}
	};
	struct ClassIdEqual {
		bool operator()(const ClassId& id1, const ClassId& id2) const {
			return id1.id_next_0 == id2.id_next_0 &&
					id1.id_next_1 == id2.id_next_1 &&
					id1.out == id2.out;
		}
	};
	std::unordered_map<ClassId, std::uint32_t, ClassIdHash, ClassIdEqual> new_classes;
	std::uint32_t next_class_id = 0;
	for (Elm elm = 0; elm <= mask_; ++elm) {
		auto data = m.Get(elm);
		key.id_next_0 = now.Get(data.next[0]);
		key.id_next_1 = now.Get(data.next[1]);
		key.out = (data.out[1] << 8) | data.out[0];
		auto iter = new_classes.find(key);
		if (iter != new_classes.end()) {
			reinterpret_cast<std::uint32_t*>(next.addr_)[elm] = iter->second;
		} else {
			new_classes[key] = next_class_id;
			reinterpret_cast<std::uint32_t*>(next.addr_)[elm] = next_class_id;
			++next_class_id;
		}
	}
	next.count = next_class_id;
	return now.count != next.count;
}

}  // namespace atmt