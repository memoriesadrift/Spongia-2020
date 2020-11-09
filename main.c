// TODO: Sprite flip

// Generic Includes
#include <gb/gb.h>

// Graphics
// Sprites
#include "gfx/playerSprites.c"
//#include "gfx/Monster1.c"
// Tiles
#include "gfx/FantasyTileset.c"
#include "gfx/MotherboardTileset.c"
// Maps
#include "gfx/MapLevel1_1.c"
#include "gfx/MapLevel1_2.c"
#include "gfx/MapLevel2_1.c"
#include "gfx/MapLevel2_2.c"
#include "gfx/MapLevel3_1.c"
#include "gfx/MapLevel3_2.c"
#include "gfx/MapLevel5_20x18.c"
#include "gfx/MapLevel6_32x18.c"

// Engine related variables
BYTE gameRunning;
const UINT8 tileSize = 8;
UBYTE advanceAnimation; // player animation is 50% slower than game ticks, at the moment so we have to advance the animation every other tick

UINT8 i8; // for loop variable for reusable code - less memory needed to be allocated
UINT8 level; // tracks which level the player is in, for map loading and game flow

// Map variables
char* currentMap;
char* currentTileSet;
UINT8 currentCollisionTileCutoff;
UINT8 currentMapWidth;
UINT8 currentMapHeight;
UINT16 xOffset;
UINT16 yOffset;
BOOLEAN loaded;
BOOLEAN facing; // 0 right 1 left

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
};

