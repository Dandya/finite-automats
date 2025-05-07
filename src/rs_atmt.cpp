
#include "rs_atmt.h"

#include <fcntl.h>
#ifndef WIN32
#include <sys/mman.h>
#else
#include <Windows.h>
#endif // !WIN32

#include <iostream>
#include <iterator>
// #include <unordered_map>

namespace atmt {

// void
// TMatrix::Init(const std::filesystem::path& file) {
// 	if (!std::filesystem::is_regular_file(file))
// 		throw std::runtime_error(file.string() + " isn't regular file");

// 	size_ = std::filesystem::file_size(file);
// 	if (!size_)
// 		throw std::runtime_error("file size for mmap equal zero");

// 	int fd = open(file.c_str(), O_RDONLY);
// 	if (fd == -1)
// 		throw std::runtime_error("opening file for mmap");

// 	addr_ = reinterpret_cast<std::uint32_t*>(mmap(NULL, size_, PROT_READ,
// 			MAP_PRIVATE, fd, 0));
// 	if (addr_ == MAP_FAILED)
// 		throw std::runtime_error("mmap failed");
// }

// void
// TMatrix::Delete() {
// 	munmap(addr_, size_);
// 	size_ = 0;
// 	addr_ = nullptr;
// }

// std::uint32_t
// TMatrix::Get(std::uint32_t elm, bool in) {
// 	return addr_[elm * 2 + in];
// }

// void
// EqClasses::Init(const std::filesystem::path& file) {
// 	if (!std::filesystem::is_regular_file(file))
// 		throw std::runtime_error(file.string() + " isn't regular file");

// 	size_ = std::filesystem::file_size(file);
// 	if (!size_)
// 		throw std::runtime_error("file size for mmap equal zero");

// 	int fd = open(file.c_str(), O_RDONLY);
// 	if (fd == -1)
// 		throw std::runtime_error("opening file for mmap");

// 	addr_ = reinterpret_cast<std::uint32_t*>(mmap(NULL, size_, PROT_READ,
// 			MAP_PRIVATE, fd, 0));
// 	if (addr_ == MAP_FAILED)
// 		throw std::runtime_error("mmap failed");
// }

// void
// EqClasses::Delete() {
// 	munmap(addr_, size_);
// 	size_ = 0;
// 	addr_ = nullptr;
// }

// std::uint32_t
// EqClasses::Get(std::uint32_t elm) {
// 	return addr_[elm];
// }

std::uint8_t
RsFunction::operator[](std::size_t offset) {
	return ((reinterpret_cast<std::uint8_t*>(addr_)[(offset >> 3)] >> (offset & 0b111)) & 0b1);
}

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
#ifndef WIN32
	std::filesystem::path path_y = dir.string() + "/func_y";
	std::filesystem::path path_g = dir.string() + "/func_g";
#else
	std::filesystem::path path_y = dir.string() + "\\func_y";
	std::filesystem::path path_g = dir.string() + "\\func_g";
#endif
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
	elm_ = ((elm_ << 1) | y_out) & mask_;
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

// void
// RsAutomat::PrintEquivalenceInfo(const std::filesystem::path& dir) {
// 	auto mtrx = CreateTMatrix(dir);
// 	auto classes = CreateEqClasses(dir);
// 	auto del = [&mtrx, &classes]() {
// 		mtrx->Delete();
// 		classes->Delete();
// 	};

// 	std::unordered_map<std::uint64_t, std::uint32_t> new_classes;
// 	std::uint32_t comp_degree = 0;
// 	try {
// 		do {
// 			++comp_degree;
			
// 		} while (!new_classes.empty());
// 		del();
// 	} catch (std::exception& e) {
// 		del();
// 		throw;
// 	}
// }

// std::shared_ptr<TMatrix>
// RsAutomat::CreateTMatrix(const std::filesystem::path& dir) {
// 	std::filesystem::path mtrx_path = dir.string() + "/tmatrix";
// 	std::ofstream mtrx(mtrx_path, std::ios::binary);
	
// 	for (Elm elm = 0; elm <= mask_; ++elm) {
// 		SetElm(elm);
// 		Next(0);
// 		mtrx.write(reinterpret_cast<const char*>(&elm_), sizeof(elm_));
// 		SetElm(elm);
// 		Next(1);
// 		mtrx.write(reinterpret_cast<const char*>(&elm_), sizeof(elm_));
// 	}

// 	mtrx.close();

// 	return std::make_shared<TMatrix>(mtrx_path);
// }

// std::shared_ptr<EqClasses>
// RsAutomat::CreateEqClasses(const std::filesystem::path& dir)  {
// 	std::filesystem::path classes_path = dir.string() + "/classes";
// 	std::ofstream classes(classes_path, std::ios::binary);
	
// 	for (Elm elm = 0; elm <= mask_; ++elm) {
// 		std::uint32_t out_class = 0;
// 		SetElm(elm);
// 		Next(0);
// 		out_class |= last_out_;
// 		SetElm(elm);
// 		Next(1);
// 		out_class |= last_out_ << 1;
// 		classes.write(
// 				reinterpret_cast<const char*>(&out_class), sizeof(out_class));
// 	}

// 	classes.close();

// 	return std::make_shared<EqClasses>(classes_path);
// }

}  // namespace atmt