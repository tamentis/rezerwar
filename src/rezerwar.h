/*
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

/*
 * Platform specific stuff
 */
#ifdef __WII__
# include <gctypes.h>
# define JOYSTICK_DEAD_ZONE	16384
# define HAS_BOOL
#endif

#ifndef MAXPATHLEN
# ifdef PATH_MAX
#  define MAXPATHLEN PATH_MAX
# else /* PATH_MAX */
#  define MAXPATHLEN 64
# endif /* PATH_MAX */
#endif /* MAXPATHLEN */


/*
 * Graphic constants
 */
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480
#define SCREEN_BPP		16
#define BOARD_LEFT		175
#define BOARD_TOP		109
#define BOARD_WIDTH		9
#define BOARD_HEIGHT		10
#define MAIN_MENU_TOP		155
#define MAIN_MENU_LEFT		-145
#define NEXT_CUBE_TOP		32
#define NEXT_CUBE_LEFT		166
#define HOLD_TOP		222
#define HOLD_LEFT		60
#define PIPE_SPRITE_TOP		298
#define PIPE_SPRITE_LEFT	161
#define PIPE_SPRITE_SIZE	40


/*
 * Game internals settings
 */
#define MAXFPS			30
#define TICK			10
#define BSIZE			32
#define FONT0_HEIGHT		19
#define FONT1_HEIGHT		17
#define LVL_MAX_SIZE		4096	// initial level buffer
#define MAX_MOLES		8	// max amount of moles on-screen
#define MOLE_TRAIL		48	// size of the mole trail
#define MAX_SCORETAGS		8	// max amount of floating score tags
#define MAX_COMBO		10	// biggest X-combo
#define MAX_FLAMES		8	// max amount of simultaneous flames


/*
 * Points/Score values
 */
#define POINTS_FIX_PIPE		500
#define POINTS_NO_HOLD		500
#define POINTS_AVALANCHE	1000
#define POINTS_NETWORK_FACTOR	16


/*
 * Wii pad buttons (as of DevKitPro+SDL of Jul 2009)
 */
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


/*
 * Transition types
 */
enum ttype {
	TTYPE_NONE,
	TTYPE_SHUTTER_OPEN,
	TTYPE_PIXEL_OPEN,
	TTYPE_GREY_CURTAIN,
};

/*
 * Plug types and statuses
 */
enum {
	PLUG_NORTH	= 1,
	PLUG_EAST	= 2,
	PLUG_SOUTH	= 4,
	PLUG_WEST	= 8,
};
enum pstat {
	PSTAT_OPENED	= 1,
	PSTAT_CONNECTED	= 7,
};


/* 
 * Menu types (really 'section' type)
 */
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


/*
 * Area types (for the cube agnostic board search).
 */
enum {
	ATYPE_FREE,
	ATYPE_BOARD_BOTTOM,
	ATYPE_BOARD_LEFT,
	ATYPE_BOARD_RIGHT,
	ATYPE_CUBE
};


/*
 * Structure prototypes
 */
typedef struct _board_s Board;
struct _text_s;
struct _queuedcube_s;


/*
 * Extra types and typedefs
 */
typedef unsigned char byte;
#ifndef HAS_BOOL
  typedef enum { false, true } bool;
#endif


/*
 * Memory Management
 */
void		*r_malloc(size_t);
void		 r_free(void *);
void		 r_checkmem();
char		*r_strcp(char *);


/* 
 * Event handling
 */
enum mtype	 handle_events(SDL_Event *);
void		 wait_for_keymouse(void);
int		 cancellable_delay(int);
bool		 prompt_polling(struct _text_s *);


/* 
 * Graphic wrappers
 */
void		 gfx_init();
int		 gfx_fadein(SDL_Surface *, int);
int		 gfx_fadeout(SDL_Surface *);
void		 gfx_shutter_open();
void		 gfx_shutter_close();
void		 gfx_greyscale();
SDL_Surface	*gfx_copyscreen();
void		 gfx_modal(unsigned);
SDL_Surface	*gfx_new(int, int);
void		 gfx_black(SDL_Surface *);
void		 gfx_toscreen(SDL_Surface *, int, int);
void		 gfx_free(SDL_Surface *);
void		 gfx_blitsprite(SDL_Rect *, SDL_Rect *);


/*
 * Audio wrappers
 */
void		 sfx_init();
void		 sfx_kill();
void		 sfx_toggle_mute(bool);
void		 sfx_play_tick1();
void		 sfx_play_tack1();
void		 sfx_play_boom();
void		 sfx_play_horn();
void		 sfx_play_menunav();
void		 sfx_play_menuselect();
void		 sfx_play_lazer();
void		 sfx_play_splash();
void		 sfx_load_library();
void		 sfx_unload_library();
void		 sfx_play_music(char *);
void		 sfx_stop_music();


/*
 * Error control
 */
void		 fatal(char *fmt, ...);


/*
 * File I/O
 */
char		*dpath(const char *org);
char		*cpath(const char *org);


/* 
 * String related
 */
