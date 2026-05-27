#include "dagr.h"
#include "types.h"
#include <stdio.h>

void cmd_help()
{
  printf(
      R"(dagr - mini git implementation

Usage:
    dagr <command> [args]

Commands:
    init                Initialize repository

    hash-object <file>  Store file as object

    cat-file <hash>     Print object contents

    help                Show this help message
)");
}

void cmd_init() { repo_init(); }

void cmd_hash_obj(const string file_path)
{
  binary_buffer data = read_binary_file(file_path.data());

  string hash = write_object(data);

  printf("%s\n", hash.data());
}

void cmd_cat_obj(const string hash)
{
  binary_buffer data = read_object(hash);

  fwrite(data.data(), sizeof(char), data.size(), stdout);
}

void cmd_add(const vector<string>& files)
{
	for (size_t i = 0; i < files.size(); i++) {
		add_file(files[i]);
		printf("Added file %s\n", files[i].data());
	}
}

void cmd_status()
{
	vector<IndexEntry> entries = read_index();

	vector<string> modified;

	for (size_t i = 0; i < entries.size(); i++) {
		binary_buffer content = read_binary_file(entries[i].filename.data());
		string current_hash = sha1(content);
		if (!(current_hash == entries[i].hash.data())) {
			modified.push_back(entries[i].filename);
		}
	}

	if (modified.size() > 0) {
		printf("Modified Files:\n\n");
		for (size_t i = 0; i < modified.size(); i++) {
			printf("\t%s\n", modified[i].data());
		}
	}
	else {
		printf("All files upto date.\n");
	}
	printf("\n");
}
