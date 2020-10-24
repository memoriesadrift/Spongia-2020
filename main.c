// Generic Includes
#include <gb/gb.h>

// TODO: Split generic functions into separate header files eventually?

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
void setup_game();

int main()
{
    return 0;
}