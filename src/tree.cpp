#include "dagr.h"

// Converts a list of TreeEntries into a flat text blob: "<type> <name> <hash>\n" per entry
binary_buffer serialize_tree(vector<TreeEntry> entries)
{
	string data;
	for (auto entry : entries) {
		data = data 
					 + ((entry.type == 0) ? "blob" : "tree") + " "
					 + entry.name + " "
					 + entry.hash + "\n";
	}

	return binary_buffer(data);
}

// Builds a tree object from the current index (flat, blobs only) and writes it to the object store
string write_tree()
{
	vector<IndexEntry> idx_entries = read_index();

	vector<TreeEntry> tree_entries;

	for (auto entry : idx_entries) {
		TreeEntry new_entry = {
			0,
			entry.filename,
			entry.hash,
		};

		tree_entries.push_back(new_entry);
	}

	binary_buffer tree_data = serialize_tree(tree_entries);
	return write_object(tree_data);
}
