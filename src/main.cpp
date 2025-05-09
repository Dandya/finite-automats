

#include <signal.h>

#include <iostream>
#include <iterator>
#include <string>

#include "rs_atmt.h"
#include "lin_atmt.h"

struct RunData {
	std::unique_ptr<atmt::RsAutomat> rs;
	std::unique_ptr<atmt::LinAutomat> lin;
} run_data;

void
DeleteRunData() {
	if (run_data.rs) {
		run_data.rs->Delete();
		run_data.rs.reset();
	}
	if (run_data.lin) {
		run_data.lin->Delete();
		run_data.lin.reset();
	}
}

void
SigHandler(int arg) {
	DeleteRunData();
	exit(0);
}

void
InitialSigHandler() {
#ifndef WIN32
	struct sigaction sigaction_info {};
	sigaction_info.sa_handler = SigHandler;
	sigaction_info.sa_flags = SA_RESTART;
	sigemptyset(&sigaction_info.sa_mask);
	sigaction(SIGINT, &sigaction_info, NULL);
	sigaction(SIGTERM, &sigaction_info, NULL);
	sigaction(SIGHUP, &sigaction_info, NULL);
#else
	signal(SIGINT, SigHandler);
#endif
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
	std::istream_iterator<std::uint32_t> in(std::cin);
	atmt::Matrix mtx(1, len);
	for (std::size_t i = 0; i < len; ++i) {
		mtx.data()[i] = *in % q;
		if (i != len - 1)
			in++;
		if (std::cin.fail())
			throw std::runtime_error("bad input");
	}
	return mtx;
}

int
main(int argc, char** argv) {
	using namespace std::string_literals;
#ifndef WIN32
	InitialSigHandler();
#endif

	if (argc < 5) {
		std::cout << "Usage: " << argv[0] << " TYPE CONF DIR_FOR_FILES MODE\n";
		return -1;
	}

	run_data.rs = std::make_unique<atmt::RsAutomat>();
	run_data.lin = std::make_unique<atmt::LinAutomat>();
	try {
		int mode = std::stoi(argv[4]);
		if (mode < 1 || mode > 2)
			throw std::logic_error("unknown mode");
		if (argv[1] == "rs"s) {
			atmt::RsConfigParser parser(argv[2]);
			auto data = parser.Parse(argv[3]);
			if (mode == 1) {
				std::cout << "Input start value with len (" << data->n << "): ";
				run_data.rs->Init(data, InputElmRs(data->n));
				run_data.rs->PrintElm();
				while (true) {
					std::cout  << "Input x: ";
					run_data.rs->Next(InputElmRs(1));
					run_data.rs->PrintElm();
					run_data.rs->PrintOut();
				}
			} else if (mode == 2) {
				run_data.rs->Init(data, 0);
				run_data.rs->PrintEquivalenceInfo(argv[3]);
			}
		} else if (argv[1] == "lin"s) {
			atmt::LinConfigParser parser(argv[2]);
			auto data = parser.Parse();
			std::cout << "Input start value with len (" << data->A.rows() << "): ";
			run_data.lin->Init(data,
				atmt::Matrix(InputElmLin(data->A.rows(), data->q)));
			run_data.lin->PrintElm();
			std::size_t len_x = data->B.rows();
			std::size_t q = data->q;
			while (true) {
				std::cout  << "Input x: ";
				run_data.lin->Next(InputElmLin(len_x, q));
				run_data.lin->PrintElm();
				run_data.lin->PrintOut();
			}
		} else {
			throw std::runtime_error("unknow automat type");
		}
	} catch(std::exception& e) {
		std::cout << "Error: " << e.what() << "\n";
		DeleteRunData();
		return 1;
	}
	DeleteRunData();
	return 0;
}