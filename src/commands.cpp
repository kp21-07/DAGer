#include "include/commands.hpp"
#include "include/repo.hpp"
#include "include/obj_store.hpp"
#include "include/utils.hpp"

#include <iostream>

void cmd::help()
{
	std::cout <<
R"(dagr - mini git implementation

Usage:
    dagr <command> [args]

Commands:
    init                Initialize repository

    hash-object <file>  Store file as object

    cat-file <hash>     Print object contents

    help                Show this help message
)";
}

void cmd::init()
{
	repo::init();
}

void cmd::hash_obj(const std::string file_path)
{
	std::vector<char> data = utils::read_binary_file(file_path);

	std::string hash = obj_store::write_object(data);

	std::cout << hash << std::endl;
}

void cmd::cat_obj(const std::string hash)
{
	std::vector<char> data = obj_store::read_object(hash);

	std::cout.write(
			data.data(),
			data.size()
	);
}
