#include "Game.hpp"
#include <iostream>
#include <string_view>

int main(int argc, char* argv[]) {
    bool testMode = false;

    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--test") {
            testMode = true;
        }
    }

    try {
        Game game(testMode);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "[Space Defender] Fatal Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[Space Defender] Unknown fatal error occurred." << std::endl;
        return 1;
    }

    return 0;
}
