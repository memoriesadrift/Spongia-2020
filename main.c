// FIXME: implement x-axis collision, currently, if the player walks into a block from the side, it won't count as a collision
// TODO: fix jumping animation! dont want it to loop while jumping as it is now
// TODO: add "standing facing left/right" animation - detection for if player is currently moving or not

// Generic Includes
#include <gb/gb.h>

// Graphics
#include "gfx/playerSprites.c"
#include "gfx/TestTileset.c"
#include "gfx/TestMap.c"
#include "gfx/Level1Map.c"
// TODO: Split generic functions into separate header files eventually?

// Engine related variables
BYTE gameRunning;
const UINT8 tileSize = 8; // FIXME: doesn't work as #define TILESIZE 8 in move_sprite() function call - why?
UBYTE advanceAnimation; // player animation is 50% slower than game ticks, at the moment so we have to advance the animation every other tick

UINT8 i8; // for loop variable for reusable code - less memory needed to be allocated
UINT8 j8; // same as above, but for nested for loops
UINT8 level; // tracks which level the player is in, for map loading and game flow

// Map variables
char* currentMap;
char* currentTileSet;
UINT8 currentCollisionTileCutoff;
UINT8 currentMapWidth;
UINT8 currentMapHeight;
UINT8 xOffset;
UINT8 yOffset;

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

// Player vars
struct GameObject player;
BYTE facing;
BYTE airborne;
INT8 gravity; // FIXME: const?
INT16 currentSpeedY;
UBYTE fall_counter;


//fucntion declarations



// CPU Efficient waiting function to be used
// when waiting is needed
void efficient_wait(UINT8 loops)
{
    UINT8 wait = 0;
    for (wait; wait < loops; ++wait)
    {
        wait_vbl_done();
    }
}

// Fades out the screen
void fadeout(UINT8 fadeRate){
    for (i8 = 0; i8 < 4; ++i8){
        switch(i8){
            case 0:
                BGP_REG = 0xE4;
                break;
            case 1:
                BGP_REG = 0xF9;
                break;
            case 2:
                BGP_REG = 0xFE;
                break;
            case 3:
                BGP_REG = 0xFF;
                break;
        }
        efficient_wait(fadeRate);
    }
}

