
/**
 * @file rezerwar.h
 * @brief Main header
 *
 * This header provides all the structures and prototypes for the whole
 * game.
 */

#define MAXFPS			30
#define TICK			10

#define BOARD_LEFT		176
#define BOARD_TOP		90
#define BOARD_WIDTH		9
#define BOARD_HEIGHT		10
#define BSIZE			32
#define SPEED_NORMAL		500
#define SPEED_LESS5K		400
#define SPEED_LESS10K		300
#define SPEED_LESS25K		200
#define SPEED_LESS50K		100
#define SPEED_MAX		50


enum {
	BLOCK_TYPE_TEE,
	BLOCK_TYPE_ELL,
	BLOCK_TYPE_JAY,
	BLOCK_TYPE_ZEE,
	BLOCK_TYPE_ESS,
	BLOCK_TYPE_SQUARE,
	BLOCK_TYPE_BAR,
	BLOCK_TYPE_ONE,
	BLOCK_TYPE_TWO,
	BLOCK_TYPE_THREE,
	BLOCK_TYPE_CORNER
};

/* Cube types */
enum {
	CTYPE_EMPTY,
	CTYPE_ANGLE,
	CTYPE_TEE,
	CTYPE_FLAT,
	CTYPE_KNOB,
	CTYPE_ALL,
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
	MTYPE_TOGGLE
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

typedef unsigned char byte;
typedef enum {
	false,
	true
} bool;

/* mem */
void		*r_malloc(size_t);
void		 r_free(void *);
void		 r_checkmem();

typedef struct _configuration {
	int difficulty;
} Configuration;

/* rcube.c */
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
Cube		*cube_new(byte);
void		 cube_kill(Cube *);
Cube		*cube_new_random();
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


/* text.c */
typedef struct text_s {
	int x;
	int y;
	int width;
	int height;
	bool colorized;
	byte color1_r;
	byte color1_g;
	byte color1_b;
	byte color2_r;
	byte color2_g;
	byte color2_b;
	int font;
	int effect;
	int fx_fade_data;
	unsigned char *value;
	int length;
	bool trashed;
	bool centered;
} Text;

Text		*text_new(unsigned char *);
void		 text_kill(Text *);
SDL_Surface	*text_get_surface(Text *);
void		 text_get_rectangle(Text *, SDL_Rect *);
void		 text_set_value(Text *, unsigned char *);
void		 text_set_color1(Text *, byte, byte, byte);
void		 text_set_color2(Text *, byte, byte, byte);
void		 text_del_last_char(Text *);
void		 text_add_char(Text *, char);

/* hiscore.c */
typedef struct _hiscore {
	int score;
	char name[16];
	time_t date;
} HiScore;

/* block.c */
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
	u_int32_t tick;
	byte type;
} Block;

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

/* board.c */
typedef struct _board_data {
	/* main characteristics */
	byte width;
	byte height;
	byte offset_x;
	byte offset_y;
	int difficulty;
	char bgfilename[256];
	SDL_Surface *bg;
	/* cubes */
	Cube **cubes;
	/* blocks */
	int block_speed;
	int block_speed_factor;
	Block **blocks;
	int block_count;
	Block *current_block;
	Block *next_block;
	/* controls */
	byte moving_left;
	byte moving_right;
	unsigned int lateral_tick;
	int lateral_speed;
	/* texts */
	bool modal;
	Text **texts;
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
	int paused;
	int gameover;
} Board;

Board		*board_new(int);
void		 board_kill(Board *);
void		 board_loadbg(Board *, char *);
void		 board_refresh(Board *);
void		 board_update(Board *, u_int32_t);
void		 board_toggle_pause(Board *);
void		 board_gameover(Board *);
void		 board_prepopulate(Board *, int);
/* rboard_cubes.c */
void		 board_add_cube(Board *);
void		 board_refresh_cubes(Board *);
void		 board_dump_cube_map(Board *);
void		 board_spread_water(Board *, Cube *, Cube *, int);
void		 board_update_water(Board *, u_int32_t);
void		 board_update_cubes(Board *, u_int32_t);
Cube		*board_get_cube(Board *, int, int);
void		 board_run_avalanche(Board *, Cube *);
void		 board_run_avalanche_column(Board *, Cube *);
int		 board_get_area_type(Board *, int, int);
/* rboard_blocks.c */
void		 board_refresh_blocks(Board *);
void		 board_refresh_next(Board *);
void		 board_update_blocks(Board *, u_int32_t);
void		 board_update_single_block(Board *, u_int32_t, int);
void		 board_add_block(Board *, Block *);
void		 board_launch_next_block(Board *);
void		 board_load_next_block(Board *);
void		 board_change_next_block(Board *);
void		 board_move_current_block_left(Board *);
void		 board_move_current_block_right(Board *);
void		 board_set_block_speed(Board *, u_int32_t);
byte		 board_move_check(Board *, Block *, Sint8, Sint8);
void		 board_rotate_cw(Board *);
void		 board_update_map(Board *);
void		 board_dump_block_map(Board *);

/* text related */
Text		*board_add_text(Board *, unsigned char *, int, int);

/* events.c */
byte		 handle_events(SDL_Event *);
void		 wait_for_keymouse(void);
int		 cancellable_delay(int);

/* hiscore.c */
void		 hiscore_add(char *, int);
void		 hiscore_dump(Board *);
bool 		 hiscore_check(int);
void		 hiscore_free();

/* engine_sdl.c */
int		 surface_fadein(SDL_Surface *, int);
int		 surface_fadeout(SDL_Surface *);
void		 r_setpixel(Uint16, Uint16, byte, byte, byte);
void		 r_setline(Uint16, Uint16, Uint16, byte, byte, byte);


SDL_Surface	*loadimage(char *);

/* strlcpy.c */
size_t		 strlcpy(char *dst, const char *src, size_t siz);


/* menus.c */
int		 main_menu(void);

/* animations */
void		 a_sky_refresh(Board *);
void		 a_chimneys_refresh(Board *);
void		 a_sky_update(Board *, u_int32_t);
void		 a_chimneys_update(Board *, u_int32_t);

/* sfx */
void		 sfx_play_tick1();
void		 sfx_load_library();
