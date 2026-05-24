#include "include/repo.hpp"
#include "include/constants.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void repo::init()
{
	if (fs::exists(".dagr"))
	{
		std::cout << "Repo already initialized.\n";
		return;
	}

	fs::create_directory(constants::DAGR);
	fs::create_directory(constants::OBJECTS_DIR);
	fs::create_directory(constants::REFS_DIR);

	std::ofstream head(constants::HEAD_FILE);
	head << "ref: refs/main\n";

	std::ofstream index(constants::INDEX_FILE);

	std::cout << "Initialized empty repo.\n";
}
