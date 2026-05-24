#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace utils {

//
// FILE I/O
//

std::string read_file(const fs::path& path);

std::vector<char> read_binary_file(const fs::path& path);

void write_file(
	const fs::path& path,
	const std::string& content
);

void write_binary_file(
	const fs::path& path,
	const std::vector<char>& data
);

//
// HASH HELPERS
//

std::string hash_dir(const std::string& hash);

std::string hash_file(const std::string& hash);

}
