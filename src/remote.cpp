#include "dagr.h"
#include"net_utils.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>

const uint8_t CMD_GET_REF     = 0x01;
const uint8_t CMD_REF_RESP    = 0x02;
const uint8_t CMD_GET_OBJ     = 0x03;
const uint8_t CMD_OBJ_RESP    = 0x04;
const uint8_t CMD_PUSH_OBJ    = 0x05;
const uint8_t CMD_UPDATE_REF  = 0x06;
const uint8_t CMD_STATUS_RESP = 0x07;

struct PacketHeader {
	char     magic[3]; // always "DAG"
	uint8_t  cmd;
	uint32_t len;
};

bool send_packet(int socket, uint8_t cmd, const char *payload, uint32_t len)
{
	PacketHeader header;
	memcpy(header.magic, "DAG", 3);
	header.cmd = cmd;
	header.len = htonl(len);

	if (send_all(socket, (const char*)&header, sizeof(header)) != sizeof(header))
		return false;

	if (len > 0 && payload != nullptr) {
		if (send_all(socket, payload, len) != (ssize_t)len)
			return false;
	}

	return true;
}

// Helper to receive a packet, verifying the "DAG" magic keyword.
bool recv_packet(int socket, uint8_t& cmd, binary_buffer& payload)
{
	PacketHeader header;

	if (recv_all(socket, (char*)&header, sizeof(header)) != sizeof(header))
		return false;

	if (memcmp(header.magic, "DAG", 3) != 0) {
		fprintf(stderr, "Protocol Error: Invalid packet magic keyword.\n");
		return false;
	}

	cmd = header.cmd;
	uint32_t len = ntohl(header.len);

	// Limit max packet size to 16MB to prevent integer overflow/OOM
	const uint32_t MAX_PACKET_SIZE = 16 * 1024 * 1024;
	if (len > MAX_PACKET_SIZE) {
		fprintf(stderr, "Protocol Error: Packet size %u exceeds safety limit.\n", len);
		return false;
	}

	payload = binary_buffer(len);

	if (len > 0) {
		if (recv_all(socket, payload.data(), len) != (ssize_t)len)
			return false;

		payload.m_size = len;
	}

	return true;
}

// Client helper to establish a TCP connection
static int connect_to_server(const string& ip, const string& port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if (fd < 0) return -1;

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port.data()));

	if (inet_pton(AF_INET, ip.data(), &addr.sin_addr) <= 0) {
		close(fd);
		return -1;
	}

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

// Local helper to check if an object exists on disk
static bool object_exists(const string& hash) {
	string dir_name = hash_dir(hash);
	string file_name = hash_file(hash);
	char file_path[512];
	sprintf(file_path, "%s/%s/%s", OBJECTS_DIR, dir_name.data(), file_name.data());
	return access(file_path, F_OK) == 0;
}

// Recursively requests and stores remote objects starting from a given hash
static void fetch_object(int socket, const string& hash)
{
	if (hash.is_empty() || object_exists(hash))
		return;

	if (!send_packet(socket, CMD_GET_OBJ, hash.data(), hash.length()))
		return;

	uint8_t resp_cmd;
	binary_buffer payload;
	if (!recv_packet(socket, resp_cmd, payload) || resp_cmd != CMD_OBJ_RESP || payload.size() < 40)
		return;

	// skipping the first 40 bytes which contains the hash
	binary_buffer obj_data;
	obj_data.append(payload.data() + 40, payload.size() - 40);
	
	write_object(obj_data);

	string content(obj_data.data(), obj_data.size());
	vector<string> lines = content.split('\n');

	bool is_commit = false;
	bool is_tree = false;

	if (lines.size() > 0) {
		vector<string> tokens = lines[0].split(' ');
		if (tokens.size() == 2 && tokens[0] == "tree") {
			is_commit = true;
		}
		else if (tokens.size() >= 3 && (tokens[0] == "blob" || tokens[0] == "tree")) {
			is_tree = true;
		}
	}

	if (is_commit) {
		for (size_t i = 0; i < lines.size(); ++i) {
			string line = lines[i];
			if (strncmp(line.data(), "tree ", 5) == 0) {
				fetch_object(socket, string(line.data() + 5, line.length() - 5));
			} else if (strncmp(line.data(), "parent ", 7) == 0) {
				fetch_object(socket, string(line.data() + 7, line.length() - 7));
			}
		}
	} else if (is_tree) {
		for (size_t i = 0; i < lines.size(); ++i) {
			vector<string> tokens = lines[i].split(' ');
			if (tokens.size() >= 3) {
				fetch_object(socket, tokens[2]); // Recursively fetch child blobs/subtrees
			}
		}
	}
}

