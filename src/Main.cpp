#include <iostream>
#include "Game/Game.h"

int main(int argc, char* argv[]) {
    // Create our new game instance. 
    Game game; 

    // Tracks the OS level return code. 
    int code = game.Initialize(); 

    // Only proceed to the main loop if initialize was successful. 
    if (code == 0)
        code = game.Run();

    // Return our OS level return code and finish the application. 
    return code;
}