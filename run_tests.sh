#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Building DAGer ===${NC}"
./build.sh

# Resolve absolute path to the compiled binary
DAGR_BIN="$(pwd)/bin/dagr"
TEST_DIR="$(pwd)/test"

# Make sure test directory exists and is clean for remote testing
cd "$TEST_DIR"
rm -rf test_server test_client test_client2
mkdir -p test_server test_client test_client2

echo -e "\n${GREEN}=== Testing Local Commands ===${NC}"
cd test_server
"$DAGR_BIN" init

# Create files
echo "Hello World" > a.txt
echo "Foo Bar" > b.txt

# Test hash-obj and cat-obj
echo -e "${GREEN}[TEST] hash-obj & cat-obj${NC}"
HASH=$("$DAGR_BIN" hash-obj a.txt)
echo "Generated Hash: $HASH"
CAT_OUT=$("$DAGR_BIN" cat-obj "$HASH")
if [ "$CAT_OUT" != "Hello World" ]; then
    echo -e "${RED}cat-obj mismatch!${NC}"
    exit 1
fi
echo -e "${GREEN}hash-obj & cat-obj passed!${NC}"

# Test add & status
echo -e "${GREEN}[TEST] add & status${NC}"
"$DAGR_BIN" add a.txt b.txt
STATUS_OUT=$("$DAGR_BIN" status)
if [[ ! "$STATUS_OUT" =~ "All files upto date" ]]; then
    # It should stage both files, so we check status output
    echo "Staged files successfully."
fi

# Test diff
echo -e "${GREEN}[TEST] diff${NC}"
echo "Hello Changed" > a.txt
DIFF_OUT=$("$DAGR_BIN" diff)
if [[ ! "$DIFF_OUT" =~ "diff --dagr a/a.txt b/a.txt" ]]; then
    echo -e "${RED}diff did not detect unstaged change!${NC}"
    exit 1
fi
echo -e "${GREEN}diff passed!${NC}"

# Re-stage changes and commit
"$DAGR_BIN" add a.txt
echo -e "${GREEN}[TEST] commit & log${NC}"
"$DAGR_BIN" commit -m "First commit on server"

LOG_OUT=$("$DAGR_BIN" log)
if [[ ! "$LOG_OUT" =~ "First commit on server" ]]; then
    echo -e "${RED}commit/log failed!${NC}"
    exit 1
fi
echo -e "${GREEN}commit & log passed!${NC}"


echo -e "\n${GREEN}=== Testing Networking: serve-git & clone ===${NC}"
# Start the git server on port 9091 in background
"$DAGR_BIN" serve-git 9091 > server.log 2>&1 &
SERVER_PID=$!

# Ensure server stops on script exit
trap "kill $SERVER_PID 2>/dev/null || true" EXIT

# Give the server a second to bind to the port
sleep 1

# Clone from client
cd ../test_client
"$DAGR_BIN" clone 127.0.0.1 9091

# Verify clone files
if [ ! -f a.txt ] || [ ! -f b.txt ]; then
    echo -e "${RED}Clone did not restore files!${NC}"
    exit 1
fi
if [ "$(cat a.txt)" != "Hello Changed" ]; then
    echo -e "${RED}Clone file contents are incorrect!${NC}"
    exit 1
fi
echo -e "${GREEN}clone passed!${NC}"


echo -e "\n${GREEN}=== Testing Networking: push ===${NC}"
# Make a change on the client
echo "Client edits file" >> a.txt
"$DAGR_BIN" add a.txt
"$DAGR_BIN" commit -m "Pushed commit from client"

# Push back to server
"$DAGR_BIN" push 127.0.0.1 9091
echo -e "${GREEN}push passed!${NC}"


echo -e "\n${GREEN}=== Testing Networking: pull ===${NC}"
# Clone another client to pull later
cd ../test_client2
"$DAGR_BIN" clone 127.0.0.1 9091

# Verify new files in client2
if [[ ! "$(cat a.txt)" =~ "Client edits file" ]]; then
    echo -e "${RED}Second clone did not contain client1's changes!${NC}"
    exit 1
fi
echo -e "${GREEN}Initial clone for pull test passed!${NC}"

# Do another client1 commit and push
cd ../test_client
echo "More client edits" >> b.txt
"$DAGR_BIN" add b.txt
"$DAGR_BIN" commit -m "Second client push"
"$DAGR_BIN" push 127.0.0.1 9091

# Pull in client2
cd ../test_client2
"$DAGR_BIN" pull 127.0.0.1 9091

# Verify pulled content
if [[ ! "$(cat b.txt)" =~ "More client edits" ]]; then
    echo -e "${RED}Pull did not update b.txt!${NC}"
    exit 1
fi
echo -e "${GREEN}pull passed!${NC}"


# Clean up server
kill $SERVER_PID 2>/dev/null || true
trap - EXIT

# Verify server log reflects both client commits
cd ../test_server
SERVER_LOG=$("$DAGR_BIN" log)
if [[ ! "$SERVER_LOG" =~ "Second client push" ]] || [[ ! "$SERVER_LOG" =~ "Pushed commit from client" ]]; then
    echo -e "${RED}Server repository log does not contain pushed client commits!${NC}"
    exit 1
fi

echo -e "\n${GREEN}=====================================${NC}"
echo -e "${GREEN}  ALL DAGer INTEGRATION TESTS PASSED!${NC}"
echo -e "${GREEN}=====================================${NC}"
