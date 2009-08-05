/* $Id$
 *
 * Copyright (c) 2008,2009 Bertrand Janin <tamentis@neopulsar.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef __WII__
#include <gctypes.h>
#endif

/* Main speed and flow control */
#define MAXFPS			30
#define TICK			10
#define NEXTLINE		30	// nb of seconds between new lines

/* Board constants */
#define BOARD_LEFT		175
#define BOARD_TOP		109
#define BOARD_WIDTH		9
#define BOARD_HEIGHT		10

/* A couple hard-coded sizes.. */
#define BSIZE			32
#define FONT0_HEIGHT		19
#define FONT1_HEIGHT		17
#define LVL_MAX_SIZE		4096
#define MAX_MOLES		8
#define MOLE_TRAIL		48

/* Speed relative to difficulty */
#define SPEED_NORMAL		1000
#define SPEED_LESS5K		800
#define SPEED_LESS10K		500
#define SPEED_LESS25K		200
#define SPEED_LESS50K		100
#define SPEED_MAX		50

/* Controls related */
#define JOYSTICK_DEAD_ZONE	16384

#ifdef __WII__
enum wii_buttons {
	WPAD_BUTTON_A,
	WPAD_BUTTON_B,
	WPAD_BUTTON_1,
	WPAD_BUTTON_2,
	WPAD_BUTTON_MINUS,
	WPAD_BUTTON_PLUS,
	WPAD_BUTTON_HOME
};
#endif

/* Block types */
enum btype {
	BLOCK_TYPE_TEE,		// 0
	BLOCK_TYPE_ELL,
	BLOCK_TYPE_JAY,
	BLOCK_TYPE_ZEE,
	BLOCK_TYPE_ESS,
	BLOCK_TYPE_SQUARE,	// 5
	BLOCK_TYPE_BAR,
	BLOCK_TYPE_ONE,
	BLOCK_TYPE_TWO,
	BLOCK_TYPE_THREE,
	BLOCK_TYPE_CORNER	// 10
};

/* Cube types */
enum ctype {
	CTYPE_EMPTY,		// 0
	CTYPE_ANGLE,
	CTYPE_TEE,
	CTYPE_FLAT,
	CTYPE_KNOB,
	CTYPE_ALL,		// 5
	CTYPE_BOMB,
	CTYPE_MEDIC,
	CTYPE_ROCK,
	CTYPE_MAX
};

/* Transition types */
enum ttype {
	TTYPE_NONE,
	TTYPE_SHUTTER_OPEN,
	TTYPE_PIXEL_OPEN
};

/* Plug types */
#define PLUG_NORTH		1
#define PLUG_EAST		2
#define PLUG_SOUTH		4
#define PLUG_WEST		8

/* Plug statuses */
#define PSTAT_CONNECTED		7
#define PSTAT_OPENED		1

/* Text alignment */
#define ALIGN_CENTER		-1

/* Difficulties */
enum {
	DIFF_EASIEST,
	DIFF_EASY,
	DIFF_MEDIUM,
	DIFF_HARD,
	DIFF_ULTRA,
	DIFF_LENGTH
};

/* Menu types */
enum mtype {
	MTYPE_NOP,		// keep going / don't do anything different
	MTYPE_QUIT,		// leave the game
	MTYPE_START,		// start scenario mode
	MTYPE_SUBMENU,		// change menu
	MTYPE_PLAIN,		// start plain board
	MTYPE_TOGGLE,		// toggle a menu item
	MTYPE_REPLAY,		// replay current level
	MTYPE_NEXTLEVEL,	// self explanatory
	MTYPE_GAMEOVER_WIN,	// win and offer next level
	MTYPE_GAMEOVER_LOSE,	// replay (tutorial)
	MTYPE_GAMEOVER_TIMEOUT, // ran out of time (tutorial)
	MTYPE_GAMEOVER_HISCORE, // prompt name and show hiscore
	MTYPE_HISCORES,		// show the current hiscores
	MTYPE_BREAK		// leave the current mode/menu (might QUIT).
};

