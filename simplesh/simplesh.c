#include<stdio.h>
#include<string>
#include<vector>
#include<unistd.h>
#include<iostream>
#include<sys/wait.h>
#include<errno.h>
#include<string.h>
#include<signal.h>
#include<algorithm>

#define TESTERR(x) test_error(x,false,__func__)
#define BUFFER 4096

using namespace std;

vector<int *> pipes;
vector<int> processes;
int main_pid;

int test_error(int ret, bool critical = false, string where = "") {
    if (ret == -1) {
        if (errno == EINTR) { //Interrupted by parent process
            return ret;
        }
        perror(where.c_str());
        if (critical) {
            exit(1);
        }
    }
    return ret;
}


void sigint_handler(int signum, siginfo_t *info, void *context) {
    for_each(begin(processes), end(processes), [](int x) {
        if (x != getpid()) {
            TESTERR(kill(x, SIGINT));
        }
    });
    if (getpid() != main_pid) {
        exit(1);
    }
}

vector<char *> split_command(const string &command) {
    vector<char *> ans;
    char *temp_str = strdup(command.c_str());
    const char sep[3] = "\n ";
    char *token;
    token = strtok(temp_str, sep);
    while (token != nullptr) {
        ans.push_back(token);
        token = strtok(NULL, sep);
    }
    return ans;
}


void exec_one(vector<char *> args) {
    char *argv[args.size() + 1];
    for (int i = 0; i < args.size(); i++) {
        argv[i] = args[i];
    }
    argv[args.size()] = NULL;
    TESTERR(execvp(argv[0], argv));

}

void execute_rec(string command, bool first = false) {
    int pipe_fd[2];
    TESTERR(pipe(pipe_fd));
    size_t pos = command.find('|');
    string next_command;
    if (pos != string::npos) {
        next_command = command.substr(pos + 1, command.size());
        command = command.substr(0, pos);
    }
    vector<char *> args = split_command(command);

    pid_t pid = TESTERR(fork());

    if (pid == 0) {
        if (!first) {
            int *out_pipe = pipes.back();
            TESTERR(dup2(out_pipe[0], STDIN_FILENO));
            TESTERR(close(out_pipe[0]));
        }
        if (pos != string::npos) {
            TESTERR(dup2(pipe_fd[1], STDOUT_FILENO));
            TESTERR(close(pipe_fd[1]));
        }
        processes.push_back(getpid());
        exec_one(args);
    } else {
        int res;
        processes.push_back(getpid());
        TESTERR(close(pipe_fd[1]));
        if (pos != string::npos) {
            pipes.push_back(pipe_fd);
            execute_rec(next_command);
        }
        TESTERR(close(pipe_fd[0]));
        TESTERR(waitpid(pid, &res, 0));
    }
}

vector<char> buf(BUFFER);

string read_line() {
    string result;
    ssize_t last_read = read(0, buf.data(), BUFFER);

    if (last_read == 0) {
        exit(0);
    }
    while (last_read != 0) {
        if (last_read == -1) {
            if (errno != EINTR) {
                perror("read");
            }
            exit(0);
        }
        for (int i = 0; i < last_read; i++) {
            result += buf[i];
        }
        if (buf[last_read - 1] == '\n') {
            break;
        }
        last_read = read(0, buf.data(), BUFFER);
    }
    return result;
}


int main() {
    struct sigaction action;
    action.sa_sigaction = &sigint_handler;
    action.sa_flags = SA_SIGINFO;
    TESTERR(sigemptyset(&action.sa_mask));
    TESTERR(sigaddset(&action.sa_mask, SIGINT));
    TESTERR(sigaction(SIGINT, &action, NULL));
    main_pid = getpid();
    write(STDOUT_FILENO, "\n$", 2);
    while (true) {
        string command = read_line();
        if (command.size() == 0) {
            break;
        }
        execute_rec(command, true);
        pipes.clear();
        processes.clear();
        write(STDOUT_FILENO, "\n$", 2);
    }
}
