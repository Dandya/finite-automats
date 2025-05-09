
#include "mmap_file.h"

namespace atmt {

MmapFile::MmapFile(const std::filesystem::path& path, MmapMode mode) {
	if (!std::filesystem::is_regular_file(path))
		throw std::runtime_error(path.string() + " isn't regular file");
	
	size_ = std::filesystem::file_size(path);
	if (!size_)
		throw std::runtime_error("file size for mmap equal zero");

#ifndef WIN32
	fd_ = open(file.c_str(), (mode = MmapMode::READ ? O_RDONLY : O_RDWR));
	if (fd_ == -1)
		throw std::runtime_error("opening file for mmap");

	addr_ = reinterpret_cast<std::uint8_t*>(mmap(NULL, size_, 
		PROT_READ | (mode = MmapMode::READ_WRITE ? PROT_WRITE : 0),
		MAP_PRIVATE, fd_, 0));
	if (addr_ == MAP_FAILED)
		throw std::runtime_error("mmap failed");
#else
	fd_ = INVALID_HANDLE_VALUE;
	fm_ = INVALID_HANDLE_VALUE;
	auto close = [](HANDLE& h) {
		if (h == INVALID_HANDLE_VALUE)
			return;			
		CloseHandle(h);
		h = INVALID_HANDLE_VALUE;
	};

	fd_ = CreateFileA(path.string().c_str(),
		GENERIC_READ | ((mode == MmapMode::READ_WRITE) ? GENERIC_WRITE : 0),
		0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fd_ == INVALID_HANDLE_VALUE) {
		LPTSTR errorText = NULL;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), 0x0409,
			(LPTSTR)&errorText, 0, NULL);
		std::string err = "opening file for mmap";
		if (errorText != NULL) {
			err = err + ": " + errorText;
			LocalFree(errorText);
			errorText = NULL;
		}
		throw std::runtime_error(err);
	}

	fm_ = CreateFileMapping(fd_, NULL, ((mode == MmapMode::READ_WRITE) ? PAGE_READWRITE : PAGE_READONLY), 0, 0, NULL);
	if (fm_ == 0) {
		close(fd_);
		throw std::runtime_error("create file mapping");
	}

	addr_ = MapViewOfFile(fm_, ((mode == MmapMode::READ_WRITE) ?
			FILE_MAP_READ | FILE_MAP_WRITE : FILE_MAP_READ), 0, 0, 0);
	if (addr_ == nullptr) {
		close(fm_);
		close(fd_);
		throw std::runtime_error("mmap error");
	}
#endif // WIN32
}

MmapFile::~MmapFile() {
#ifndef WIN32
	munmap(addr_, size);
	close(fd_);
#else
	UnmapViewOfFile(addr_);
	CloseHandle(fm_);
	CloseHandle(fd_);
#endif // WIN32
}

}  // namespace atmt 