/* Text Effect types */
enum {
	EFFECT_NONE    = 0,
	EFFECT_SHAKE   = 1 << 0,
	EFFECT_WAVE    = 1 << 1,
	EFFECT_FADEOUT = 1 << 2
};

/* Area types. */
enum {
	ATYPE_FREE,
	ATYPE_BOARD_BOTTOM,
	ATYPE_BOARD_LEFT,
	ATYPE_BOARD_RIGHT,
	ATYPE_BLOCK
};

/* Level Objective types. */
enum {
	OBJTYPE_NONE,
	OBJTYPE_CLEARALL,
	OBJTYPE_TIMED_BLOCKS,
	OBJTYPE_TIMED_SCORE,
	OBJTYPE_LINK,
	OBJTYPE_LENGTH
};

/* Pre-define types */
struct _board_s;
struct _block_s;
struct _text_s;

/* Boolean and byte types, easier to read ;) */
typedef unsigned char byte;
#ifndef __WII__
typedef enum {
	false,
	true
} bool;
#endif


/* Memory Management functions */
void		*r_malloc(size_t);
void		 r_free(void *);
void		 r_checkmem();
char		*r_strcp(char *);


/* Event functions */
enum mtype	 handle_events(SDL_Event *);
void		 wait_for_keymouse(void);
int		 cancellable_delay(int);
bool		 prompt_polling(struct _text_s *);


/* Graphic related wrappers */
void		 init_gfx();
int		 surface_fadein(SDL_Surface *, int);
int		 surface_fadeout(SDL_Surface *);
void		 surface_shutter_open();
void		 surface_shutter_close();
void		 surface_pixel_open();
void		 surface_pixel_close();
void		 surface_greyscale();
void		 r_setpixel(Uint16, Uint16, byte, byte, byte);
void		 r_setline(Uint16, Uint16, Uint16, byte, byte, byte);
SDL_Surface	*loadimage(char *);
SDL_Surface	*copy_screen();
void		 blit_modal(unsigned);


/* String related functions from OpenBSD */
size_t		 strlcpy(char *dst, const char *src, size_t size);
// char		*strsep(char **, const char *);


/* Cube structure */
typedef struct _cube {
	int current_position;
	int x;
	int y;
	int type;
	int water;
	int network_integrity;
	int network_size;
	int fade_status;
	bool trashed;
	struct _cube **network;
	struct _cube *root;
} Cube;

/* Cube functions */
Cube		*cube_new(byte);
Cube		*cube_new_type(byte, int);
Cube		*cube_new_from_char(char);
void		 cube_kill(Cube *);
Cube		*cube_new_random();
Cube		*cube_new_random_max(int);
void		 cube_init_texture();
SDL_Surface	*cube_get_surface(Cube *);
void		 cube_get_rectangle(Cube *, SDL_Rect *);
void		 cube_rotate_cw(Cube *);
void		 cube_rotate_ccw(Cube *);
byte		 cube_get_plugs(Cube *);
int		 cube_plug_match(Cube *, byte);
int		 cube_get_plug_status(Cube *, byte, Cube *, byte);
void		 cube_network_add(Cube *, Cube *);
void		 cube_network_flush(Cube *);
void		 cube_network_taint(Cube *);


/* Text structure */
typedef struct _text_s {
	int x;			// position on the screen
	int y;
	int width;		// size of the text surface (rendered)
	int height;
	bool colorized;		// fast check to skip the colorization.
	byte color1_r;		// text colors
	byte color1_g;
	byte color1_b;
	byte color2_r;
	byte color2_g;
	byte color2_b;
	int effect;		// bit map of effects
	int fx_fade_data;	// fade value
	int fx_wave_data;	// sinus value
	char *value;		// actual c-string text
	int length;		// num of chars in the value
	int max_length;		// max num of chars
	int line_spacing;	// num. of pixels between lines
	bool trashed;		// kill on next tick
	bool centered;		// horizontal centering
	bool temp;		// trash on next move
	int font;		// font idx
} Text;

