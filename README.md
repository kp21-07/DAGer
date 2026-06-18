# DAGer
A mini-git clone in C++

A learning project focused on understanding the internals of Git — content-addressable object storage, the index, tree objects, and commit chains.

---

## Build

### Requirements

- C++20
- clang++
- OpenSSL (`libcrypto`)

### Compile

```bash
./build.sh
```

---

## Features

- **Zero STL** — custom implementations of `string`, `vector`, and `binary_buffer` in `types.h`; no `<string>`, `<vector>`, or `<algorithm>`
- **Content-addressable object store** — files are stored as SHA-1 hashed blobs under `.dagr/objects/<xx>/<rest>`, identical to Git's layout
- **Staging index** — `.dagr/index` tracks filename → blob hash mappings, updated by `dagr add`
- **Tree objects** — the index is snapshotted into a flat tree object on every commit
- **Commit chain** — each commit stores a `tree`, optional `parent`, and metadata; commits form a linked list traversable by `dagr log`
- **Working tree status** — detects modified, deleted, and untracked files using POSIX `access()` and `opendir()`/`readdir()`
- **Line-level diff** — LCS-based diff between the staged version and working tree, with colored output
- **Remote Synchronization** — clone, push, and pull repositories over raw TCP sockets with concurrent connection handling, a strict 16MB packet memory limit, and path/hash validation (no built-in authentication/encryption)
- **POSIX only** — no platform-specific APIs beyond POSIX and OpenSSL

---

## Commands

### `dagr init`

Creates a new repository in the current directory:

```
.dagr/
├── objects/
├── refs/
├── HEAD
└── index
```

### `dagr add <file...>`

Hashes a file, stores it as a blob object, and stages it in `.dagr/index`:

```bash
dagr add hello.txt
dagr add a.txt b.txt
```

### `dagr status`

Compares the working directory against the staged index and reports:

- **Modified Files** — tracked files whose content has changed since last `add`
- **Deleted Files** — tracked files that no longer exist on disk
- **Untracked Files** — files in the current directory not yet staged

### `dagr commit -m <message>`

Snapshots the current index into a commit object and updates the branch ref:

```bash
dagr commit -m "Initial commit"
# [a3f4c8b...] Initial commit
```

Internally this writes a tree object from the index, then a commit object pointing to that tree (and the previous commit as its parent).

### `dagr log`

Walks the commit chain from HEAD backwards and prints each commit:

```
commit a3f4c8b2d1e...
Date:   2026-06-13 13:12:45

    Initial commit

```

### `dagr write-tree`

Low-level plumbing: writes the current index as a tree object and prints its hash.

### `dagr hash-obj <file>`

Stores a file as a raw blob object and prints its SHA-1 hash:

```
.dagr/objects/2a/ae6c35c94fcfb415dbe95f408b9ce91ee846ed
```

### `dagr diff`

Compares each staged file against its current working-tree version using **LCS (Longest Common Subsequence)** and prints a colored unified diff:

```
diff --dagr a/hello.txt b/hello.txt
--- a/hello.txt
+++ b/hello.txt
 unchanged line
-old line
+new line
 another unchanged line
```

Red lines (`-`) were in the staged version. Green lines (`+`) are in the working tree.

### `dagr cat-obj <hash>`

Prints the raw contents of any stored object to stdout.

### `dagr serve-git <port>`

Starts the Git server daemon on the specified port. It listens for incoming raw TCP socket connections and coordinates object exchanges (clones, pulls, pushes).

### `dagr clone <ip> <port>`

Clones a remote repository over TCP. It downloads the commit history and files, sets up tracking branches, and restores the working directory.

### `dagr push <ip> <port>`

Traverses the local commit history, identifies all missing commits/trees/blobs, packages and uploads them to the remote server, and updates the remote's branch tracking head.

### `dagr pull <ip> <port>`

Fetches new commit objects and files from the remote server, updates the local branch HEAD, and checkouts the files into your working directory.

---

## Project Structure

```
DAGer/
│
├── src/                — source files
│   ├── dagr.h          — shared declarations and structs
│   ├── types.h         — custom string, vector, binary_buffer (no STL)
│   ├── net_utils.h     — socket utilities (partial send/recv)
│   ├── net_utils.cpp   — low-level network I/O helpers
│   ├── remote.cpp      — TCP client/server sync protocols
│   ├── main.cpp        — CLI argument dispatch
│   ├── commands.cpp    — thin command wrappers
│   ├── repo.cpp        — repo initialization
│   ├── hashing.cpp     — SHA-1 via OpenSSL
│   ├── index.cpp       — read/write .dagr/index
│   ├── obj_store.cpp   — write/read objects from .dagr/objects
│   ├── tree.cpp        — serialize index → tree object
│   ├── commit.cpp      — build and store commit objects
│   ├── log.cpp         — parse and walk commit history
│   ├── status.cpp      — working dir vs index comparison
│   ├── diff.cpp        — LCS-based line diff (staged vs working tree)
│   └── utils.cpp       — file I/O helpers
│
├── bin/                — compiled output
├── test/               — test suite & test repositories
│   └── run_tests.sh    — automated local & network integration tests
│
├── build.sh            — build script
│
└── README.md
```
