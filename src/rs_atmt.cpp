
#include "rs_atmt.h"

#include <fcntl.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <sys/mman.h>
// #include <sys/stat.h>
// #include <sys/types.h>
// #include <unistd.h>

#include <iostream>
#include <iterator>

namespace atmt {

void
RsFunction::Init(const std::filesystem::path& file) {
	if (!std::filesystem::is_regular_file(file))
		throw std::runtime_error(file.string() + " isn't regular file");

	size_ = std::filesystem::file_size(file);
	size_b_ = size_ << 3;
	if (!size_)
		throw std::runtime_error("file size for mmap equal zero");

	int fd = open(file.c_str(), O_RDONLY);
	if (fd == -1)
		throw std::runtime_error("opening file for mmap");

	addr_ = reinterpret_cast<std::uint8_t*>(mmap(NULL, size_, PROT_READ,
			MAP_PRIVATE, fd, 0));
	if (addr_ == MAP_FAILED)
		throw std::runtime_error("mmap failed");
}

void
RsFunction::Delete() {
	munmap(addr_, size_);
	size_ = 0;
	size_b_ = 0;
	addr_ = nullptr;
}

std::uint8_t
RsFunction::operator[](std::size_t offset) {
	return ((addr_[(offset >> 3)] >> (offset & 0b111)) & 0b1);
}

std::shared_ptr<RsData>
RsConfigParser::Parse(const std::filesystem::path& dir) {
	std::cout << "Creating shift register\n";
	std::shared_ptr<RsData> data = std::make_shared<RsData>();
	in_ >> data->n;
	std::cout << "n: " << data->n << "\n";
	in_.get();
	std::istream_iterator<bool> values(in_);
	std::size_t offset = 0;
	std::size_t len = (1 << data->n) * 2;
	std::filesystem::path path_y = dir.string() + "/func_y";
	std::filesystem::path path_g = dir.string() + "/func_g";
	std::ofstream func_y(path_y);
	std::ofstream func_g(path_g);
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
	data->y.Init(path_y);
	data->g.Init(path_g);
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

}  // namespace atmt