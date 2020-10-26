// FIXME: x-axis collision, currently, if the player walks into a block from the side, it won't count as a collision
// TODO: smooth out falling, it happens too fast, possibly due to fall() being called twice? see comment in main()
// TODO: add jumping animation!
// TODO: add "standing facing left/right" animation - detection for if player is currently moving or not

// Generic Includes
#include <gb/gb.h>

// Graphics
#include "gfx/playerSprites.c"
#include "gfx/TestTileset.c"
#include "gfx/TestMap.c"
// TODO: Split generic functions into separate header files eventually?

// Engine related variables
BYTE gameRunning;
const UINT8 tileSize = 8; // FIXME: doesn't work as #define TILESIZE 8 in move_sprite() function call - why?
UBYTE advanceAnimation; // player animation is 50% slower than game ticks, at the moment so we have to advance the animation every other tick
char* currentMap;
char* currentTileSet;
UINT8 currentCollisionTileCutoff;

// Bigger sprite supporting struct
struct GameObject 
{
    UBYTE spriteids[4];
    UINT8 spritenos[4];
    UINT8 x;
    UINT8 y;
    UINT8 width;
    UINT8 height;
    UINT8 animationLength; // 1 - 2 frames, 2 - 3 frames
    UINT8 animationStep;
    UINT8 animationType; //0 - idle left, 1 - idle right, 2 - walk left, 3 - walk right, 4 - jump left, 5 - jump right 
    // TODO: introduce constants for player animation types and lengths
};
BYTE facing;
BYTE airborne;
INT8 gravity; // FIXME: const?
INT16 currentSpeedY;

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

// A recursive binary search function. It returns 
// location of x in given array arr[l..r] is present, 
// otherwise -1 
BYTE binary_search(unsigned char* arr, UINT8 l, UINT8 r, unsigned char x) 
{ 
    if (r >= l) { 
        UINT8 mid = l + (r - l) / 2; 
  
        // If the element is present at the middle 
        // itself 
        if (arr[mid] == x) 
            return mid; 
  
        // If element is smaller than mid, then 
        // it can only be present in left subarray 
        if (arr[mid] > x) 
            return binary_search(arr, l, mid - 1, x); 
  
        // Else the element can only be present 
        // in right subarray 
        return binary_search(arr, mid + 1, r, x); 
    } 
  
    // We reach here when element is not 
    // present in array 
    return -1; 
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
            // idle left
            player.spritenos[0] = 6;
            player.spritenos[1] = 7;
            break;
        case 1:
            // idle right
            player.spritenos[0] = 0;
            player.spritenos[1] = 1;
            break;
        case 2:
            // walk left
            player.spritenos[0] = 18;
            player.spritenos[1] = 19;
            break;
        case 3:
            // walk right
            player.spritenos[0] = 12;
            player.spritenos[1] = 13;
            break;
        case 4:
            // jump left
            // warning, this animation is 2 frames not 3!
            player.spritenos[0] = 28;
            player.spritenos[1] = 29;
        case 5:
            // jump right
            // warning, this animation is 2 frames not 3!
            player.spritenos[0] = 24;
            player.spritenos[1] = 25;
        default:
            break;
        }
    // change the player's animation length according to what's going on
    if (player.animationType > 3)
    {
        player.animationLength = 1;
    } else
    {
        player.animationLength = 2;
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
    set_sprite_data(0, 31, playerSprites);
    player.height = 16;
    player.width = 8;
    player.x = 8; // placeholder
    player.y = 96; // placeholder
    player.animationLength = 2;
    player.animationType = 0; // idle
    player.animationStep = 0;
    player.spriteids[0] = 0;
    player.spriteids[1] = 1;
    player.spritenos[0] = 0;
    player.spritenos[1] = 1;
    set_sprite_tile(player.spriteids[0], player.spritenos[0]);
    set_sprite_tile(player.spriteids[1], player.spritenos[1]);
    airborne = 0;
}

// Function to setup game
// to be called at the beginning of main
// sets up bg, turns on display, etc.
void setup_game()
{
    setup_player();
    set_bkg_data(0, 46, TestTileset);
    set_bkg_tiles(0, 0, 20, 18, TestMap);
    currentMap = TestMap;
    currentTileSet = TestTileset;
    currentCollisionTileCutoff = 7;
    SHOW_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;
    gravity = -2;
    gameRunning = 1;
    advanceAnimation = 0;
}

// collision detection
// WARNING: Sprite is tracked by its top left corner, this function checks collisions with the BOTTOM LEFT
INT8 detect_collision(UINT8 newx, UINT8 newy)
{
    UINT16 indexTLx, indexTLy, tileindexTL;

    indexTLx = (newx - 8) / 8;
    indexTLy = (newy) / 8; // ney-16 for the TOP LEFT of sprite
    tileindexTL = 20 * indexTLy + indexTLx;

    if ((UBYTE) currentMap[tileindexTL] < COLLISION_CUTOFF_TEST_MAP){
        airborne = 0;
        return (indexTLy*8u); // -16u for the TOP LEFT of sprite
    }

    return newy;
}

// Function for falling
void fall()
{
    currentSpeedY = currentSpeedY + gravity;
    if (currentSpeedY < -7) currentSpeedY = -7;

    player.y = player.y - currentSpeedY;
    player.y = detect_collision(player.x, player.y);
}

// Jump function
void jump()
{
    if(airborne==0)
    {
        airborne=1;
        currentSpeedY = 10;
    }

    fall();
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
        UBYTE j = joypad();

        if((j & J_A && !airborne) || airborne){ //FIXME: I am not sure I did this right!!!
            jump();
        }
        if(j & J_LEFT){
            player.x -= 1;
            if (facing != -1)
            {
                facing = -1;
                change_player_animation(2);
            }
        }
        if(j & J_RIGHT){
            player.x += 1;
            if (facing != 1)
            {
                facing = 1;
                change_player_animation(3);
            }
        }

        
        move_player(player.x, player.y);

        // Animation
        if(advanceAnimation == 2)
        {
            fall(); // fall called only every few game ticks, because falling and jumping was extremely fast otherwise
                    // this can probably be fixed some way and someone should do it
            advanceAnimation = 0;
            advance_player_animation();
        } else
        {
            ++advanceAnimation;
        }

        // end of game tick, delay
        efficient_wait(2);
    }

    return 0;
}