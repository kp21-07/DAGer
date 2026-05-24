#include "include/commands.hpp"

#include <iostream>
#include <string>

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cmd::help();
		return 0;
	}

	std::string command = argv[1];

	try
	{
		if (command == "init")
		{
			cmd::init();
		}
		else if (command == "help")
		{
			cmd::help();
		}
		else if (command == "hash-obj")
		{
			if (argc < 3)
			{
				std::cout << "Usage: dagr hash-obj <file>\n";
				return 1;
			}

			std::string filename = argv[2];

			cmd::hash_obj(filename);
		}
		else if (command == "cat-obj")
		{
			if (argc < 3)
			{
				std::cout << "Usage: dagr cat-obj <help>\n";
				return 1;
			}

			std::string hash = argv[2];

			cmd::cat_obj(hash);
		}
		else
		{
			std::cout << "Unknown command\n";
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return 1;
	}

	return 0;
}
