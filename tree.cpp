#include "dagr.h"
#include "types.h"

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
