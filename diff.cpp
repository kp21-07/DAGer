#include "dagr.h"
#include <stdio.h>
#include <unistd.h>

// Prints diff using LCS between staged and working-tree versions
static void print_diff(const string& filename,
                       const vector<string>& old_lines, size_t m,
                       const vector<string>& new_lines, size_t n)
{
    // Flat LCS DP table: dp[i*(n+1)+j]
    vector<int> dp( (m + 1) * (n + 1), 0);

    for (size_t i = 1; i <= m; i++) {
        for (size_t j = 1; j <= n; j++) {
            size_t idx = i * (n + 1) + j;
            bool match = strcmp(old_lines[i-1].data(), new_lines[j-1].data()) == 0;
            if (match) {
                dp[idx] = dp[(i-1)*(n+1)+(j-1)] + 1;
            } else {
                int up   = dp[(i-1)*(n+1)+j];
                int left = dp[i*(n+1)+(j-1)];
                dp[idx]  = (up > left) ? up : left;
            }
        }
    }

    // Collecting edit ops by backtracking (comes out reversed)
    vector<int>    ops;
    vector<string> buf;

    size_t i = m, j = n;
    while (i > 0 || j > 0) {
        bool match = (i > 0 && j > 0 &&
                      strcmp(old_lines[i-1].data(), new_lines[j-1].data()) == 0);
        if (match) {
            ops.push_back(0);
            buf.push_back(old_lines[i-1]);
            i--;
            j--;
        }
        else if (j > 0 && (i == 0 || dp[i*(n+1)+(j-1)] >= dp[(i-1)*(n+1)+j])) {
            ops.push_back(2);
            buf.push_back(new_lines[j-1]);
            j--;
        } 
        else {
            ops.push_back(1);
            buf.push_back(old_lines[i-1]);
            i--;
        }
    }

    printf("\033[1mdiff --dagr a/%s b/%s\033[0m\n", filename.data(), filename.data());
    printf("\033[1m--- a/%s\033[0m\n", filename.data());
    printf("\033[1m+++ b/%s\033[0m\n", filename.data());

    // Printing in forward order
    for (size_t k = ops.size(); k > 0; k--) {
        const string& line = buf[k-1];
        if      (ops[k-1] == 0) printf(" %s\n",               line.data());
        else if (ops[k-1] == 1) printf("\033[31m-%s\033[0m\n", line.data());
        else                    printf("\033[32m+%s\033[0m\n", line.data());
    }
}

// Compares each staged file against its working-tree version and prints differences
void run_diff()
{
    vector<IndexEntry> entries = read_index();
    bool any_diff = false;

    for (size_t i = 0; i < entries.size(); i++) {
        const IndexEntry& entry = entries[i];

        if (access(entry.filename.data(), F_OK) != 0) continue; // deleted

        binary_buffer current  = read_binary_file(entry.filename.data());
        string        cur_hash = sha1(current);
        if (cur_hash == entry.hash) continue;  // identical

        any_diff = true;

        binary_buffer staged      = read_object(entry.hash);
        string        staged_str  = string(staged.data(),  staged.size());
        string        current_str = string(current.data(), current.size());

        vector<string> old_lines = staged_str.split('\n');
        vector<string> new_lines = current_str.split('\n');

        size_t m = old_lines.size();
        if (m > 0 && old_lines[m-1].is_empty()) m--;
        size_t n = new_lines.size();
        if (n > 0 && new_lines[n-1].is_empty()) n--;

        print_diff(entry.filename, old_lines, m, new_lines, n);
        printf("\n");
    }

    if (!any_diff)
        printf("\033[32mNo differences — working tree matches the index.\033[0m\n\n");
}
