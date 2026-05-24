#include "include/utils.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace utils {

//
// TEXT FILE READ
//

std::string read_file(const fs::path& path)
{
	std::ifstream file(path);
	
	if (!file)
	{
		throw std::runtime_error(
			"Failed to open file"
		);
	}
	
	std::stringstream buffer;
	
	buffer << file.rdbuf();
	
	return buffer.str();
}

//
// BINARY FILE READ
//

std::vector<char> read_binary_file( const fs::path& path)
{
	std::ifstream file(
		path,
		std::ios::binary
	);
	
	if (!file)
	{
		throw std::runtime_error(
			"Failed to open binary file"
		);
	}
	
	return std::vector<char>(
		(std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>()
	);
}

//
// TEXT FILE WRITE
//

void write_file(
	const fs::path& path,
	const std::string& content
)
{
	std::ofstream out(path);
	
	if (!out)
	{
		throw std::runtime_error(
			"Failed to write file"
		);
	}
	
	out << content;
}

//
// BINARY FILE WRITE
//

void write_binary_file(
	const fs::path& path,
	const std::vector<char>& data
)
{
	std::ofstream out(
		path,
		std::ios::binary
	);
	
	if (!out)
	{
		throw std::runtime_error(
			"Failed to write binary file"
		);
	}
	
	out.write(
		data.data(),
		data.size()
	);
}

//
// HASH HELPERS
//

std::string hash_dir(const std::string& hash)
{
	return hash.substr(0, 2);
}

std::string hash_file(const std::string& hash)
{
	return hash.substr(2);
}

}
