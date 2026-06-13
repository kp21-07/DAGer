#include "dagr.h"

// Reads .dagr/index and returns all staged entries as a vector of {filename, hash}
vector<IndexEntry> read_index()
{
	string contents = read_file(INDEX_FILE);

	vector<string> lines = contents.split('\n');

	vector<IndexEntry> entries;
	for (auto line : lines) {
		if (line.is_empty()) continue;
		vector<string> content = line.split(' ');
		if (content.size() >= 2) {
			entries.push_back({content[0], content[1]});
		}
	}

	return entries;
}

// Serializes entries to .dagr/index in the format "<filename> <hash>\n" per line
void write_index(const vector<IndexEntry>& entries)
{
	string index_content;

	size_t total_length = 0;
	for (const auto& entry : entries) {
			total_length += entry.filename.length() + 1 + entry.hash.length() + 1; // space + newline
	}
	
	index_content.reserve(total_length);

	for (const auto& entry : entries) {
			if (!entry.filename.is_empty() && !entry.hash.is_empty()) {
					index_content.append(entry.filename.data(), entry.filename.length());
					index_content.append(" ", 1);
					index_content.append(entry.hash.data(), entry.hash.length());
					index_content.append("\n", 1);
			}
	}

	write_file(INDEX_FILE, index_content);
}

// Hashes a file, writes it as an object, and upserts its entry in the index
void add_file(const string& filename)
{
	binary_buffer data = read_binary_file(filename.data());
	string hash = write_object(data);

	vector<IndexEntry> entries = read_index();

	bool found = false;
	for (size_t i = 0; i < entries.size(); i++) {
		if (entries[i].filename == filename.data()) {
			entries[i].hash = hash;
			found = true;
			break;
		}
	}

	if (!found) {
		entries.push_back({filename, hash});
	}

	write_index(entries);
}
