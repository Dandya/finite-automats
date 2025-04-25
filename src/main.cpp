#include <iostream>
#include <iterator>

#include "rs_atmt.h"

std::uint32_t InputStartElmRs(std::size_t len) {
	std::cout << "Input start value with len (" << len << "): ";
	// std::istream_iterator<bool> input(std::cin);
	std::uint32_t res = 0;
	for (std::size_t i = 0; i < len; ++i) {
		std::uint8_t digit = std::cin.get();
		if (digit != '0' && digit != '1')
			throw std::runtime_error("incorrect digit: " +
					std::to_string(static_cast<std::uint32_t>(digit)));
		res |= static_cast<std::uint32_t>(digit - '0') << (len - 1 - i);
	}
	return res;
}

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cout << "Usage: " << argv[0] << " CONF_RS DIR_FOR_FILES\n";
		return -1;
	}
	atmt::RsAutomat rs_atmt;
	auto clean = [&rs_atmt]() {
		rs_atmt.Delete();
	};
	try {
		atmt::RsConfigParser rs_parser(argv[1]);
		auto data = rs_parser.Parse(argv[2]);
		rs_atmt.Init(data, InputStartElmRs(data->n));
		std::int8_t in = 0;
		rs_atmt.PrintElm();
		while (true) {
			std::cout  << "Input x: ";
			while ((in = std::cin.get()) == '\n')
				continue;
			if (in == 'q')
				break;
			if (in != '0' && in != '1')
				throw std::runtime_error("incorrect digit: " +
						std::to_string(static_cast<std::uint32_t>(in)));
			rs_atmt.Next(in - '0');
			rs_atmt.PrintElm();
			rs_atmt.PrintOut();
		}
		clean();
	} catch(std::exception& e) {
		std::cout << "Error: " << e.what() << "\n";
		clean();
		return 1; 
	}
	return 0;
}