#include "dagr.h"
#include <stdio.h>

void cmd_help() {
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

void cmd_hash_obj(const string file_path) {
  binary_buffer data = read_binary_file(file_path.data());

  string hash = write_object(data);

  printf("%s\n", hash.data());
}

void cmd_cat_obj(const string hash) {
  binary_buffer data = read_object(hash);

  fwrite(data.data(), sizeof(char), data.size(), stdout);
}
