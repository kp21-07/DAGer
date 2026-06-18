#include "dagr.h"
#include <time.h>

// Serialize a CommitObject to a plain-text binary_buffer
static binary_buffer serialize_commit(const CommitObject& commit)
{
	string content = "tree " + commit.tree_hash + "\n";

	if (!commit.parent_hash.is_empty()) {
		content = content + "parent " + commit.parent_hash + "\n";
	}

	content = content + "author User <user@example.com> " + commit.timestamp + "\n";
	content = content + "committer User <user@example.com> " + commit.timestamp + "\n";
	content = content + "\n";
	content = content + commit.message + "\n";

	return binary_buffer(content);
}

// Resolve HEAD -> branch ref path, e.g. ".dagr/refs/main"
static string resolve_head_ref_path()
{
	if (access(HEAD_FILE, F_OK) != 0) return "";

	string head = read_file(HEAD_FILE);

	// Expected format: "ref: refs/main\n"
	if (strncmp(head.data(), "ref: ", 5) != 0) return "";

	size_t start = 5;
	size_t end = head.length();
	while (end > start && (head.data()[end - 1] == '\n' || head.data()[end - 1] == '\r')) {
		end--;
	}

	// Build full path: ".dagr/" + "refs/main"
	string ref_rel = string(head.data() + start, end - start);
	return string(DAGR) + "/" + ref_rel;
}

// Read the current parent commit hash from the branch ref file
static string read_parent_hash(const string& ref_path)
{
	if (ref_path.is_empty() || access(ref_path.data(), F_OK) != 0) return "";

	string content = read_file(ref_path.data());
	size_t len = content.length();
	while (len > 0 && (content.data()[len - 1] == '\n' || content.data()[len - 1] == '\r')) {
		len--;
	}
	return string(content.data(), len);
}

string create_commit(const string& message)
{
	string tree_hash = write_tree();

	string ref_path    = resolve_head_ref_path();
	string parent_hash = read_parent_hash(ref_path);

	CommitObject commit;
	commit.tree_hash   = tree_hash;
	commit.parent_hash = parent_hash;
	commit.message     = message;

	char time_buf[32];
	sprintf(time_buf, "%ld", (long)time(nullptr));
	commit.timestamp = string(time_buf);

	binary_buffer commit_data = serialize_commit(commit);
	string commit_hash = write_object(commit_data);

	if (!ref_path.is_empty()) {
		write_file(ref_path.data(), commit_hash + "\n");
	}

	return commit_hash;
}