// Restores working directory files and updates the index from a flat tree object
static void checkout_tree(const string& tree_hash)
{
	binary_buffer tree_data = read_object(tree_hash);
	string content(tree_data.data(), tree_data.size());
	vector<string> lines = content.split('\n');

	vector<IndexEntry> index_entries;
	
	for (size_t i = 0; i < lines.size(); ++i) {
		vector<string> tokens = lines[i].split(' ');
		if (tokens.size() >= 3 && tokens[0] == "blob") {
			string filename = tokens[1];
			string blob_hash = tokens[2];

			binary_buffer blob_data = read_object(blob_hash);
			write_binary_file(filename.data(), blob_data);
			
			index_entries.push_back({filename, blob_hash});
		}
	}
	
	write_index(index_entries);
}

// Local helper to read current branch commit hash from local .dagr
static string get_local_head_hash()
{
	if (access(HEAD_FILE, F_OK) != 0)
		return "";

	string head = read_file(HEAD_FILE);

	if (strncmp(head.data(), "ref: ", 5) != 0)
		return "";

	size_t start = 5;
	size_t end = head.length();

	while (end > start && (head.data()[end - 1] == '\n' || head.data()[end - 1] == '\r')) end--;

	string ref_path = string(DAGR) + "/" + string(head.data() + start, end - start);
	if (access(ref_path.data(), F_OK) != 0)
		return "";

	string hash = read_file(ref_path.data());
	size_t len = hash.length();

	while (len > 0 && (hash.data()[len - 1] == '\n' || hash.data()[len - 1] == '\r')) len--;

	return string(hash.data(), len);
}

static bool is_safe_hash(const string& hash) {
	if (hash.length() != 40) return false;
	for (size_t i = 0; i < 40; ++i) {
		char c = hash.data()[i];
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
			return false;
		}
	}
	return true;
}

static bool is_safe_ref(const string& ref) {
	if (ref.length() < 5) return false;
	if (strncmp(ref.data(), "refs/", 5) != 0) return false;

	for (size_t i = 0; i < ref.length(); ++i) {
		char c = ref.data()[i];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
			  c == '/' || c == '-' || c == '_' || c == '.')) {
			return false;
		}
		if (c == '.' && i + 1 < ref.length() && ref.data()[i + 1] == '.') {
			return false;
		}
		if (c == '/' && i + 1 < ref.length() && ref.data()[i + 1] == '/') {
			return false;
		}
	}
	return true;
}

static void handle_client(int client_fd)
{
	uint8_t cmd;
	binary_buffer payload;

	// Loop receiving packets until client disconnects
	while (recv_packet(client_fd, cmd, payload)) {
		if (cmd == CMD_GET_REF) {
			string ref_name(payload.data(), payload.size());
			if (!is_safe_ref(ref_name)) {
				fprintf(stderr, "Security Warning: Rejected unsafe ref name from client.\n");
				break;
			}
			string ref_path = string(DAGR) + "/" + ref_name;

			string ref_hash;
			if (access(ref_path.data(), F_OK) == 0) {
				string raw = read_file(ref_path.data());
				size_t len = raw.length();
				while (len > 0 && (raw.data()[len - 1] == '\n' || raw.data()[len - 1] == '\r')) len--;
				ref_hash = string(raw.data(), len);
			}
			send_packet(client_fd, CMD_REF_RESP, ref_hash.data(), ref_hash.length());
		}
		else if (cmd == CMD_GET_OBJ) {
			string obj_hash(payload.data(), payload.size());
			if (!is_safe_hash(obj_hash)) {
				fprintf(stderr, "Security Warning: Rejected unsafe object hash from client.\n");
				break;
			}
			binary_buffer obj_data = read_object(obj_hash);

			if (obj_data.empty()) {
					send_packet(client_fd, CMD_OBJ_RESP, nullptr, 0);
			} else {
				// hash (40 bytes) + actual content
				binary_buffer response(40 + obj_data.size());
				response.append(obj_hash.data(), 40);
				response.append(obj_data.data(), obj_data.size());
				send_packet(client_fd, CMD_OBJ_RESP, response.data(), response.size());
			}
		}
		else if (cmd == CMD_PUSH_OBJ) {
			if (payload.size() < 40) continue;
			
			string obj_hash(payload.data(), 40);
			if (!is_safe_hash(obj_hash)) {
				fprintf(stderr, "Security Warning: Rejected unsafe object hash in push from client.\n");
				break;
			}

			binary_buffer obj_data;
			obj_data.append(payload.data() + 40, payload.size() - 40);
			
			write_object(obj_data);
		}
		else if (cmd == CMD_UPDATE_REF) {
			if (payload.size() < 42) continue;
			uint8_t name_len = payload.data()[0];
			if (1 + name_len + 40 > payload.size()) continue;

			string ref_name(payload.data() + 1, name_len);
			if (!is_safe_ref(ref_name)) {
				fprintf(stderr, "Security Warning: Rejected unsafe ref name in update from client.\n");
				break;
			}

			string new_hash(payload.data() + 1 + name_len, 40);
			if (!is_safe_hash(new_hash)) {
				fprintf(stderr, "Security Warning: Rejected unsafe object hash in update from client.\n");
				break;
			}

			string ref_path = string(DAGR) + "/" + ref_name;
			write_file(ref_path.data(), new_hash + "\n");

			char status = 0; // 0: Success
			send_packet(client_fd, CMD_STATUS_RESP, &status, 1);
		}
	}
	close(client_fd);
}