size_t		 strlcpy(char *dst, const char *src, size_t size);
// char		*strsep(char **, const char *);


/*
 * Main menu
 */
int		 main_menu(void);
int		 gameover_menu();


/*
 * Animations
 */
void		 sky_update(Board *, uint32_t);
void		 sky_render(Board *);
void		 chimneys_update(Board *, uint32_t);
void		 chimneys_render(Board *);


/*
 * Cube
 */
enum ctype {
	CTYPE_EMPTY,	// 0
	CTYPE_ANGLE,
	CTYPE_TEE,
	CTYPE_FLAT,
	CTYPE_KNOB,
	CTYPE_ALL,	// 5
	CTYPE_BOMB,
	CTYPE_MEDIC,
	CTYPE_ROCK,
	CTYPE_MAX
};

typedef struct _cube_s {
	int	 index;			// position on the board map.
	int	 current_position;
	int	 x;
	int	 y;
	int	 prev_y;		// 'y' one tick earlier
	enum ctype type;
	int	 water;
	int	 network_integrity;
	int	 network_size;
	int	 fade_status;
	bool	 trashed;
	int	 speed;
	uint32_t tick;
	bool	 falling;
	struct _cube_s	**network;
	struct _cube_s	*root;
} Cube;

void		 cube_init_rmap();
Cube		*cube_new(byte);
Cube		*cube_new_type(byte, int);
Cube		*cube_new_from_char(char);
void		 cube_kill(Cube *);
Cube		*cube_new_one(bool, bool);
Cube		*cube_new_random();
Cube		*cube_new_random_max(int);
Cube		*cube_new_random_mask(unsigned int);
void		 cube_init_texture();
SDL_Surface	*cube_get_surface(Cube *);
void		 cube_rotate_cw(Cube *);
void		 cube_rotate_ccw(Cube *);
byte		 cube_get_plugs(Cube *);
int		 cube_plug_match(Cube *, byte);
int		 cube_get_plug_status(Cube *, byte, Cube *, byte);
void		 cube_network_add(Cube *, Cube *);
void		 cube_network_flush(Cube *);
void		 cube_network_taint(Cube *);
int		 cube_get_abs_x(Cube *);
int		 cube_get_abs_y(Cube *);
void		 cube_landing(Board *, Cube *);
void		 cube_sync_map(Board *, Cube *);


/*
 * Text
 */
enum {
	EFFECT_NONE    = 0,
	EFFECT_SHAKE   = 1 << 0,
	EFFECT_WAVE    = 1 << 1,
	EFFECT_FADEOUT = 1 << 2,
	EFFECT_FLOAT   = 1 << 3
};

typedef struct _text_s {
	int	 length;	// num of chars in the value
	int	 max_length;	// max num of chars
	char	*value;		// actual c-string text
	int	 line_spacing;	// num. of pixels between lines
	int	 x;		// position on the screen
	int	 y;
	int	 width;		// size of the text surface (rendered)
	int	 height;
	bool	 colorized;	// fast check to skip the colorization.
	byte	 color1_r;	// text colors
	byte	 color1_g;
	byte	 color1_b;
	byte	 color2_r;
	byte	 color2_g;
	byte	 color2_b;
	int	 effect;	// bit map of effects
	int	 fx_fade_data;	// fade value
	int	 fx_wave_data;	// sinus value
	int	 fx_float_data;	// float value
	bool	 trashed;	// kill on next tick
	bool	 centered;	// horizontal centering
	bool	 temp;		// trash on next move
	int	 font;		// font idx
} Text;

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


/* 
 * HiScore
 */
typedef struct _hiscore {
	int	 score;
	char	 name[16];
	time_t	 date;
} HiScore;

void		 hiscore_add(char *, int);
void		 hiscore_show();
enum mtype	 hiscore_prompt();
bool 		 hiscore_check(int);
void		 hiscore_free();


/*
 * Level
 */
enum {
	OBJTYPE_NONE,
	OBJTYPE_CLEARALL,
	OBJTYPE_TIMED_CUBES,
	OBJTYPE_TIMED_SCORE,
	OBJTYPE_LINK,
	OBJTYPE_LENGTH
};

typedef struct _level_s {
	char	*name;
	char	*description;
	byte	*cmap;
	size_t	 queue_len;
	int	 max_moles;		// number of moles in the level
	int	 dead_pipes;		// number of dead pipes at start
	bool	 allow_bomb;
	bool	 allow_medic;
	int	 objective_type;
	char	*next;
	int	 max_cubes;		// max cubes per level
	int	 time_limit;		// max seconds per level
	int	 rising_speed;		// seconds between floor rising
	struct _queuedcube_s **queue;
} Level;

typedef struct _queuedcube_s {
	int	 type;
	int	 pos;
	size_t	 cmap_len;
	byte	*cmap;
} QueuedCube;

Level		*lvl_load(char *);
void		 lvl_dump(Level *);
void		 lvl_kill(Level *);
bool		 lvl_bool(const char *);


/*
 * Flame
 */
