/*
 * Copyright 2009, (c) Bertrand Janin <tamentis@neopulsar.org>
 * http://tamentis.com/
 */

/* Main speed and flow control */
#define MAXFPS			30
#define TICK			10

/* Board constants */
#define BOARD_LEFT		176
#define BOARD_TOP		90
#define BOARD_WIDTH		9
#define BOARD_HEIGHT		10

/* A couple hard-coded sizes.. */
#define BSIZE			32
#define FONT_HEIGHT		19
#define LVL_MAX_SIZE		4096

/* Speed relative to difficulty */
#define SPEED_NORMAL		500
#define SPEED_LESS5K		400
#define SPEED_LESS10K		300
#define SPEED_LESS25K		200
#define SPEED_LESS50K		100
#define SPEED_MAX		50

/* Block types */
enum {
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
enum {
	CTYPE_EMPTY,		// 0
	CTYPE_ANGLE,
	CTYPE_TEE,
	CTYPE_FLAT,
	CTYPE_KNOB,
	CTYPE_ALL,		// 5
	CTYPE_BOMB
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
enum {
	MTYPE_NOP,
	MTYPE_QUIT,
	MTYPE_START,
	MTYPE_SUBMENU,
	MTYPE_PLAIN,
	MTYPE_TOGGLE,
	MTYPE_NEXTLEVEL
};

/* Text Effect types */
enum {
	EFFECT_NONE,
	EFFECT_SHAKE,
	EFFECT_FADEOUT
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
	OBJTYPE_CLEARALL,
	OBJTYPE_TIMED_BLOCKS,
	OBJTYPE_TIMED_SCORE,
	OBJTYPE_LINK,
	OBJTYPE_LENGTH
};

/* Pre-define types */
struct _board_s;

/* Boolean and byte types, easier to read ;) */
typedef unsigned char byte;
typedef enum {
	false,
	true
} bool;


/* Memory Management functions */
void		*r_malloc(size_t);
void		 r_free(void *);
void		 r_checkmem();
char		*r_strcp(char *);


/* Event functions */
byte		 handle_events(SDL_Event *);
void		 wait_for_keymouse(void);
int		 cancellable_delay(int);


/* Graphic related wrappers */
int		 surface_fadein(SDL_Surface *, int);
int		 surface_fadeout(SDL_Surface *);
void		 r_setpixel(Uint16, Uint16, byte, byte, byte);
void		 r_setline(Uint16, Uint16, Uint16, byte, byte, byte);
SDL_Surface	*loadimage(char *);


/* String related functions from OpenBSD */
size_t		 strlcpy(char *dst, const char *src, size_t siz);
char		*strsep(char **, const char *);


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
	unsigned char *value;	// actual c-string text
	int length;		// num of chars in the value
	int line_spacing;	// num. of pixels between lines
	bool trashed;		// kill on next tick
	bool centered;		// horizontal centering
	bool temp;		// trash on next move
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


/* HiScore structure */
typedef struct _hiscore {
	int score;
	char name[16];
	time_t date;
} HiScore;

/* HiScore functions */
void		 hiscore_add(char *, int);
void		 hiscore_dump(struct _board_s *);
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
	int objective_type;
	char *next;
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
typedef struct _block_data {
	byte falling;
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
} Block;

/* Block-related functions */
Block		*block_new(byte);
void		 block_kill(Block *);
SDL_Surface	*block_get_surface(Block *);
void		 block_get_rectangle(Block *, SDL_Rect *);
Block		*block_new_one_from_cube(Cube *);
Block		*block_new_one();
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


/* Configuration structure (part of Board) */
typedef struct _configuration {
	int difficulty;
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
	/* cubes */
	int cube_count;
	Cube **cubes;
	/* blocks */
	int block_speed;
	int block_speed_factor;
	Block **blocks;
	int block_count;
	Block *current_block;
	Block *next_block;
	Block **bqueue;
	size_t bqueue_len;
	/* controls */
	byte moving_left;
	byte moving_right;
	unsigned int lateral_tick;
	int lateral_speed;
	/* texts */
	bool modal;
	struct _text_s **texts;
	int text_count;
	Text *status_t;
	Text *score_t;
	Text *fps_t;
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
	/* current level */
	int objective_type;
	char *next_level;
} Board;

/* Board functions */
Board		*board_new(int);
Board		*board_new_from_level(Level *);
void		 board_kill(Board *);
void		 board_loadbg(Board *, char *);
void		 board_refresh(Board *);
int		 board_update(Board *, uint32_t);
void		 board_toggle_pause(Board *);
void		 board_gameover(Board *, bool);
void		 board_prepopulate(Board *, int);
/* Board functions (cube related) */
void		 board_add_cube(Board *);
void		 board_refresh_cubes(Board *);
void		 board_dump_cube_map(Board *);
void		 board_spread_water(Board *, Cube *, Cube *, int);
void		 board_update_water(Board *, uint32_t);
void		 board_update_cubes(Board *, uint32_t);
Cube		*board_get_cube(Board *, int, int);
void		 board_run_avalanche(Board *, Cube *);
void		 board_run_avalanche_column(Board *, Cube *);
int		 board_get_area_type(Board *, int, int);
/* Board functions (block related) */
void		 board_refresh_blocks(Board *);
void		 board_refresh_next(Board *);
void		 board_update_blocks(Board *, uint32_t);
void		 board_update_single_block(Board *, uint32_t, int);
void		 board_add_block(Board *, Block *);
void		 board_launch_next_block(Board *);
void		 board_load_next_block(Board *);
void		 board_change_next_block(Board *);
void		 board_move_current_block_left(Board *);
void		 board_move_current_block_right(Board *);
void		 board_set_block_speed(Board *, uint32_t);
byte		 board_move_check(Board *, Block *, Sint8, Sint8);
void		 board_rotate_cw(Board *);
void		 board_update_map(Board *);
void		 board_dump_block_map(Board *);
void		 board_cube_bomb(Board *, Cube *);
/* Board functions (text related) */
Text		*board_add_text(Board *, char *, int, int);


/* Main menu */
int		 main_menu(void);


/* Animations */
void		 a_sky_refresh(Board *);
void		 a_chimneys_refresh(Board *);
void		 a_sky_update(Board *, uint32_t);
void		 a_chimneys_update(Board *, uint32_t);


/* Audio functions */
void		 init_audio();
void		 sfx_play_tick1();
void		 sfx_play_tack1();
void		 sfx_play_boom();
void		 sfx_play_music();
void		 sfx_play_horn();
void		 sfx_play_menunav();
void		 sfx_play_menuselect();
void		 sfx_load_library();
void		 sfx_play_lazer();


/* Error control */
void		 fatal(char *fmt, ...);

