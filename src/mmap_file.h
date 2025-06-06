
#pragma once

#include <tuple>
#include <filesystem>

#ifdef WIN32
#include <windows.h>
#endif // WIN32

namespace atmt {

enum MmapMode {
	READ = 0,
	READ_WRITE = 1
};

struct MmapFile {
	std::size_t size_ = 0;
	void* addr_ = nullptr;
#ifndef WIN32
	int fd_ = -1;
#else
	HANDLE fd_ = INVALID_HANDLE_VALUE;
	HANDLE fm_ = INVALID_HANDLE_VALUE;
#endif // !WIN32

	MmapFile(const std::filesystem::path& path, MmapMode mode);

	~MmapFile();
};

}  // namespace atmt