typedef struct _flame_s {
	int	 type;	
	int	 pos;
	uint32_t tick;
	int	 state;
	struct _board_s	*board;
} Flame;

void		 flame_kill(Flame *);


/*
 * Mole
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
	bool trashed;
} Mole;

Mole		*mole_new();
void		 mole_kill(Mole *);
void		 mole_render_trail(Mole *); 
void		 mole_render(Mole *); 
void		 mole_update(Mole *, uint32_t);
void		 mole_destroys_left_pipe(Mole *);
void		 mole_destroys_right_pipe(Mole *);


/*
 * Pipe
 */
typedef struct _pipe_s {
	uint32_t tick;
	int	 status;
	Mole	*mole;
} Pipe;

Pipe		*pipe_new();
void		 pipe_kill(Pipe *);
void		 pipe_fix(Board *, Pipe *);


/*
 * Configuration stuff
 */
typedef struct _configuration {
	char	*current_level;
	char	*next_level;
	int	 last_score;
	bool	 sound;
	bool	 fullscreen;
} Configuration;


/*
 * Board stuff
 */
struct _board_s {
	/* main characteristics */
	byte		 width;
	byte		 height;
	byte		 offset_x;
	byte		 offset_y;
	char		 bgfilename[256];
	SDL_Surface	*bg;
	enum ttype	 transition;
	/* cubes */
	int		 cube_count;
	Cube		**cubes;
	bool		 allow_bomb;
	bool		 allow_medic;
	uint32_t	 next_line;
	uint32_t	 elapsed;	// msec passed since start.
	int		 cube_speed;
	Cube		*current_cube;
	Cube		*next_cube;
	Cube		*hold;
	Cube		**cqueue;
	size_t		 cqueue_len;
	int		 remains;	// number of cubes to end of level
	bool		 launch_next;	// launch the next cube at next tick
	byte		 moving_left;
	byte		 moving_right;
	unsigned int	 lateral_tick;
	int		 lateral_speed;
	/* cubes - bomb/flames */
	Flame		*flames[MAX_FLAMES];
	int		 last_flame;
	/* moles */
	Mole		*moles[MAX_MOLES];
	int		 last_mole;
	int		 max_moles;
	/* pipes */
	Pipe		*pipes[BOARD_HEIGHT*2];
	/* texts */
	bool		 modal;
	struct _text_s	**texts;
	int		 text_count;
	Text		*status_t;
	Text		*score_t;
	Text		*fps_t;
	Text		*timeleft_t;
	bool		 show_fps;
	/* prompt related */
	Text		*prompt_text;
	int		(*prompt_func)(Text *, Text *);
	void		*prompt_data;
	/* player stuff */
	int		 score;
	bool		 settled;	// a cube settled during this loop
	int		 combo;		// number of hits in a row
	bool		 paused;	// stop the game flow when true
	bool		 silent;	// do not show paused or score
	bool		 gameover;	// stop the game completely on next tick
	bool		 success;	// was it good?
	enum mtype	 status;	// where to return after game over
	int		 time_limit;	// remaining time (-1 is unlimited)
	int		 rising_speed;	// speed at which we add lines
	/* current level */
	char		 current_level[MAXPATHLEN];
	int		 objective_type;
	char		*next_level;
};

Board		*board_new();
Board		*board_new_from_level(Level *);
void		 board_kill(Board *);
void		 board_loadbg(Board *, char *);
void		 board_render(Board *);
enum mtype	 board_update(Board *, uint32_t);
void		 board_toggle_pause(Board *);
enum mtype	 board_gameover(Board *);
void		 board_prepopulate(Board *, int);
void		 board_add_line(Board *);
void		 board_add_points_from_cube(Board *, int, Cube *);
void		 board_add_points(Board *, int, int, int);
void		 board_set_difficulty_from_score(Board *);
/* board/cube funcs */
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
void		 board_render_next(Board *);
void		 board_render_hold(Board *);
void		 board_launch_next_cube(Board *);
void		 board_load_next_cube(Board *);
void		 board_change_next_cube(Board *);
void		 board_move_current_cube_left(Board *);
void		 board_move_current_cube_right(Board *);
byte		 board_move_check(Board *, Cube *, Sint8, Sint8);
void		 board_rotate_cw(Board *);
void		 board_update_map(Board *);
void		 board_cube_medic(Board *, Cube *);
void		 board_kill_row(Board *, int);
void		 board_kill_column(Board *, int);
void		 board_hold(Board *);
void		 board_cube_fall(Board *);
/* board/flame board/bomb */
void		 board_cube_bomb(Board *, Cube *);
void		 board_spawn_flame(Board *, int, int);
void		 board_initialize_flames(Board *);
void		 board_update_flames(Board *, uint32_t);
void		 board_render_flames(Board *);
/* board/text funcs */
Text		*board_add_text(Board *, char *, int, int);
/* board/mole funcs */
void		 board_update_moles(Board *, uint32_t);
void		 board_spawn_mole(Board *);
/* board/pipe funcs */
void		 board_render_pipes(Board *);