// Fades in the screen
void fadein(UINT8 fadeRate){
    for (i8 = 0; i8 < 4; ++i8){
        switch(i8){
            case 0:
                BGP_REG = 0xFF;
                break;
            case 1:
                BGP_REG = 0xFE;
                break;
            case 2:
                BGP_REG = 0xF9;
                break;
            case 3:
                BGP_REG = 0xE4;
                break;
        }
        efficient_wait(fadeRate);
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

// generic function for loading maps
void load_map(UINT8 mapId)
{
    switch (mapId)
    {
    case 0:
        // test map
        set_bkg_data(0, 46, TestTileset);
        set_bkg_tiles(0, 0, 20, 18, TestMap);
        currentMap = TestMap;
        currentTileSet = TestTileset;
        currentCollisionTileCutoff = 7;
        currentMapHeight = TestMapHeight;
        currentMapWidth = TestMapWidth;
        break;
    case 1:
        // level 1
        set_bkg_data(0, 46, TestTileset);
        set_bkg_tiles(0, 0, 40, 18, Level1Map);
        currentMap = Level1Map;
        currentTileSet = TestTileset;
        currentCollisionTileCutoff = 7;
        currentMapHeight = Level1MapHeight;
        currentMapWidth = Level1MapWidth;
    default:
        break;
    }
}

// Function to change player offset
void set_player_offset(UINT8 x, UINT8 y)
{
    xOffset = x/8;
    yOffset = y/8;
}

// Function to add to player ofset
void incr_player_offset(INT8 x, INT8 y)
{
    xOffset += x/8;
    yOffset += y/8;
}

// Get offset in pixels
UINT8 get_x_offset()
{
    return xOffset*8;
}

UINT8 get_y_offset()
{
    return yOffset*8;
}

// function for changing maps, with cool fade effect
void change_map(UINT8 mapId)
{
    fadeout(5);
    load_map(mapId);
    fadein(5);
}

UINT8 get_tile_x(UINT8 x){
    return (x-8u)/8u + xOffset;
}

UINT8 get_tile_y(UINT8 y){
    return (y-16u)/8u +yOffset;
}

//new collision block

BOOLEAN has_collision(UINT8 x, UINT8 y){
    UINT16 tileindexTL = currentMapWidth * get_tile_y(y) + get_tile_x(x);
    *((UINT8*)0xC0A0) = get_tile_x(x);
    *((UINT8*)0xC0A1) = get_tile_y(y);
    if ((UBYTE) currentMap[tileindexTL] < currentCollisionTileCutoff )

        return TRUE;

    return FALSE;

}

// Function for falling
void fall()
{
    if(fall_counter != 3) { //don't apply gravity every third frame. essentialy making it 2/3*g
        currentSpeedY = currentSpeedY + gravity/2;
        ++fall_counter;
    } else
        fall_counter = 0;
        

    if (currentSpeedY < -7)
        currentSpeedY = -7;

    player.y = player.y - currentSpeedY;
    //if(has_collision(player.x, player.y+16u)){ //collision down

    if(has_collision(player.x, player.y+16u) || has_collision(player.x+8u, player.y+16u)){
        player.y = get_tile_y(player.y+16)*8u - 16u + 16u;//last 16u is for coordinate offset
        airborne = 0;
        currentSpeedY = 0;
        fall_counter = 0;
    }


    if(has_collision(player.x, player.y-8u) || has_collision(player.x+8u, player.y-8u)){
        player.y = get_tile_y(player.y-8u)*8u +16u + 16u;//last 16u is for coordinate offset
        if(currentSpeedY < 0)
            currentSpeedY = 0;
    }
}

// Jump function
void jump()
{
    if(airborne==0)
    {
        if (facing = -1)
        {
            change_player_animation(4);
        }else
        {
            change_player_animation(5);
        }
        airborne=1;
        currentSpeedY = 7;
    }
}


// Setup Functions

void setup_player()
{
    set_sprite_data(0, 31, playerSprites);
    player.height = 16;
    player.width = 8;
    //player.x = 8; // placeholder
    //player.y = 96; // placeholder
    xOffset = 0;
    yOffset = 0;
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
    load_map(0);
    SHOW_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;
    gravity = -3;
    gameRunning = 1;
    advanceAnimation = 0;
    fall_counter = 0;
}

int main()
{
    setup_game();
    move_player(8, 96);

    while(gameRunning)
    {

        // joypad controls
        UBYTE j = joypad(); // FIXME: is there a reason for this?

        if(j & J_A && !airborne){
            jump();
        }
        if(j & J_LEFT){
            player.x -= 1;
            if(has_collision(player.x-8u, player.y) || has_collision(player.x-8u, player.y+8u)){
                player.x +=1;
            }
            if (facing != -1)
            {
                facing = -1;
                change_player_animation(2);
            }
        }
        if(j & J_RIGHT){
            player.x += 1;
            if(has_collision(player.x+8u, player.y) || has_collision(player.x+8u, player.y+8u)){
                player.x -=1;
            }
            if (facing != 1)
            {
                facing = 1;
                change_player_animation(3);
            }
        }
        
        fall();

        move_player(player.x, player.y);
        

        // Animation
        if(advanceAnimation == 2)
        {
            advanceAnimation = 0;
            advance_player_animation();
        } else
        {
            ++advanceAnimation;
        }

        // Reached right end of screen
        if (player.x > 168 && player.y > 48 && player.y < 88)
        {
            change_map(1);
            move_player(8,96);
        } else if (player.x > 154 && currentMap != TestMap)
        {
            scroll_bkg(8,0);
            move_player(player.x-8, player.y);
            incr_player_offset(8,0);
        }
        

        // end of game tick, delay
        efficient_wait(2);
    }

    return 0;
}