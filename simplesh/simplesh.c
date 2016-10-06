#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

vector<pid_t> childs;
size_t BUFFER_SIZE = 8192;
vector<string> tmp;

void sig_handler(int signo, siginfo_t *siginfo, void *context) {
    for (auto i = 0; i < childs.size(); i++) {
        kill(childs[i], signo);
        waitpid(childs[i], NULL, 0);
    }
    write(STDOUT_FILENO, "\n", 1);
}



vector<char *> make_tokens(char *str, char *sep) {
    auto *token = strtok(str, sep);
    vector<char *> tokens;
    while (token) {
        tokens.push_back(token);
        token = strtok(NULL, sep);
    }
    return tokens;
}

string read_string() {
    string s = "";
    if (!tmp.empty()) {
        s = tmp[0];
        tmp.erase(tmp.begin());
    }
    if (s.size() && s.back() == '\n') {
        s.pop_back();
        return s;
    }
    char buffer[BUFFER_SIZE];
    ssize_t total_read_cnt = 0;
    ssize_t read_cnt;

    bool eol = 0;
    while (!eol && (read_cnt = read(STDIN_FILENO, buffer + total_read_cnt, sizeof(buffer))) > 0) {
        total_read_cnt += read_cnt;
        for_each(buffer + total_read_cnt - read_cnt, buffer + total_read_cnt, [&](char &ch) {
            if (ch == '\n') {
                eol = 1;
            }
        });
    }

    if (total_read_cnt == 0 && read_cnt == 0 && !s.size()) {
        raise(SIGINT);
        exit(0);
    }
    string t = "";
    for_each(buffer, buffer + total_read_cnt, [&](char &ch) {
        t += ch;
        if (ch == '\n') {
            tmp.push_back(t);
            t = "";
        }
    });
    if (t.size()) {
        tmp.push_back(t);
    }
    if (!tmp.empty()) {
        s += tmp[0];
        tmp.erase(tmp.begin());
    }
    if (s.size()) {
        s.pop_back();
    }
    return s;
}

void process(vector<char *> &commands) {
    int fpipe[2];
    int spipe[2];
    for (auto i = 0; i < commands.size(); i++) {
        auto parts = make_tokens(commands[i], (char *) " ");
        pipe(spipe);
        auto newpid = fork();
        if (newpid < 0) {
            perror("Error fork\n");
        } else {
            childs.push_back(newpid);
        }
        if (newpid == 0) {
            if (i > 0) {
                dup2(fpipe[0], STDIN_FILENO);
                close(fpipe[0]);
                close(fpipe[1]);
            }
            if (i != commands.size() - 1) dup2(spipe[1], STDOUT_FILENO);
            close(spipe[0]);
            close(spipe[1]);
            parts.push_back(NULL);
            if (execvp(parts[0], parts.data()) != 0) {
                   exit(errno);
            }
        } else {
            if (i > 0) {
                close(fpipe[0]);
                close(fpipe[1]);
            }
            if (i == commands.size() - 1) {
                close(spipe[0]);
                close(spipe[1]);
            }
        }
        fpipe[0] = spipe[0];
        fpipe[1] = spipe[1];
    }
    for (auto child : childs) {
        waitpid(child, 0, 0);
    }
}

int main(void) {
    struct sigaction sa;
    sa.sa_sigaction = &sig_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) != 0)
        return errno;
    while (1) {
        write(STDOUT_FILENO, "$ ", 2);
        string str = read_string();
        vector<char *> commands = make_tokens(const_cast<char *>(str.c_str()), (char *) "|");
        process(commands);
    }
}
