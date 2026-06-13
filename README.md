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

### `dagr cat-obj <hash>`

Prints the raw contents of any stored object to stdout.

---

## Project Structure

```
DAGer/
│
├── dagr.h          — shared declarations and structs
├── types.h         — custom string, vector, binary_buffer (no STL)
│
├── main.cpp        — CLI argument dispatch
├── commands.cpp    — thin command wrappers
├── repo.cpp        — repo initialization
├── hashing.cpp     — SHA-1 via OpenSSL
├── index.cpp       — read/write .dagr/index
├── obj_store.cpp   — write/read objects from .dagr/objects
├── tree.cpp        — serialize index → tree object
├── commit.cpp      — build and store commit objects
├── log.cpp         — parse and walk commit history
├── status.cpp      — working dir vs index comparison
├── utils.cpp       — file I/O helpers
│
├── bin/            — compiled output
├── build.sh        — build script
│
└── README.md
```
