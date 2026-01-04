#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Sleeping for 1 second...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "Wake up!\n";
    return 0;
}