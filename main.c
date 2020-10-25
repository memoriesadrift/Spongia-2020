// Generic Includes
#include <gb/gb.h>

// Graphics
#include "gfx/playerSprites.c"
#include "gfx/TestTileset.c"
#include "gfx/TestMap.c"
// TODO: Split generic functions into separate header files eventually?

// Engine related variables
BYTE airborne;
INT8 gravity; // FIXME: const?
INT16 currentSpeedY;
BYTE gameRunning;
const UINT8 tileSize = 8; // FIXME: doesn't work as #define TILESIZE 8 in move_sprite() function call - why?
UBYTE advanceAnimation; // player animation is 50% slower than game ticks, at the moment so we have to advance the animation every other tick

// Bigger sprite supporting struct
struct GameObject 
{
    UBYTE spriteids[4];
    UINT8 spritenos[4];
    UINT8 x;
    UINT8 y;
    UINT8 width;
    UINT8 height;
    UINT8 animationLength;
    UINT8 animationStep;
    UINT8 animationType; // 0 - idle, 1 - idle left, 2 - idle right, 3 - walk left, 4 - walk right
};
BYTE facing;

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

// Function to move n x n tile game objects
void move_game_object(struct GameObject* obj, UINT8 x, UINT8 y)
{
    // set new location in GameObject
    obj->x = x;
    obj->y = y;
    // top left / only sprite
    move_sprite(obj->spriteids[0], obj->x, obj->y);
    
    //bottom left sprite / top sprite of 16x8 sprites
    if (obj->height > 8)
    {
        move_sprite(obj->spriteids[1], obj->x, obj->y + tileSize);
    }

    // top right sprite
    if (obj->width > 8)
    {
        move_sprite(obj->spriteids[2], obj->x + tileSize, obj->y);
    }

    // bottom right sprite
    if (obj->width > 8 && obj->height > 8)
    {
        move_sprite(obj->spriteids[3], obj->x + tileSize, obj->y + tileSize);
    }
}

// function to scroll n x n tile game objects
void scroll_game_object(struct GameObject* obj, INT8 movex, INT8 movey)
{
    // set new location in GameObject
    obj->x += movex;
    obj->y += movey;
    // FIXME: Maybe make these update after every progress in the loop below?

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

// Player movement function
void move_player(UINT8 x, UINT8 y)
{
    move_game_object(&player, x, y);
}

// Player scroll function
void scroll_player(INT8 x, INT8 y)
{
    scroll_game_object(&player, x, y);
}

// function to change animation type dynamically
void change_player_animation(UINT8 type)
{
    player.animationType = type;
    // choose appropriate sprite id based on animation type
    switch (player.animationType)
        {
        case 0:
            player.spritenos[0] = 0;
            player.spritenos[1] = 1;
            break;
        case 1:
            player.spritenos[0] = 6;
            player.spritenos[1] = 7;
            break;
        case 2:
            break;
        case 3:
            player.spritenos[0] = 18;
            player.spritenos[1] = 19;
            break;
        case 4:
            player.spritenos[0] = 12;
            player.spritenos[1] = 13;
            break;
        default:
            break;
        }
}

// player animate function
void advance_player_animation()
{
    ++player.animationStep;
    // Reset sprite to the starting point
    if (player.animationStep > player.animationLength)
    {
        change_player_animation(player.animationType);
        player.animationStep = 0;
    } else
    {
        player.spritenos[0] += (UINT8)2;
        player.spritenos[1] += (UINT8)2;
    }
    
    set_sprite_tile(player.spriteids[0], player.spritenos[0]);
    set_sprite_tile(player.spriteids[1], player.spritenos[1]);
}

void setup_player()
{
    set_sprite_data(0, 30, playerSprites);
    player.height = 16;
    player.width = 8;
    player.x = 80; // placeholder
    player.y = 72; // placeholder
    player.animationLength = 2;
    player.animationType = 0; // idle
    player.animationStep = 0;
    player.spriteids[0] = 0;
    player.spriteids[1] = 1;
    player.spritenos[0] = 0;
    player.spritenos[1] = 1;
    set_sprite_tile(player.spriteids[0], player.spritenos[0]);
    set_sprite_tile(player.spriteids[1], player.spritenos[1]);
}

// Function to setup game
// to be called at the beginning of main
// sets up bg, turns on display, etc.
void setup_game()
{
    setup_player();
    SHOW_SPRITES;
    DISPLAY_ON;
    gameRunning = 1;
    advanceAnimation = 0;
}


// Jump function
void jump()
{

}

int main()
{
    setup_game();
    move_player(player.x, player.y);

    while(gameRunning)
    {
        // this check allows us to use a switch for the movement (prettier) but also enables
        // moving while in the air, which is nice.
        if (airborne)
        {
            // fall
        }

        // joypad controls       
        switch (joypad())
        {
        case J_A:
            if (!airborne)
                jump();
            break;
        case J_LEFT:
            player.x -= 2;
            if (facing != -1)
            {
                facing = -1;
                change_player_animation(3);
            }
            break;
        case J_RIGHT:
            player.x += 2;
            if (facing != 1)
            {
                facing = 1;
                change_player_animation(4);
            }
            break;
        default:
            break;
        }

        move_player(player.x, player.y);

        // Animation
        if(advanceAnimation)
        {
            advanceAnimation = 0;
            advance_player_animation();
        } else
        {
            advanceAnimation = 1;
        }

        // end of game tick, delay
        efficient_wait(5);
    }

    return 0;
}