/* Text functions */
Text		*text_new(char *);
void		 text_kill(Text *);
SDL_Surface	*text_get_surface(Text *);
void		 text_get_rectangle(Text *, SDL_Rect *);
void		 text_set_value(Text *, char *);
void		 text_set_colors(Text *, uint32_t, uint32_t);
void		 text_set_color1(Text *, byte, byte, byte);
void		 text_set_color2(Text *, byte, byte, byte);
void		 text_del_last_char(Text *);
void		 text_add_char(Text *, char);
void		 text_blit(Text *, SDL_Surface *);


/* HiScore structure */
typedef struct _hiscore {
	int score;
	char name[16];
	time_t date;
} HiScore;

/* HiScore functions */
void		 hiscore_add(char *, int);
void		 hiscore_show();
enum mtype	 hiscore_prompt();
bool 		 hiscore_check(int);
void		 hiscore_free();


/* Level related structures */
struct _queuedblock_s;
typedef struct _level_s {
	char *name;
	char *description;
	byte *cmap;
	struct _queuedblock_s **queue;
	size_t queue_len;
	bool allow_dynamite;
	int objective_type;
	char *next;
	int max_blocks;			// max blocks per level
	int time_limit;			// max seconds per level
	int rising_speed;		// seconds between floor rising
} Level;
typedef struct _queuedblock_s {
	int type;
	int pos;
	size_t cmap_len;
	byte *cmap;
} QueuedBlock;

/* Level functions */
Level		*lvl_load(char *);
void		 lvl_dump(Level *);
void		 lvl_kill(Level *);


/* Block structure */
typedef struct _block_s {
	bool falling;
	byte size;
	byte **positions;
	Cube **cubes;
	int cube_count;
	byte current_position;
	Sint8 x;
	Sint8 y;
	byte prev_y;
	uint32_t tick;
	byte type;
	bool existing_cubes;
} Block;

/* Block-related functions */
Block		*block_new(byte);
void		 block_kill(Block *);
SDL_Surface	*block_get_surface(Block *);
void		 block_get_rectangle(Block *, SDL_Rect *);
Block		*block_new_one_from_cube(Cube *);
Block		*block_new_one(bool);
Block		*block_new_two();
Block		*block_new_tee();
Block		*block_new_ell();
Block		*block_new_jay();
Block		*block_new_zee();
Block		*block_new_ess();
Block		*block_new_square();
Block		*block_new_bar();
Block		*block_new_random();
Block		*block_new_of_type(int);
void		 block_rotate_cw(Block *);
void		 block_rotate_ccw(Block *);


/*
 * Mole stuff
 */
typedef struct _mole_s {
	struct _board_s *board;
	unsigned int id;
	uint32_t drill_tick;
	uint32_t move_tick;
	int trail_x[MOLE_TRAIL];
	int trail_y[MOLE_TRAIL];
	int trail_cur;
	int direction;
	int drill_anim;
	int x;
	int y;
	bool flooded;
} Mole;

Mole		*mole_new();
void		 mole_kill(Mole *);


/* Configuration structure (keep data between games) */
typedef struct _configuration {
	int difficulty;
	char *current_level;
	char *next_level;
	int last_score;
	bool sound;
	bool fullscreen;
} Configuration;


