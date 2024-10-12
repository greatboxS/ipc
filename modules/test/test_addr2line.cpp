#include <iostream>
#include <execinfo.h>
#include <cstdlib>
#include <sstream>
#include <regex>
#include <csignal>
#include <fstream>

std::string getCommandLine() {
    std::ifstream file("/proc/self/cmdline");
    std::string cmdline;
    if (file) {
        std::getline(file, cmdline);
    } else {
        std::cerr << "Error opening /proc/self/cmdline" << std::endl;
    }

    return cmdline;
}

void printBacktrace() {
    const int maxFrames = 10;
    void *buffer[maxFrames];
    int numAddresses = backtrace(buffer, maxFrames);
    char **symbols = backtrace_symbols(buffer, numAddresses);
    std::string cmdline = getCommandLine();

    // Regular expression to extract the address from the symbol
    std::regex pattern(R"(\+0x([0-9a-fA-F]+))");

    for (int i = 0; i < numAddresses; i++) {
        std::string symbol = symbols[i];
        std::smatch match;
        printf("[%d] symbol: %s\n", i, symbol.c_str());

        // Find the memory address in the symbol string
        if (std::regex_search(symbol, match, pattern)) {
            std::string address = std::string(match[1].str().c_str());
            // Construct the addr2line command with the extracted address
            std::ostringstream command;
            command << "addr2line -e " << cmdline.c_str() << " " <<  address.c_str();

            // Execute the command using system() and print the output
            int result = system(command.str().c_str());
            if (result == -1) {
                std::cerr << "Error: Failed to run addr2line for address " << address << std::endl;
            }
        } else {
            std::cerr << "Error: Could not extract address from symbol: " << symbol << std::endl;
        }
    }

    free(symbols);
}

void signalHandler(int signum) {
    printf("signal %d\n", signal);
    if (signum == SIGINT) {
        exit(0);
    }
    printBacktrace();
    exit(signum);
}

void causeError() {
    int *p = nullptr;
    *p = 10; // Intentional segmentation fault
}

int main(int argc, const char *argv[]) {
    signal(SIGSEGV, signalHandler);

    try {
        causeError();
    } catch (...) {
        printBacktrace();
    }
    return 0;
}
