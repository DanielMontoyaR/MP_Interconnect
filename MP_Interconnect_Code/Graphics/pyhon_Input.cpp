#include <iostream>
#include <fstream>
#include <string>

void wait_for_continue() {
    std::ifstream pipe("tmp/continue_pipe");
    std::string signal;
    std::getline(pipe, signal);
    if (signal == "continue") {
        std::cout << "Continuing C++ process...\n";
    }
}

int main() {
    std::cout << "Waiting" << std::endl;
    wait_for_continue();

    std::cout << "Waiting Again" << std::endl;
    wait_for_continue();

    std::cout << "Finished" << std::endl;
    return 0;
}
