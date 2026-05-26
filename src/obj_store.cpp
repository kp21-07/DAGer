#include "dagr.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

string write_object(binary_buffer &data) {
  string hash = sha1(data);

  string dir_name = hash_dir(hash);
  string file_name = hash_file(hash);

  char dir_path[512];
  sprintf(dir_path, "%s/%s", OBJECTS_DIR, dir_name.data());

  char file_path[512];
  sprintf(file_path, "%s/%s", dir_path, file_name.data());

  mkdir(dir_path, 0755);

  write_binary_file(file_path, data);

  return hash;
}

binary_buffer read_object(const string &hash) {
  string dir_name = hash_dir(hash);
  string file_name = hash_file(hash);

  char file_path[512];
  sprintf(file_path, "%s/%s/%s", OBJECTS_DIR, dir_name.data(),
          file_name.data());

  return read_binary_file(file_path);
}