void cmd_serve_git(const string& port)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("Failed to create socket");
		return;
	}

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(atoi(port.data()));

	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("Bind failed");
		close(server_fd);
		return;
	}

	if (listen(server_fd, 10) < 0) {
		perror("Listen failed");
		close(server_fd);
		return;
	}

	printf("DAGer server listening on port %s...\n", port.data());

	int active_children = 0;
	const int MAX_CHILDREN = 10;

	while (true) {
		// Reap any exited children non-blockingly
		pid_t reaped_pid;
		while ((reaped_pid = waitpid(-1, nullptr, WNOHANG)) > 0) {
			if (active_children > 0) {
				active_children--;
			}
		}

		// Block until a child slot opens up
		while (active_children >= MAX_CHILDREN) {
			pid_t pid = waitpid(-1, nullptr, 0);
			if (pid > 0) {
				if (active_children > 0) {
					active_children--;
				}
			} else if (pid < 0) {
				if (errno == ECHILD) {
					active_children = 0;
					break;
				} else if (errno == EINTR) {
					continue;
				}
			}
		}

		int client_fd = accept(server_fd, nullptr, nullptr);
		if (client_fd < 0) {
			if (errno == EINTR) {
				continue;
			}
			perror("Accept failed");
			continue;
		}

		// 10s timeout to prevent idle hangs
		struct timeval tv{10, 0};
		setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
		setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

		pid_t pid = fork();
		if (pid == 0) {
			// Child process
			close(server_fd);
			handle_client(client_fd);
			exit(0);
		} else if (pid > 0) {
			// Parent process
			close(client_fd);
			active_children++;
		} else {
			perror("Fork failed");
			close(client_fd);
		}
	}
	close(server_fd);
}

void cmd_clone(const string& ip, const string& port)
{
	int sock = connect_to_server(ip, port);
	if (sock < 0) {
		fprintf(stderr, "Error: Could not connect to server %s:%s\n", ip.data(), port.data());
		return;
	}

	string ref_name = "refs/main";
	send_packet(sock, CMD_GET_REF, ref_name.data(), ref_name.length());

	uint8_t cmd;
	binary_buffer payload;
	if (!recv_packet(sock, cmd, payload) || cmd != CMD_REF_RESP) {
		fprintf(stderr, "Error: Did not receive valid reference from server.\n");
		close(sock);
		return;
	}

	// initialize .dagr directory in local
	repo_init();

	if (payload.empty()) {
		printf("Cloned empty repository.\n");
		close(sock);
		return;
	}

	string remote_hash(payload.data(), payload.size());
	printf("Cloning from remote commit %s...\n", remote_hash.data());

	// fetch object file from remote
	fetch_object(sock, remote_hash);

	// Update local HEAD
	write_file(".dagr/HEAD", "ref: refs/main\n");
	write_file(".dagr/refs/main", remote_hash + "\n");

	// Checkout files
	binary_buffer commit_data = read_object(remote_hash);
	string content(commit_data.data(), commit_data.size());
	vector<string> lines = content.split('\n');
	for (size_t i = 0; i < lines.size(); ++i) {
		if (strncmp(lines[i].data(), "tree ", 5) == 0) {
			checkout_tree(string(lines[i].data() + 5, lines[i].length() - 5));
			break;
		}
	}

	printf("Finished Cloning from server %s:%s.\n", ip.data(), port.data());
	close(sock);
}

