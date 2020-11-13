#include<string.h>
#include"toolbox.h"
#include"caveship.h"
#include"cave2.h"
#include"gameover.h"
#include"explosion.h"

#define height 16
#define width 32
#define caveship_velocity 2
#define bullet_velocity 3
#define max_bullets 19

typedef enum direction { UP, DOWN, LEFT, RIGHT } DIRECTION;

typedef struct point {
	int x;
	int y;
} point;

OBJ_ATTRS obj_buffer[128];

int x;
int y;

int bgx;
int bgy;

int bg_sbb_offset = 0;

int num_bullets;
int bullet_states[max_bullets];

point tile_index(int x, int y){
	point index;
	index.x = (bgx + x)/8;
	index.y = (bgy + y)/8;
	return index;
}

int se_index(int tx, int ty, int pitch)
{	
    int sbb= (ty/32)*(pitch/32) + (tx/32);
    return sbb*1024 + (ty%32)*32 + tx%32;
}

int isCaveshipCollision(DIRECTION direction){
	int i;

	if(direction == UP){
		for(i = 0;i <= 4; i++){
			if(se_mem[24 + bg_sbb_offset][se_index(tile_index(x+8*i-i/4, y-1).x, tile_index(x+8*i-i/4, y-1).y, 64)]) return 1;
		}
	}else if(direction == RIGHT){
		for(i = 0;i <= 2; i++){
			if(se_mem[24 + bg_sbb_offset][se_index(tile_index(x+width+1, y+8*i-i/2).x, tile_index(x+width+1, y+8*i-i/2).y, 64)]) return 1;
		}
	}else if(direction == DOWN){
		for(i = 0;i <= 4; i++){
			if(se_mem[24 + bg_sbb_offset][se_index(tile_index(x+8*i-i/4, y+height+1).x, tile_index(x+8*i-i/4, y+height+1).y, 64)]) return 1;
		}
	}else if(direction == LEFT){
		for(i = 0;i <= 2; i++){
			if(se_mem[24 + bg_sbb_offset][se_index(tile_index(x-1, y+8*i-i/2).x, tile_index(x-1, y+8*i-i/2).y, 64)]) return 1;
		}
	}
	
	return 0;
}

int isBulletCollision(volatile OBJ_ATTRS bullet){
	if(se_mem[24 + bg_sbb_offset][se_index(tile_index(get_obj_x(bullet), get_obj_y(bullet)).x, tile_index(get_obj_x(bullet), get_obj_y(bullet)).y, 64)]) return 1;
	else if(se_mem[24 + bg_sbb_offset][se_index(tile_index(get_obj_x(bullet)+8, get_obj_y(bullet)).x, tile_index(get_obj_x(bullet)+8, get_obj_y(bullet)).y, 64)]) return 1;
	else if(se_mem[24 + bg_sbb_offset][se_index(tile_index(get_obj_x(bullet), get_obj_y(bullet)+8).x, tile_index(get_obj_x(bullet), get_obj_y(bullet)+8).y, 64)]) return 1;
	else if(se_mem[24 + bg_sbb_offset][se_index(tile_index(get_obj_x(bullet)+8, get_obj_y(bullet)+8).x, tile_index(get_obj_x(bullet)+8, get_obj_y(bullet)+8).y, 64)]) return 1;
	else return 0;
}

void gameover()
{
	REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

	memcpy(vid_mem, gameover1Bitmap, gameoverBitmapLen);
	memcpy(vid_mem_back, gameover2Bitmap, gameoverBitmapLen);
	memcpy(pal_mem, gameoverPal, gameoverPalLen);

	int cont = 0;

	int framecounter = 0;

	while(!cont){
		vsync();
		key_poll();

		if(KEY_PRESSED(KEY_A)) cont = 1;

		if(++framecounter == 40){
			REG_DISPCNT ^= DCNT_PAGE;
			framecounter = 0;
		};
		
	}
}

