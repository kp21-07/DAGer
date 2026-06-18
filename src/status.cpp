#include "dagr.h"

// Simple string sorting helper for clean status output
static void sort_strings(vector<string>& v) {
  if (v.size() < 2) return;
  for (size_t i = 0; i < v.size() - 1; i++) {
    for (size_t j = 0; j < v.size() - i - 1; j++) {
      if (strcmp(v[j].data(), v[j + 1].data()) > 0) {
        string temp = v[j];
        v[j] = v[j + 1];
        v[j + 1] = temp;
      }
    }
  }
}

static void scan_current_directory(vector<string>& all_files) {
  scan_cwd(all_files);
}

void run_status() {
  vector<IndexEntry> entries = read_index();

  vector<string> all_files;
  scan_current_directory(all_files);

  vector<string> modified;
  vector<string> deleted;
  vector<string> untracked;

  for (size_t i = 0; i < entries.size(); i++) {
    string filename = entries[i].filename;
    if (access(filename.data(), F_OK) != 0) {
      deleted.push_back(filename);
    } else {
      binary_buffer content = read_binary_file(filename.data());
      string current_hash = sha1(content);
      if (current_hash != entries[i].hash) {
        modified.push_back(filename);
      }
    }
  }

  for (size_t i = 0; i < all_files.size(); i++) {
    string filename = all_files[i];
    // Ignore internal dagr binary and dotfiles
    if (filename == "dagr" || filename == ".dagr" || filename == ".git") {
      continue;
    }
    bool tracked = false;
    for (size_t j = 0; j < entries.size(); j++) {
      if (entries[j].filename == filename) {
        tracked = true;
        break;
      }
    }
    if (!tracked) {
      untracked.push_back(filename);
    }
  }

  sort_strings(modified);
  sort_strings(deleted);
  sort_strings(untracked);

  bool has_changes = false;

  if (modified.size() > 0) {
    has_changes = true;
    printf("Modified Files:\n\n");
    for (size_t i = 0; i < modified.size(); i++) {
      printf("\t\033[31m%s\033[m\n", modified[i].data());
    }
    printf("\n");
  }

  if (deleted.size() > 0) {
    has_changes = true;
    printf("Deleted Files:\n\n");
    for (size_t i = 0; i < deleted.size(); i++) {
      printf("\t\033[31m%s\033[m\n", deleted[i].data());
    }
    printf("\n");
  }

  if (untracked.size() > 0) {
    has_changes = true;
    printf("Untracked Files:\n\n");
    for (size_t i = 0; i < untracked.size(); i++) {
      printf("\t\033[31m%s\033[m\n", untracked[i].data());
    }
    printf("\n");
  }

  if (!has_changes) {
    printf("\033[32mAll files upto date.\033[m\n\n");
  }
}