void cmd_pull(const string& ip, const string& port)
{
	int sock = connect_to_server(ip, port);
	if (sock < 0) {
		fprintf(stderr, "Error: Could not connect to server %s:%s\n", ip.data(), port.data());
		return;
	}

	string ref_name = "refs/main";
	send_packet(sock, CMD_GET_REF, ref_name.data(), ref_name.length());

	uint8_t cmd;
	binary_buffer payload;
	if (!recv_packet(sock, cmd, payload) || cmd != CMD_REF_RESP) {
		fprintf(stderr, "Error: Failed to fetch remote reference.\n");
		close(sock);
		return;
	}

	if (payload.empty()) {
		printf("Remote %s:%s is empty. Nothing to pull.\n", ip.data(), port.data());
		close(sock);
		return;
	}

	string remote_hash(payload.data(), payload.size());
	string local_hash = get_local_head_hash();

	if (local_hash == remote_hash) {
		printf("Already up to date.\n");
		close(sock);
		return;
	}

	printf("Fetching remote changes...\n");

	// fethc object from remote
	fetch_object(sock, remote_hash);

	// Update local ref
	write_file(".dagr/refs/main", remote_hash + "\n");

	// Checkout
	binary_buffer commit_data = read_object(remote_hash);
	string content(commit_data.data(), commit_data.size());
	vector<string> lines = content.split('\n');
	for (size_t i = 0; i < lines.size(); ++i) {
		if (strncmp(lines[i].data(), "tree ", 5) == 0) {
			checkout_tree(string(lines[i].data() + 5, lines[i].length() - 5));
			break;
		}
	}

	printf("Successfully pulled from %s:%s. Updated HEAD to %s\n", ip.data(), port.data(), remote_hash.data());
	close(sock);
}

// Recursively collects commit, tree, and blob hashes to push
static void collect_push_objects(const string& commit_hash, vector<string>& list)
{
	if (commit_hash.is_empty()) return;

	for (size_t i = 0; i < list.size(); ++i) {
		if (list[i] == commit_hash) return;
	}

	list.push_back(commit_hash);

	binary_buffer commit_data = read_object(commit_hash);
	string content(commit_data.data(), commit_data.size());
	vector<string> lines = content.split('\n');

	string tree_hash;
	string parent_hash;

	for (size_t i = 0; i < lines.size(); ++i) {
		if (strncmp(lines[i].data(), "tree ", 5) == 0) {
			tree_hash = string(lines[i].data() + 5, lines[i].length() - 5);
		} else if (strncmp(lines[i].data(), "parent ", 7) == 0) {
			parent_hash = string(lines[i].data() + 7, lines[i].length() - 7);
		}
	}

	if (!tree_hash.is_empty()) {
		list.push_back(tree_hash);
		binary_buffer tree_data = read_object(tree_hash);
		string tc(tree_data.data(), tree_data.size());
		vector<string> tl = tc.split('\n');
		for (size_t i = 0; i < tl.size(); ++i) {
			vector<string> tokens = tl[i].split(' ');
			if (tokens.size() >= 3 && tokens[0] == "blob") {
				list.push_back(tokens[2]);
			}
		}
	}

	if (!parent_hash.is_empty()) {
		collect_push_objects(parent_hash, list);
	}
}

void cmd_push(const string& ip, const string& port)
{
	int sock = connect_to_server(ip, port);
	if (sock < 0) {
		fprintf(stderr, "Error: Could not connect to server %s:%s\n", ip.data(), port.data());
		return;
	}

	string ref_name = "refs/main";
	send_packet(sock, CMD_GET_REF, ref_name.data(), ref_name.length());

	uint8_t cmd;
	binary_buffer payload;
	if (!recv_packet(sock, cmd, payload) || cmd != CMD_REF_RESP) {
		fprintf(stderr, "Error: Failed to fetch remote reference.\n");
		close(sock);
		return;
	}

	string remote_hash(payload.data(), payload.size());
	string local_hash = get_local_head_hash();

	if (local_hash.is_empty()) {
		printf("No local commits to push.\n");
		close(sock);
		return;
	}

	if (local_hash == remote_hash) {
		printf("Everything up-to-date.\n");
		close(sock);
		return;
	}

	vector<string> objects_to_push;
	collect_push_objects(local_hash, objects_to_push);

	printf("Pushing %zu objects to remote...\n", objects_to_push.size());

	for (size_t i = 0; i < objects_to_push.size(); ++i) {
		string hash = objects_to_push[i];
		binary_buffer obj_data = read_object(hash);
		if (obj_data.empty()) continue;

		binary_buffer obj_payload(40 + obj_data.size());
		obj_payload.append(hash.data(), 40);
		obj_payload.append(obj_data.data(), obj_data.size());

		send_packet(sock, CMD_PUSH_OBJ, obj_payload.data(), obj_payload.size());
	}

	binary_buffer update_payload(1 + ref_name.length() + 40);
	char name_len = ref_name.length();
	update_payload.append(&name_len, 1);
	update_payload.append(ref_name.data(), ref_name.length());
	update_payload.append(local_hash.data(), local_hash.length());

	send_packet(sock, CMD_UPDATE_REF, update_payload.data(), update_payload.size());

	if (recv_packet(sock, cmd, payload) && cmd == CMD_STATUS_RESP && payload.size() > 0) {
		if (payload.data()[0] == 0) {
			printf("Push successful! Remote HEAD updated to %s\n", local_hash.data());
		} else {
			fprintf(stderr, "Error: Server failed to update reference.\n");
		}
	} else {
		fprintf(stderr, "Error: Did not receive status response from server.\n");
	}

	close(sock);
}
