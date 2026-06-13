#include "dagr.h"
#include <stdio.h>
#include <unistd.h>

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

    add <file...>       Stage files

    write-tree          Write current index as tree object

    commit -m <msg>     Record a commit

    status              Show working tree status

    log                 Show commit history

    diff                Show changes not yet staged

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

void cmd_add(const vector<string> &files)
{
  for (size_t i = 0; i < files.size(); i++) {
    add_file(files[i]);
    printf("Added file %s\n", files[i].data());
  }
}

void cmd_status() { run_status(); }

void cmd_write_tree()
{
	string hash = write_tree();
	printf("%s\n", hash.data());
}

// Creates a commit object from the current index and prints its hash
void cmd_commit(const string& message)
{
	// Write a candidate tree from the current index
	string new_tree = write_tree();

	// Compare against the tree hash in the last commit — if identical, nothing changed
	string ref_path = string(DAGR) + "/refs/main";
	if (access(ref_path.data(), F_OK) == 0) {
		string parent_hash = read_file(ref_path.data());
		// trim newline
		size_t len = parent_hash.length();
		while (len > 0 && (parent_hash.data()[len-1] == '\n' || parent_hash.data()[len-1] == '\r')) len--;
		string parent_hash_clean(parent_hash.data(), len);

		binary_buffer obj = read_object(parent_hash_clean);
		string obj_str(obj.data(), obj.size());
		vector<string> lines = obj_str.split('\n');
		if (lines.size() > 0) {
			string first = lines[0];
			if (strncmp(first.data(), "tree ", 5) == 0) {
				string last_tree(first.data() + 5, first.length() - 5);
				if (last_tree == new_tree) {
					printf("Nothing to commit — working tree is clean.\n");
					return;
				}
			}
		}
	} else if (read_index().size() == 0) {
		// No prior commits: guard against completely empty index
		printf("Nothing to commit\n");
		return;
	}

	string hash = create_commit(message);
	printf("[%s] %s\n", hash.data(), message.data());
}

void cmd_log() { run_log(); }

// Delegates to run_diff() to compare working tree against the index
void cmd_diff() { run_diff(); }
