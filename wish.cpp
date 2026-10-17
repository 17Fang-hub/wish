#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void print_error()
{
    std::cerr << "An error has occurred\n";
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
        std::string command;
        if (!(words >> command)) {
            continue;
        }

        if (command == "exit") {
            std::string extra_argument;
            if (words >> extra_argument) {
                print_error();
                continue;
            }
            return 0;
        }

        // Запуск зовнішніх команд додамо на наступному етапі.
        print_error();
    }

    if (input->bad()) {
        print_error();
        return 1;
    }

    return 0;
}
