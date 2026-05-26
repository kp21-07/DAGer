#include "dagr.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cmd_help();
		return 0;
	}

	string command = argv[1];

	if (command == "init")
	{
		cmd_init();
	}
	else if (command == "help")
	{
		cmd_help();
	}
	else if (command == "hash-obj")
	{
		if (argc < 3)
		{
			printf("Usage: dagr hash-obj <file>\n");
			return 1;
		}

		string filename = argv[2];

		cmd_hash_obj(filename);
	}
	else if (command == "cat-obj")
	{
		if (argc < 3)
		{
			printf("Usage: dagr cat-obj <hash>\n");
			return 1;
		}

		string hash = argv[2];

		cmd_cat_obj(hash);
	}
	else
	{
		printf("Unknown command\n");
	}

	return 0;
}
