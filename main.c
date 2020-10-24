// Generic Includes
#include <gb/gb.h>

// TODO: Split generic functions into separate header files eventually?

// Engine related variables
BYTE airborne;
INT8 gravity; // FIXME: const?
INT16 currentSpeedY;
BYTE gameRunning;

// CPU Efficient waiting function to be used
// when waiting is needed
void efficient_wait(INT16 loops)
{
    INT16 i = 0;
    for (i; i < loops; ++i)
    {
        wait_vbl_done();
    }
}

// Function to setup game
// to be called at the beginning of main
// sets up bg, turns on display, etc.
void setup_game()
{
    gameRunning = 1;
}

// Player movement function
void move_player();

// Jump function
void jump();

int main()
{
    setup_game();

    while(gameRunning)
    {
        // this check allows us to use a switch for the movement (prettier) but also enables
        // moving while in the air, which is nice.
        if (airborne)
        {
            // fall
        }
        
        switch (joypad())
        {
        case J_A:
            if (!airborne)
                jump();
            break;
        case J_LEFT:
            move_player(/*params*/);
            break;
        case J_RIGHT:
            move_player(/*params*/);
            break;
        default:
            break;
        }
    }

    return 0;
}