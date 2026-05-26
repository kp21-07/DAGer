#ifndef DAGR_H
#define DAGR_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

//
// Data Structures
//

struct string {
  char *m_data;
  size_t m_size;
  size_t m_capacity;

  string() : m_data(nullptr), m_size(0), m_capacity(0) {}

  string(const char *str) {
    m_size = str ? strlen(str) : 0;
    m_capacity = m_size;
    m_data = (char *)malloc(m_size + 1);
    if (str) {
      memcpy(m_data, str, m_size + 1);
    } else {
      if (m_data)
        m_data[0] = '\0';
    }
  }

  string(const char *str, size_t len) {
    m_size = len;
    m_capacity = len;
    m_data = (char *)malloc(len + 1);
    if (str && len > 0) {
      memcpy(m_data, str, len);
    }
    if (m_data)
      m_data[len] = '\0';
  }

  const char *data() const { return m_data; }

  size_t length() const { return m_size; }

  size_t size() const { return m_size; }

  bool is_empty() const { return m_size == 0; }

  void reserve(size_t new_cap) {
    if (new_cap <= m_capacity)
      return;
    char *new_data = (char *)realloc(m_data, new_cap + 1);
    if (new_data) {
      m_data = new_data;
      m_capacity = new_cap;
    }
  }

  void append(const char *str, size_t len) {
    if (!str || len == 0)
      return;
    if (m_size + len > m_capacity) {
      reserve((m_size + len) * 2);
    }
    memcpy(m_data + m_size, str, len);
    m_size += len;
    m_data[m_size] = '\0';
  }

  bool operator==(const char *str) const {
    return strcmp(data(), str ? str : "") == 0;
  }
};

struct binary_buffer {
    char* m_data;
    size_t m_size;
    size_t m_capacity;

    binary_buffer() : m_data(nullptr), m_size(0), m_capacity(0) {}

    binary_buffer(size_t cap) : m_data(nullptr), m_size(0), m_capacity(cap) {
        if (cap > 0) {
            m_data = (char*)malloc(cap);
        }
    }

    const char* data() const { return m_data; }
    char* data() { return m_data; }
    size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }

    void reserve(size_t new_cap) {
        if (new_cap <= m_capacity) return;
        char* new_data = (char*)realloc(m_data, new_cap);
        if (new_data) {
            m_data = new_data;
            m_capacity = new_cap;
        }
    }

    void append(const char* data, size_t len) {
        if (!data || len == 0) return;
        if (m_size + len > m_capacity) {
            reserve((m_size + len) * 2);
        }
        memcpy(m_data + m_size, data, len);
        m_size += len;
    }
};


//
// Constants
//

#define DAGR           ".dagr"
#define OBJECTS_DIR    ".dagr/objects"
#define REFS_DIR       ".dagr/refs"
#define HEAD_FILE      ".dagr/HEAD"
#define INDEX_FILE     ".dagr/index"
#define DEFAULT_BRANCH "main"


//
// commands.cpp
//

void cmd_help();
void cmd_init();
void cmd_hash_obj(const string content);
void cmd_cat_obj(const string hash);


//
// hashing.cpp
//

string sha1(binary_buffer& data);


//
// obj_store.cpp
//

string write_object(binary_buffer& data);
binary_buffer read_object (const string& hash);


//
// repo.cpp
//

void repo_init();
    

//
// utils.cpp
//

string read_file(const char* path);

binary_buffer read_binary_file(const char* path);

void write_file(
	const char* path,
	const string& content
);

void write_binary_file(
	const char* path,
	const binary_buffer& data
);

string hash_dir(const string& hash);

string hash_file(const string& hash);

#endif /* DAGR_H */
