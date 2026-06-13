#include "dagr.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// Resolve HEAD -> branch ref -> current commit hash
static string get_head_commit_hash()
{
	if (access(HEAD_FILE, F_OK) != 0) return "";

	string head = read_file(HEAD_FILE);
	if (strncmp(head.data(), "ref: ", 5) != 0) return "";

	size_t start = 5;
	size_t end = head.length();
	while (end > start && (head.data()[end - 1] == '\n' || head.data()[end - 1] == '\r'))
		end--;

	string ref_rel(head.data() + start, end - start);
	string ref_path = string(DAGR) + "/" + ref_rel;

	if (access(ref_path.data(), F_OK) != 0) return "";

	string hash = read_file(ref_path.data());
	size_t len = hash.length();
	while (len > 0 && (hash.data()[len - 1] == '\n' || hash.data()[len - 1] == '\r'))
		len--;

	return string(hash.data(), len);
}

// Parse a raw commit object back into a CommitObject
static CommitObject parse_commit(const string& hash)
{
	binary_buffer data = read_object(hash);
	string content(data.data(), data.size());
	vector<string> lines = content.split('\n');

	CommitObject commit;
	bool in_body = false;

	for (size_t i = 0; i < lines.size(); i++) {
		string line = lines[i];

		if (in_body) {
			if (!line.is_empty()) {
				if (!commit.message.is_empty())
					commit.message = commit.message + "\n";
				commit.message = commit.message + line;
			}
		} else if (line.is_empty()) {
			in_body = true;
		} else if (strncmp(line.data(), "tree ", 5) == 0) {
			commit.tree_hash = string(line.data() + 5, line.length() - 5);
		} else if (strncmp(line.data(), "parent ", 7) == 0) {
			commit.parent_hash = string(line.data() + 7, line.length() - 7);
		} else if (strncmp(line.data(), "author ", 7) == 0) {
			// Timestamp is the last space-separated token on the author line
			const char* p = line.data();
			size_t len = line.length();
			size_t last_space = 7;
			for (size_t j = 7; j < len; j++) {
				if (p[j] == ' ') last_space = j;
			}
			commit.timestamp = string(p + last_space + 1, len - last_space - 1);
		}
	}

	return commit;
}

// Print a single commit entry
static void print_commit(const string& hash, const CommitObject& commit)
{
	printf("commit %s\n", hash.data());

	if (!commit.timestamp.is_empty()) {
		time_t t = (time_t)atol(commit.timestamp.data());
		char buf[64];
		struct tm* tm_info = localtime(&t);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
		printf("Date:   %s\n", buf);
	}

	printf("\n    %s\n\n", commit.message.data());
}

void run_log()
{
	string hash = get_head_commit_hash();

	if (hash.is_empty()) {
		printf("No commits yet.\n");
		return;
	}

	while (!hash.is_empty()) {
		CommitObject commit = parse_commit(hash);
		print_commit(hash, commit);
		hash = commit.parent_hash;
	}
}
