#include "dagr.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

//
// TEXT FILE READ
//

string read_file(const char *path) {
  int fd = open(path, O_RDONLY);

  if (fd < 0) {
    fprintf(stderr, "Error: Failed to open file '%s'\n", path);
    exit(1);
  }

  struct stat st;
  string content;
  if (fstat(fd, &st) == 0 && st.st_size > 0) {
    content.reserve(st.st_size);
  }

  char buffer[4096];
  ssize_t bytes_read;
  while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
    content.append(buffer, bytes_read);
  }

  close(fd);

  if (bytes_read < 0) {
    fprintf(stderr, "Error: Failed to read file '%s'\n", path);
    exit(1);
  }

  return content;
}

//
// BINARY FILE READ
//

binary_buffer read_binary_file(const char *path) {
  int fd = open(path, O_RDONLY);

  if (fd < 0) {
    fprintf(stderr, "Error: Failed to open binary file '%s'\n", path);
    exit(1);
  }

  struct stat st;
  binary_buffer data;
  if (fstat(fd, &st) == 0 && st.st_size > 0) {
    data.reserve(st.st_size);
  }

  char buffer[4096];
  ssize_t bytes_read;
  while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
    data.append(buffer, bytes_read);
  }

  close(fd);

  if (bytes_read < 0) {
    fprintf(stderr, "Error: Failed to read binary file '%s'\n", path);
    exit(1);
  }

  return data;
}

//
// TEXT FILE WRITE
//

void write_file(const char *path, const string &content) {
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (fd < 0) {
    fprintf(stderr, "Error: Failed to write file '%s'\n", path);
    exit(1);
  }

  if (!content.is_empty()) {
    const char *ptr = content.data();
    size_t remaining = content.size();
    while (remaining > 0) {
      ssize_t bytes_written = write(fd, ptr, remaining);
      if (bytes_written < 0) {
        close(fd);
        fprintf(stderr, "Error: Failed to write all content to file '%s'\n", path);
        exit(1);
      }
      ptr += bytes_written;
      remaining -= bytes_written;
    }
  }

  close(fd);
}

//
// BINARY FILE WRITE
//

void write_binary_file(const char *path, const binary_buffer &data) {
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (fd < 0) {
    fprintf(stderr, "Error: Failed to write binary file '%s'\n", path);
    exit(1);
  }

  if (!data.empty()) {
    const char *ptr = data.data();
    size_t remaining = data.size();
    while (remaining > 0) {
      ssize_t bytes_written = write(fd, ptr, remaining);
      if (bytes_written < 0) {
        close(fd);
        fprintf(stderr, "Error: Failed to write all data to binary file '%s'\n", path);
        exit(1);
      }
      ptr += bytes_written;
      remaining -= bytes_written;
    }
  }

  close(fd);
}

//
// HASH HELPERS
//

string hash_dir(const string &hash) {
  if (hash.length() < 2) {
    return hash;
  }
  return string(hash.data(), 2);
}

string hash_file(const string &hash) {
  if (hash.length() < 2) {
    return "";
  }
  return string(hash.data() + 2);
}
