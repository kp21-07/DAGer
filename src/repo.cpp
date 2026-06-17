#include "dagr.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

// Creates the .dagr directory structure (objects/, refs/, HEAD, index) for a new repository
void repo_init()
{
	if (access(".dagr", F_OK) == 0)
	{
		printf("Repo already initialized.\n");
		return;
	}

	mkdir(DAGR, 0755);
	mkdir(OBJECTS_DIR, 0755);
	mkdir(REFS_DIR, 0755);

	write_file(HEAD_FILE, "ref: refs/main\n");

	write_file(INDEX_FILE, "");

	printf("Initialized empty repo.\n");
}

// Returns true if a .dagr repository exists in the current directory
bool repo_exists()
{
	return access(DAGR, F_OK) == 0;
}
