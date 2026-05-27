# DAGer
A mini-git clone in C++

This is  learning project focused on understanding the workings of git.

---

## Build

### Requirements

- C++20
- clang++

### Compile

```bash
./build.sh
```

## Features (so-far)

### dagr init

Creates:

```
.dagr/
├── objects/
├── refs/
├── HEAD
└── index
```

### dagr hash-obj <file>

Stores objects in a Git-like layout:
```
.dagr/objects/2a/ae6c35c94fcfb415dbe95f408b9ce91ee846ed
```

### dagr cat-obj <hash>

Outputs the data stores respective of the hash

### dagr add <file>

Update .dagr/index to store the file name and the most recent hashed content.

### dagr status

Identifies the modified files compared to the most recet staged content.

## Project Structure

```
dagr/
│
├── dagr.h
├── types.h
│
├── main.cpp
├── command.cpp
├── repo.cpp
├── hashing.cpp
├── index.cpp
├── obj_store.cpp
├── utils.cpp
│
├── bin/
├── build.sh
│
└── README.md
```
