#ifndef DAGR_H
#define DAGR_H

#include "types.h"

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
void cmd_cat_obj (const string hash);
void cmd_add     (const vector<string>& files);
void cmd_status();


//
// hashing.cpp
//

string sha1(binary_buffer& data);


//
// obj_store.cpp
//

string        write_object(binary_buffer& data);
binary_buffer read_object (const string& hash);


//
// repo.cpp
//

void repo_init();


//
// index.cpp
//

struct IndexEntry {
	string filename;
	string hash;
};

vector<IndexEntry> read_index();
void               write_index(const vector<IndexEntry>& entries);

void               add_file(const string& filename);


//
// utils.cpp
//

string read_file (const char* path);
void   write_file(const char* path, const string& content, bool append = false);

binary_buffer read_binary_file (const char* path);
void          write_binary_file(const char* path, const binary_buffer& data);

string hash_dir (const string& hash);
string hash_file(const string& hash);

#endif /* DAGR_H */
