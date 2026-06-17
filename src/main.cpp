#include "dagr.h"

#include <stdio.h>

// Commands that need an existing repo
static bool is_dagr_repo()
{
	if (!repo_exists()) {
		fprintf(stderr, "fatal: not a dagr repository: .dagr\n"
		                "Run 'dagr init' to create one.\n");
		return false;
	}
	return true;
}

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
		cmd_hash_obj(string(argv[2]));
	}
	else if (command == "cat-obj")
	{
		if (argc < 3)
		{
			printf("Usage: dagr cat-obj <hash>\n");
			return 1;
		}
		cmd_cat_obj(string(argv[2]));
	}
	else if (command == "add")
	{
		if (!is_dagr_repo()) return 1;

		if (argc < 3)
		{
			printf("Usage: dagr add <file...> | .\n");
			return 1;
		}

		vector<string> files;

		// "dagr add ." — stage all files in the current directory
		if (argc == 3 && string(argv[2]) == ".")
		{
			scan_cwd(files);
			if (files.size() == 0)
			{
				printf("Nothing to add.\n");
				return 0;
			}
		}
		else
		{
			for (int i = 2; i < argc; i++)
				files.push_back(string(argv[i]));
		}

		cmd_add(files);
	}
	else if (command == "status")
	{
		if (!is_dagr_repo()) return 1;
		cmd_status();
	}
	else if (command == "write-tree")
	{
		if (!is_dagr_repo()) return 1;
		cmd_write_tree();
	}
	else if (command == "commit")
	{
		if (!is_dagr_repo()) return 1;

		if (argc < 4 || string(argv[2]) != "-m")
		{
			printf("Usage: dagr commit -m <message>\n");
			return 1;
		}

		// Join all remaining tokens after -m to support unquoted multi-word messages
		string message = string(argv[3]);
		for (int i = 4; i < argc; i++)
			message = message + " " + string(argv[i]);

		cmd_commit(message);
	}
	else if (command == "log")
	{
		if (!is_dagr_repo()) return 1;
		cmd_log();
	}
	else if (command == "diff")
	{
		if (!is_dagr_repo()) return 1;
		cmd_diff();
	}
	else
	{
		printf("Unknown command: %s\n", command.data());
	}

	return 0;
}