// Player vars
struct GameObject player;
BYTE facing;
BYTE airborne;
INT8 gravity;
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
    // FIXME: this needs reworking with new sprite numbers and flipping doesnt work...
    player.animationType = type;
    // choose appropriate sprite id based on animation type
    switch (player.animationType)
        {
        case 1:
            // idle right
            player.spritenos[0] = 0;
            player.spritenos[1] = 1;
            break;
        case 2:
            // walk left
            if(!facing)
            {
                facing = 1;
                set_sprite_prop(player.spritenos[0], S_FLIPX);
                set_sprite_prop(player.spritenos[1], S_FLIPX);
            }
            player.spritenos[0] = 12;
            player.spritenos[1] = 13;

            break;
        case 3:
            // walk right
            if(facing)
            {
                facing = 0;
                set_sprite_prop(player.spritenos[0], S_FLIPX);
                set_sprite_prop(player.spritenos[1], S_FLIPX);
            }
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

// generic function for loading maps
void load_map(UINT8 mapId)
{
    // TODO: Implement all maps
    switch (mapId)
    {
    case 11:
        // level 1-1
        scroll_bkg(-xOffset,0);
        xOffset = 0;
        set_bkg_data(0, 46, FantasyTileset);
        set_bkg_tiles(0, 0, 40, 18, MapLevel1_1);
        currentMap = MapLevel1_1;
        currentTileSet = FantasyTileset;
        currentCollisionTileCutoff = 7;
        currentMapHeight = MapLevel1_1Height;
        currentMapWidth = MapLevel1_1Width;
        break;
    case 12:
        // level 1-2
        scroll_bkg(-xOffset,0);
        xOffset = 0;
        set_bkg_data(0, 46, FantasyTileset);
        set_bkg_tiles(0, 0, 40, 18, MapLevel1_2);
        currentMap = MapLevel1_2;
        currentTileSet = FantasyTileset;
        currentCollisionTileCutoff = 7;
        currentMapHeight = MapLevel1_2Height;
        currentMapWidth = MapLevel1_2Width;
        break;
    case 21:
        // level 21
        scroll_bkg(-xOffset,0);
        xOffset = 0;
        set_bkg_data(0, 49, MotherboardTileset);
        set_bkg_tiles(0, 0, 40, 18, MapLevel2_1);
        currentMap = MapLevel2_1;
        currentTileSet = MotherboardTileset;
        currentCollisionTileCutoff = 10;
        currentMapHeight = MapLevel2_1Height;
        currentMapWidth = MapLevel2_1Width;
        break;
    case 22:
        // level 21
        scroll_bkg(-xOffset,0);
        xOffset = 0;
        set_bkg_data(0, 49, MotherboardTileset);
        set_bkg_tiles(0, 0, 40, 18, MapLevel2_2);
        currentMap = MapLevel2_2;
        currentTileSet = MotherboardTileset;
        currentCollisionTileCutoff = 10;
        currentMapHeight = MapLevel2_2Height;
        currentMapWidth = MapLevel2_2Width;
        break;
    case 31:
        // level 21
        scroll_bkg(-xOffset,0);
        xOffset = 0;
        set_bkg_data(0, 49, MotherboardTileset);
        set_bkg_tiles(0, 0, 40, 18, MapLevel3_1);
        currentMap = MapLevel3_1;
        currentTileSet = MotherboardTileset;
        currentCollisionTileCutoff = 10;
        currentMapHeight = MapLevel3_1Height;
        currentMapWidth = MapLevel3_1Width;
        break;
    case 32:
        // level 21
        scroll_bkg(-xOffset,0);
        xOffset = 0;
        set_bkg_data(0, 49, MotherboardTileset);
        set_bkg_tiles(0, 0, 40, 18, MapLevel3_2);
        currentMap = MapLevel3_2;
        currentTileSet = MotherboardTileset;
        currentCollisionTileCutoff = 10;
        currentMapHeight = MapLevel3_2Height;
        currentMapWidth = MapLevel3_2Width;
        break;
    case 5:
        // level 5
        scroll_bkg(-xOffset,0);
        xOffset = 0;
        set_bkg_data(0, 49, MotherboardTileset);
        set_bkg_tiles(0, 0, 20, 18, MapLevel5_20x18);
        currentMap = MapLevel5_20x18;
        currentTileSet = MotherboardTileset;
        currentCollisionTileCutoff = 10;
        currentMapHeight = MapLevel5_20x18Height;
        currentMapWidth = MapLevel5_20x18Width;
        break;
    case 6:
        // level 6
        scroll_bkg(-xOffset,0);
        xOffset = 0;
        set_bkg_data(0, 49, FantasyTileset);
        set_bkg_tiles(0, 0, 32, 18, MapLevel6_32x18);
        currentMap = MapLevel6_32x18;
        currentTileSet = FantasyTileset;
        currentCollisionTileCutoff = 7;
        currentMapHeight = MapLevel6_32x18Height;
        currentMapWidth = MapLevel6_32x18Width;
        break;
    default:
        break;
    }

    loaded = FALSE;
}

// Function to change player offset
void set_player_offset(UINT8 x, UINT8 y)
{
    xOffset = x;
    yOffset = y;
}

// Function to add to player ofset
void incr_player_offset(INT8 x, INT8 y)
{
    xOffset += x;
    yOffset += y;
}

// Get offset in pixels
UINT8 get_x_offset()
{
    return xOffset;
}

UINT8 get_y_offset()
{
    return yOffset;
}

// function for changing maps, with cool fade effect
void change_map(UINT8 mapId)
{
    NR10_REG = 0x75;
    NR11_REG = 0xC7;
    NR12_REG = 0x50;
    NR13_REG = 0x83;
    NR14_REG = 0xC6;
    fadeout(5);
    load_map(mapId);
    fadein(5);
}

UINT8 get_tile_x(UINT8 x){
    return (x-8u+xOffset)/8u;
}

UINT8 get_tile_y(UINT8 y){
    return (y-16u+yOffset)/8u;
}

BOOLEAN has_collision(UINT8 tile_x, UINT8 tile_y){
    UINT16 tileindexTL = currentMapWidth * tile_y + tile_x;
    if ((UBYTE) currentMap[tileindexTL] < currentCollisionTileCutoff)
        return TRUE;

    return FALSE;
}

//TODO: Michal: Fix upside down gravity, test on levels 2_1 and 2_2, commands: change_map(21) and 22
// Function for falling
void fall()
{
    if(fall_counter != 3) { //don't apply gravity every third frame. essentialy making it 2/3*g
        currentSpeedY = currentSpeedY + gravity/2;
        ++fall_counter;
    } else
        fall_counter = 0;
        

    if (currentSpeedY < -7 * (-gravity/3))
        currentSpeedY = -7 * (-gravity/3);

    player.y = player.y - currentSpeedY;

    //collision down
    if(has_collision(get_tile_x(player.x), get_tile_y(player.y)+2u) || has_collision(get_tile_x(player.x)+1u, get_tile_y(player.y)+2u)){ 
        player.y = (get_tile_y(player.y)+2u)*8u - 16u + 16u;//last 16u is for coordinate offset
        airborne = 0u;
        currentSpeedY = 0u;
        fall_counter = 0u;
    }


    //collision up
    if(has_collision(get_tile_x(player.x), get_tile_y(player.y)) || has_collision(get_tile_x(player.x)+1u, get_tile_y(player.y))){ 
        player.y = get_tile_y(player.y)*8u +8u + 16u;//last 16u is for coordinate offset
        if(currentSpeedY < 0)
            currentSpeedY = 0;
    }
}

// Jump function
void jump()
{
    if(airborne==0)
    {
        airborne=1;
        currentSpeedY = 7 * (-gravity/3);
        NR10_REG = 0x44;
        NR11_REG = 0x81;
        NR12_REG = 0x41;
        NR13_REG = 0x83;
        NR14_REG = 0xC6;
    }
}


// Setup Functions

void setup_player()
{
    set_sprite_data(0, 31, playerSprites);
    player.height = 16;
    player.width = 8;
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
    load_map(11);   
    gravity = -3;
    gameRunning = 1;
    advanceAnimation = 0;
    fall_counter = 0;
    loaded = FALSE;

    NR52_REG = 0x80;
    NR50_REG = 0x77;
    NR51_REG = 0xFF;
    SHOW_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;
}

int main()
{
    setup_game();
    move_player(8, 96);
    while(gameRunning)
    {
        UINT16 oldx = player.x;
        UINT16 oldy = player.y;

        // joypad controls
        UBYTE j = joypad();

        if(j & J_A && !airborne){
            jump();
        }
        if(j & J_LEFT){
            player.x -= 1;
            if(has_collision(get_tile_x(player.x), get_tile_y(player.y)) || has_collision(get_tile_x(player.x), get_tile_y(player.y)+1u)){
                player.x += 1;
            }
            if (facing != -1)
            {
                facing = -1;
                change_player_animation(2);
            }
        }
        if(j & J_RIGHT) {
            player.x += 1;
            if(has_collision(get_tile_x(player.x)+1u, get_tile_y(player.y)) || has_collision(get_tile_x(player.x)+1u, get_tile_y(player.y)+1u)){
                player.x -=1;
            }
            if (facing != 1)
            {
                facing = 1;
                change_player_animation(3);
            }
        }
        
        fall();

        //Check if need to scroll
        if (!(player.x+xOffset < SCREENWIDTH/2 || player.x+xOffset > currentMapWidth*8 - SCREENWIDTH/2)){ 
            scroll_bkg((INT16)player.x - oldx, 0);
            xOffset+=(INT16)player.x - oldx;
            player.x = SCREENWIDTH/2;
        }

        if (!(player.y+yOffset < SCREENHEIGHT/2 + 8 || player.y+yOffset > currentMapHeight*8 - (SCREENHEIGHT/2 + 8))){
            scroll_bkg(0, (INT16)player.y - oldy);
            yOffset+=(INT16)player.y - oldy;
            player.y = SCREENHEIGHT/2 - 8;
        }

        move_player(player.x, player.y);
        
    
        //check if need to load different tiles
        if((player.x+xOffset)/8 >= 21 && !loaded){ //can be done more dynamicly to allow for bigger maps
            for(UINT8 i = 0; i < 18; ++i){
                set_bkg_tiles(0,i,8,1,currentMap+40*i + 32);
            }
            loaded = TRUE;
        }

        // Map Changing

        if(currentMap == MapLevel1_1 && player.x > 160 && player.y < 70)
        {
            change_map(12);
            move_player(8,24);
        }

        //if(currentMap == MapLevel1_2 && player.x+xOffset > 15+xOffset && player.y == 14+yOffset) // FIXME: Michal: fix map change here
        // We have to basically create a sprite at this location that when touched will move the player back and then change its sprite to the gameboy
        // if you get it to work when the player walks into the spot shown in the discord picture of th emap in #fileuploads i will do the sprite work
        // 
        if(j & J_START) // Debug
        {
            /*
            //TODO: Michal: Fix upside down gravity
            change_map(21);
            move_player(8,36);
            gravity = 3;
            */
           // then delete this and uncomment the top commands
           change_map(31);
           move_player(8,136);
        }
        //TODO: Sam, tweak numbers if necessary
        if (currentMap == MapLevel2_1 && player.x > 160 && player.y > 110)
        {
            change_map(22);
            move_player(8, 136);
        }

        if (currentMap == MapLevel2_2 && player.x == 22 * 8 +8 && player.y == 4 * 8 + 16) // TODO: Sam, rework to sprite collision, same as 1_2
        {
            gravity = -3;
        }
        if (currentMap == MapLevel2_2 && player.x > 160 && player.y < 70)
        {
            change_map(31);
            move_player(8,136);
        }
        if (currentMap == MapLevel3_1 && player.x > 160 && player.y < 70)
        {
            change_map(32);
            move_player(8, player.y);
        }
        if (currentMap == MapLevel3_2 && player.x > 160 && player.y < 50)
        {
            // TODO: maybe add something here? currently the game is won by pressing the A button
            change_map(5);
            move_player(48,114);
        }
        if(currentMap == MapLevel5_20x18 && (j & J_A))
        {
            change_map(6);
            move_player(15 * 8 +8, 13 * 8 + 16);
            // Game is over at this point
        }
        

        // Animation
        if(advanceAnimation == 2)
        {
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