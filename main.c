// Generic Includes
#include <gb/gb.h>

// TODO: Split generic functions into separate header files eventually?

// Engine related variables
BYTE airborne;
INT8 gravity; // FIXME: const?
INT16 currentSpeedY;
BYTE gameRunning;
const UINT8 tileSize = 8; // FIXME: doesn't work as #define TILESIZE 8 in move_sprite() function call - why?

// Bigger sprite supporting struct
struct GameObject 
{
    UBYTE spriteids[4];
    UINT8 x;
    UINT8 y;
    UINT8 width;
    UINT8 height;
};

// Player vars
struct GameObject player;

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

void setup_player()
{
    player.height = 16;
    player.width = 8;
    player.x = 80; // placeholder
    player.y = 72; // placeholder
    player.spriteids[0] = 0;
    player.spriteids[1] = 1;
    set_sprite_tile(0,0);
    set_sprite_tile(1,1);
}

// Function to move n x n tile game objects
void move_game_object(struct GameObject* obj, UINT8 x, UINT8 y)
{
    // top left / only sprite
    move_sprite(obj->spriteids[0], x, y);
    // FIXME: add constant size offset for sprites
    
    //bottom left sprite / top sprite of 16x8 sprites
    if (obj->height > 8)
    {
        move_sprite(obj->spriteids[1], x, y + tileSize);
    }

    // top right sprite
    if (obj->width > 8)
    {
        move_sprite(obj->spriteids[2], x + tileSize, y);
    }

    // bottom right sprite
    if (obj->width > 8 && obj->height > 8)
    {
        move_sprite(obj->spriteids[3], x + tileSize, y + tileSize);
    }
}

// function to scroll n x n tile game objects
void scroll_game_object(struct GameObject* obj, INT8 movex, INT8 movey)
{
    while(movex != 0 && movey != 0)
    {
        if(movex != 0){
            // top left / only sprite
            scroll_sprite(obj->spriteids[0], movex < 0 ? -1 : 1, 0);
            
            //bottom left sprite / top sprite of 16x8 sprites
            if (obj->height > 8)
            {
                scroll_sprite(obj->spriteids[1], movex < 0 ? -1 : 1, 0);
            }

            // top right sprite
            if (obj->width > 8)
            {
                scroll_sprite(obj->spriteids[2], movex < 0 ? -1 : 1, 0);
            }

            // bottom right sprite
            if (obj->width > 8 && obj->height > 8)
            {
                scroll_sprite(obj->spriteids[3], movex < 0 ? -1 : 1, 0);
            }

            movex += (movex < 0 ? 1 : -1);
            wait_vbl_done();
        }

        if(movey != 0){
                    // top left / only sprite
            scroll_sprite(obj->spriteids[0], 0, movex < 0 ? -1 : 1);
            
            //bottom left sprite / top sprite of 16x8 sprites
            if (obj->height > 8)
            {
                scroll_sprite(obj->spriteids[1], 0, movex < 0 ? -1 : 1);
            }

            // top right sprite
            if (obj->width > 8)
            {
                scroll_sprite(obj->spriteids[2], 0, movex < 0 ? -1 : 1);
            }

            // bottom right sprite
            if (obj->width > 8 && obj->height > 8)
            {
                scroll_sprite(obj->spriteids[3], 0, movex < 0 ? -1 : 1);
            }

            movey += (movey < 0 ? 1 : -1);
            wait_vbl_done();
        }
    }
}

// Function to setup game
// to be called at the beginning of main
// sets up bg, turns on display, etc.
void setup_game()
{
    DISPLAY_ON;
    SHOW_SPRITES;
    setup_player();
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