/* Board structure */
typedef struct _board_s {
	/* main characteristics */
	byte width;
	byte height;
	byte offset_x;
	byte offset_y;
	int difficulty;
	char bgfilename[256];
	SDL_Surface *bg;
	enum ttype transition;
	/* cubes */
	int cube_count;
	Cube **cubes;
	bool allow_dynamite;
	uint32_t next_line;
	uint32_t elapsed;	// msec passed since start.
	/* blocks */
	int block_speed;
	int block_speed_factor;
	Block **blocks;
	int block_count;
	Block *current_block;
	Block *next_block;
	Block *hold;
	Block **bqueue;
	size_t bqueue_len;
	int remains;		// number of blocks to end of level
	bool launch_next;	// launch the next block at next update tick
	byte moving_left;
	byte moving_right;
	unsigned int lateral_tick;
	int lateral_speed;
	/* moles */
	Mole *moles[MAX_MOLES];
	int last_mole;
	/* pipes */
	int pipe_status_left[BOARD_HEIGHT];
	int pipe_status_right[BOARD_HEIGHT];
	uint32_t pipe_tick;
	/* texts */
	bool modal;
	struct _text_s **texts;
	int text_count;
	Text *status_t;
	Text *score_t;
	Text *fps_t;
	Text *timeleft_t;
	bool show_fps;
	/* prompt related */
	Text *prompt_text;
	int (*prompt_func)(Text *, Text *);
	void *prompt_data;
	/* player stuff */
	int score;
	bool paused;		// stop the game flow when true
	bool silent;		// do not show paused or score
	bool gameover;		// stop the game completely on next tick
	bool success;		// was it good?
	enum mtype status;	// where to return after game over
	int time_limit;		// remaining time (-1 is unlimited)
	int rising_speed;	// speed at which we add lines
	/* current level */
	int objective_type;
	char *next_level;
} Board;

/* Board functions */
Board		*board_new(int);
Board		*board_new_from_level(Level *);
void		 board_kill(Board *);
void		 board_loadbg(Board *, char *);
void		 board_render(Board *);
enum mtype	 board_update(Board *, uint32_t);
void		 board_toggle_pause(Board *);
enum mtype	 board_gameover(Board *);
void		 board_prepopulate(Board *, int);
void		 board_add_line(Board *);
/* Board functions (cube related) */
void		 board_add_cube(Board *);
void		 board_trash_cube(Board *, Cube *);
void		 board_render_cubes(Board *);
void		 board_dump_cube_map(Board *);
void		 board_spread_water(Board *, Cube *, Cube *, int);
void		 board_update_water(Board *, uint32_t);
void		 board_update_cubes(Board *, uint32_t);
Cube		*board_get_cube(Board *, int, int);
void		 board_run_avalanche(Board *, Cube *);
void		 board_run_avalanche_column(Board *, Cube *);
int		 board_get_area_type(Board *, int, int);
/* Board functions (block related) */
void		 board_render_blocks(Board *);
void		 board_render_next(Board *);
void		 board_render_hold(Board *);
void		 board_update_blocks(Board *, uint32_t);
void		 board_update_single_block(Board *, uint32_t, int);
void		 board_add_block(Board *, Block *);
void		 board_launch_next_block(Board *);
void		 board_load_next_block(Board *);
void		 board_change_next_block(Board *);
void		 board_move_current_block_left(Board *);
void		 board_move_current_block_right(Board *);
byte		 board_move_check(Board *, Block *, Sint8, Sint8);
void		 board_rotate_cw(Board *, Block *);
void		 board_update_map(Board *);
void		 board_dump_block_map(Board *);
void		 board_cube_bomb(Board *, Cube *);
void		 board_hold(Board *);
void		 board_block_fall(Board *);
/* Board functions (text related) */
Text		*board_add_text(Board *, char *, int, int);


/* Main menu */
int		 main_menu(void);
int		 gameover_menu();


/* Animations */
void		 sky_render(Board *);
void		 chimneys_render(Board *);
void		 a_sky_update(Board *, uint32_t);
void		 a_chimneys_update(Board *, uint32_t);


/* Audio functions */
void		 init_audio();
void		 sfx_toggle_mute(bool);
void		 sfx_play_tick1();
void		 sfx_play_tack1();
void		 sfx_play_boom();
void		 sfx_play_horn();
void		 sfx_play_menunav();
void		 sfx_play_menuselect();
void		 sfx_play_lazer();
void		 sfx_load_library();
void		 sfx_unload_library();
void		 sfx_play_music(char *);
void		 sfx_stop_music();


/* Error control */
void		 fatal(char *fmt, ...);

/* File I/O */
char		*dpath(const char *org);