void game()
{
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;

	oam_init(obj_buffer, 128);

	memcpy(&tile_mem[4][0], caveship1Bitmap, caveshipBitmapLen);
	memcpy(&tile_mem[4][8], caveship2Bitmap, caveshipBitmapLen);
	memcpy(obj_pal_mem, caveshipPal, caveshipPalLen);

	memcpy(&tile_mem[4][16], bulletsBitmap, bulletsBitmapLen);
	memcpy(&obj_pal_mem[16], bulletsPal, bulletsPalLen);

	memcpy(&tile_mem[4][20], explosionBitmap, explosionBitmapLen);
	memcpy(&obj_pal_mem[32], explosionPal, explosionPalLen);

	memcpy(tile_mem, cave2Tiles, cave2TilesLen);
	memcpy(pal_mem, cave2Pal, cave2PalLen);
	memcpy(&se_mem[16][0], cave2Map, cave2MapLen);
	memcpy(&se_mem[20][0], cave2Map, cave2MapLen/4); //for looping
	memcpy(&se_mem[24][0], cave2colMap, cave2colMapLen);
	memcpy(&se_mem[28][0], cave2colMap, cave2colMapLen/4); //for looping

	REG_BGCNT[2] = BG_CBB(0) | BG_SBB(16) | BG_REG_64x32 | BG_PRIO(1);

	volatile OBJ_ATTRS* caveship = &obj_buffer[0];
	set_obj_attrs(caveship, ATTR0_WIDE, ATTR1_SIZE_2, ATTR2_PRIO(1));

	volatile OBJ_ATTRS* bullets = &obj_buffer[1];

	volatile OBJ_ATTRS* explosion = &obj_buffer[20];

	x = 20;
	y = 64;

	bgx = 0;
	bgy = 0;

	bg_sbb_offset = 0;

	num_bullets = 0;

	set_obj_pos(caveship, x, y);

	set_bg_offset(2, bgx, bgy);

	oam_copy(oam_mem, obj_buffer, 1);

	int framecounter = 0;

	int bg_increment = 1;

	int dead = 0;

	while(1){

		vsync();
		key_poll();

		//move

		if(KEY_DOWN_NOW(KEY_UP) && !isCaveshipCollision(UP) && !dead) y -= caveship_velocity;
		if(KEY_DOWN_NOW(KEY_DOWN) && !isCaveshipCollision(DOWN) && !dead) y += caveship_velocity;
		if(KEY_DOWN_NOW(KEY_RIGHT) && !isCaveshipCollision(RIGHT) && !dead) x += caveship_velocity;
		if(KEY_DOWN_NOW(KEY_LEFT) && !isCaveshipCollision(LEFT) && !dead) x -= caveship_velocity;

		x = clamp(x, 0, SCREEN_WIDTH - width);
		y = clamp(y, 0, SCREEN_HEIGHT - height);

		//shooting and bullets

		for(int i = 0; i<num_bullets;i++){
			if(get_obj_x(bullets[i]) > 240 || isBulletCollision(bullets[i])){
				bullet_states[i] = 1;
			}

			if(!bullet_states[i]){
				set_obj_x(&bullets[i], get_obj_x(bullets[i]) + bullet_velocity);
			}
		}

		if(KEY_PRESSED(KEY_A) && num_bullets < max_bullets){
			set_obj_attrs(&bullets[num_bullets], 0, 0, ATTR2_PALBANK(1) | ATTR2_ID(16));
			set_obj_pos(&bullets[num_bullets], x + width, y + 4);
			bullet_states[num_bullets] = 0;
			num_bullets++;
		}

		//scroll fix

		if(isCaveshipCollision(RIGHT)) x -= bg_increment;
		
		for(int i = 0; i < num_bullets; i++){
			if(bullet_states[i]) set_obj_x(&bullets[i], get_obj_x(bullets[i]) - bg_increment);
		}

		//update position

		set_obj_pos(caveship, x, y);

		//bg scroll

		if((bgx += bg_increment) >= 256){
			bg_sbb_offset++;
			REG_BGCNT[2] = BG_CBB(0) | BG_SBB(16 + bg_sbb_offset) | BG_REG_64x32 | BG_PRIO(1);
			bgx = 0;
		}

		set_bg_offset(2, bgx, bgy);

		//bg loop

		if(bg_sbb_offset == 4){
			bg_sbb_offset = 0;
			REG_BGCNT[2] = BG_CBB(0) | BG_SBB(16) | BG_REG_64x32 | BG_PRIO(1);
		}

		//dead

		if(x < 0){
			dead = 1;
			set_obj_attrs(explosion, 0, ATTR1_SIZE_1, ATTR2_PALBANK(2) | ATTR2_ID(20));
			set_obj_pos(explosion, x, y);
			bg_increment = 0;
		}

		//animation

		if(framecounter%15 == 0){
			//caveship anim
			if(get_obj_basetile(*caveship) == 8) set_obj_basetile(caveship, 0);
			else set_obj_basetile(caveship, get_obj_basetile(*caveship) + 8);
		}

		if(framecounter%2 == 0){
			//bullet crash anim
			for(int i = 0; i<num_bullets;i++){
				if(bullet_states[i]){
					if(get_obj_basetile(bullets[i]) == 19){
						for(int j = i;j<num_bullets-1;j++){
							bullets[j] = bullets[j+1];
							bullet_states[j] = bullet_states[j+1];
						}
						bullets[num_bullets-1].attr2 = 0;
						bullets[num_bullets-1].attr0 = ATTR0_HIDE;

						num_bullets--;
					}else {
						set_obj_basetile(&bullets[i], get_obj_basetile(bullets[i]) + 1);
					}			
				}
			}
		}

		if(dead){
			//explosion anim
			if(get_obj_basetile(*explosion) == 260){
				return;
			}
			set_obj_basetile(explosion, get_obj_basetile(*explosion) + 4);
		}

		if(++framecounter == 30) framecounter = 0;

		//copy to oam

		oam_copy(oam_mem, obj_buffer, 21);
	}
}

int main()
{
	while(1){
		game();

		gameover();
	}

    return 0;
}


