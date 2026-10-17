#include <cerrno>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

void print_error()
{
    std::cerr << "An error has occurred\n";
}

void run_program(std::vector<std::string>& args)
{
    std::string program = "/bin/" + args[0];
    if (access(program.c_str(), X_OK) != 0) {
        print_error();
        return;
    }

    std::vector<char*> argv;
    for (std::string& arg : args) {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        print_error();
        return;
    }

    if (pid == 0) {
        execv(program.c_str(), argv.data());
        // Успішний execv не повертається до цього коду.
        print_error();
        _exit(1);
    }

    while (waitpid(pid, nullptr, 0) == -1) {
        if (errno != EINTR) {
            print_error();
            return;
        }
    }
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

    std::string line;
    while (true) {
        if (interactive) {
            std::cout << "wish> " << std::flush;
        }

        if (!std::getline(*input, line)) {
            break;
        }

        std::istringstream words(line);
        std::vector<std::string> args;
        std::string word;
        while (words >> word) {
            args.push_back(word);
        }

        if (args.empty()) {
            continue;
        }

        if (args[0] == "exit") {
            if (args.size() != 1) {
                print_error();
                continue;
            }
            return 0;
        }

        run_program(args);
    }

    if (input->bad()) {
        print_error();
        return 1;
    }

    return 0;
}
