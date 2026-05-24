#include "include/obj_store.hpp"
#include "include/constants.hpp"
#include "include/hashing.hpp"
#include "include/utils.hpp"

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

std::string obj_store::write_object(std::vector<char> & data)
{
	std::string hash = hashing::sha1(data);

	std::string dir_name = utils::hash_dir(hash);
	std::string file_name = utils::hash_file(hash);

	fs::path path =
		fs::path(constants::OBJECTS_DIR)
		/ dir_name
		/ file_name;

	fs::create_directories(
		fs::path(constants::OBJECTS_DIR)
		/ dir_name);

	utils::write_binary_file(path, data);

	return hash;
}

std::vector<char> obj_store::read_object (const std::string& hash)
{
	std::string dir_name = utils::hash_dir(hash);
	std::string file_name = utils::hash_file(hash);

	fs::path path =
		fs::path(constants::OBJECTS_DIR)
		/ dir_name
		/ file_name;

	return utils::read_binary_file(path);
}
