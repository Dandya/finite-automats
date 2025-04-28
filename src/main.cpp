

#include <signal.h>

#include <iostream>
#include <iterator>

#include "rs_atmt.h"
#include "lin_atmt.h"

struct RunData {
	atmt::RsAutomat* rs;
} run_data;

void
SigHandler(int arg) {
	if (run_data.rs)
		run_data.rs->Delete();
	exit(0);
}

void
InitialSigHandler() {
	struct sigaction sigaction_info;
	sigaction_info.sa_handler = SigHandler;
	sigaction_info.sa_flags = SA_RESTART;
	sigemptyset(&sigaction_info.sa_mask);
	sigaction(SIGINT, &sigaction_info, NULL);
	sigaction(SIGTERM, &sigaction_info, NULL);
	sigaction(SIGHUP, &sigaction_info, NULL);
}

std::uint32_t
InputElmRs(std::size_t len) {
	std::uint32_t res = 0;
	std::istream_iterator<std::uint32_t> in(std::cin);
	for (std::size_t i = 0; i < len; ++i) {
		res |= static_cast<bool>(*in) << (len - 1 - i);
		if (i != len - 1)
			in++;
		if (std::cin.fail())
			throw std::runtime_error("bad input");
	}
	return res;
}

atmt::Matrix
InputElmLin(std::size_t len, std::uint32_t q) {
	std::vector<std::uint32_t> res;
	std::istream_iterator<std::uint32_t> in(std::cin);
	res.resize(len);
	for (std::size_t i = 0; i < len; ++i) {
		res[i] = *in % q;
		if (i != len - 1)
			in++;
		if (std::cin.fail())
			throw std::runtime_error("bad input");
	}
	return atmt::Matrix(std::move(res), 1, len, q);
}

int
main(int argc, char** argv) {
	using namespace std::string_literals;

	InitialSigHandler();

	if (argc < 4) {
		std::cout << "Usage: " << argv[0] << " TYPE CONF DIR_FOR_FILES\n";
		return -1;
	}
	atmt::RsAutomat rs_atmt;
	atmt::LinAutomat lin_atmt;
	auto clean = [&rs_atmt, &lin_atmt]() {
		rs_atmt.Delete();
		lin_atmt.Delete();
	};
	try {
		if (argv[1] == "rs"s) {
			atmt::RsConfigParser parser(argv[2]);
			auto data = parser.Parse(argv[3]);
			std::cout << "Input start value with len (" << data->n << "): ";
			rs_atmt.Init(data, InputElmRs(data->n));
			rs_atmt.PrintElm();
			while (true) {
				std::cout  << "Input x: ";
				rs_atmt.Next(InputElmRs(1));
				rs_atmt.PrintElm();
				rs_atmt.PrintOut();
			}
			clean();
		} else if (argv[1] == "lin"s) {
			atmt::LinConfigParser parser(argv[2]);
			auto data = parser.Parse();
			std::cout << "Input start value with len (" << data->A.Row() << "): ";
			lin_atmt.Init(data,
					atmt::Matrix(InputElmLin(data->A.Row(), data->q)));
			lin_atmt.PrintElm();
			std::size_t len_x = data->B.Row();
			std::size_t q = data->q;
			while (true) {
				std::cout  << "Input x: ";
				lin_atmt.Next(InputElmLin(len_x, q));
				lin_atmt.PrintElm();
				lin_atmt.PrintOut();
			}
			clean();
		} else {
			throw std::runtime_error("unknow automat type");
		}
	} catch(std::exception& e) {
		std::cout << "Error: " << e.what() << "\n";
		clean();
		return 1;
	}
	return 0;
}