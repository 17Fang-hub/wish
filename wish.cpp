#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

void print_error()
{
    std::cerr << "An error has occurred\n";
}

pid_t run_program(std::vector<std::string>& args,
                  const std::vector<std::string>& shell_path,
                  const std::string& output_file)
{
    std::string program;
    for (const std::string& directory : shell_path) {
        std::string candidate = directory + "/" + args[0];
        if (access(candidate.c_str(), X_OK) == 0) {
            program = candidate;
            break;
        }
    }

    if (program.empty()) {
        print_error();
        return -1;
    }

    std::vector<char*> argv;
    for (std::string& arg : args) {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        print_error();
        return -1;
    }

    if (pid == 0) {
        if (!output_file.empty()) {
            int fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                print_error();
                _exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        execv(program.c_str(), argv.data());
        // Успішний execv не повертається до цього коду.
        print_error();
        _exit(1);
    }

    return pid;
}

int main(int argc, char* argv[])
{
    if (argc > 2) {
        print_error();
        return 1;
    }

    const bool interactive = (argc == 1);
    std::ifstream batch_file;
    std::istream* input = &std::cin;

    if (!interactive) {
        batch_file.open(argv[1]);
        if (!batch_file) {
            print_error();
            return 1;
        }
        input = &batch_file;
    }

    std::vector<std::string> shell_path = {"/bin"};
    std::string line;
    while (true) {
        if (interactive) {
            std::cout << "wish> " << std::flush;
        }

        if (!std::getline(*input, line)) {
            break;
        }

        std::istringstream commands(line);
        std::string command;
        std::vector<pid_t> pids;
        while (std::getline(commands, command, '&')) {
            std::string output_file;
            std::size_t redirect = command.find('>');
            if (redirect != std::string::npos) {
                if (command.find('>', redirect + 1) != std::string::npos) {
                    print_error();
                    continue;
                }

                std::istringstream files(command.substr(redirect + 1));
                std::string extra_file;
                if (!(files >> output_file) || (files >> extra_file)) {
                    print_error();
                    continue;
                }
                command = command.substr(0, redirect);
            }

            std::istringstream words(command);
            std::vector<std::string> args;
            std::string word;
            while (words >> word) {
                args.push_back(word);
            }

            if (args.empty()) {
                if (!output_file.empty()) {
                    print_error();
                }
                continue;
            }

            if (args[0] == "exit") {
                if (args.size() != 1) {
                    print_error();
                    continue;
                }
                return 0;
            }

            if (args[0] == "cd") {
                if (args.size() != 2) {
                    print_error();
                } else if (chdir(args[1].c_str()) != 0) {
                    print_error();
                }
                continue;
            }

            if (args[0] == "path") {
                shell_path.clear();
                for (std::size_t i = 1; i < args.size(); i++) {
                    shell_path.push_back(args[i]);
                }
                continue;
            }

            pid_t pid = run_program(args, shell_path, output_file);
            if (pid > 0) {
                pids.push_back(pid);
            }
        }

        for (pid_t pid : pids) {
            waitpid(pid, nullptr, 0);
        }
    }

    if (input->bad()) {
        print_error();
        return 1;
    }

    return 0;
}
