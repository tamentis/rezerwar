
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

/* There is no reason why you would want the speed to be less than the
 * normal tickin of the game. */
#define DROP_SPEED		16
#define DROP_COLOR_R		15
#define DROP_COLOR_G		96
#define DROP_COLOR_B		190

/* Area types. */
enum {
	ATYPE_FREE,
	ATYPE_BOARD_BOTTOM,
	ATYPE_DROP,
	ATYPE_BOARD_LEFT,
	ATYPE_BOARD_RIGHT,
	ATYPE_BLOCK
};

/* Cube types */
#define CTYPE_EMPTY		0
#define CTYPE_ANGLE		1
#define CTYPE_TEE		2
#define CTYPE_FLAT		3
#define CTYPE_KNOB		4
#define CTYPE_ALL		5

/* Plug types */
#define PLUG_NORTH		1
#define PLUG_EAST		2
#define PLUG_SOUTH		4
#define PLUG_WEST		8

/* Plug statuses */
#define PSTAT_CONNECTED		7
#define PSTAT_OPENED		1

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

typedef unsigned char byte;
typedef enum {
	false,
	true
} bool;

void		*r_malloc(size_t);
void		 r_free(void *);
void		 r_checkmem();
void		 r_setpixel(Uint16, Uint16, Uint8, Uint8, Uint8);
void		 r_setline(Uint16, Uint16, Uint16, Uint8, Uint8, Uint8);


typedef struct _configuration {
	int difficulty;
} Configuration;

/* rcube.c */
typedef struct _cube {
	Sint8 current_position;
	Sint16 x;
	Sint16 y;
	int type;
	int water;
	int network_integrity;
	int network_size;
	int fade_status;
	int trashed;
	struct _cube **network;
	struct _cube *root;
} Cube;
Cube		*cube_new(Uint8);
void		 cube_kill(Cube *);
Cube		*cube_new_random();
void		 cube_init_texture();
SDL_Surface	*cube_get_surface(Cube *);
void		 cube_get_rectangle(Cube *, SDL_Rect *);
void		 cube_rotate_cw(Cube *);
void		 cube_rotate_ccw(Cube *);
Uint8		 cube_get_plugs(Cube *);
int		 cube_plug_match(Cube *, Uint8);
int		 cube_get_plug_status(Cube *, Uint8, Cube *, Uint8);
void		 cube_network_add(Cube *, Cube *);
void		 cube_network_flush(Cube *);
void		 cube_network_taint(Cube *);


/* rdrop.c */
typedef struct _drop {
	Sint16 x;
	Sint16 y;
	double acc_x;
	double acc_y;
	Uint32 tick;
	Sint8 moving;
} Drop;

Drop		*drop_new(Sint16, Sint16);
void		 drop_kill(Drop *);
void		 drop_draw(Drop *, Uint16, Uint16);


/* routput.c */
typedef struct _wateroutput {
	Uint8 size;
	Sint16 x;
	Sint16 y;
	Uint16 flow;
	Uint32 last_drop;
} WaterOutput;

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
	int *fx_shake_data;
	int fx_fade_data;
	unsigned char *value;
	int length;
	bool trashed;
} Text;

Text		*text_new(unsigned char *);
void		 text_kill(Text *);
SDL_Surface	*text_get_surface(Text *);
void		 text_get_rectangle(Text *, SDL_Rect *);
void		 text_set_value(Text *, unsigned char *);
void		 text_set_color1(Text *, byte, byte, byte);
void		 text_set_color2(Text *, byte, byte, byte);

/* block.c */
typedef struct _block_data {
	Uint8 falling;
	Uint8 size;
	Uint8 **positions;
	Cube **cubes;
	int cube_count;
	Uint8 current_position;
	Sint8 x;
	Sint8 y;
	Uint8 prev_y;
	Uint32 tick;
	Uint8 type;
} Block;

Block		*block_new(Uint8);
void		 block_kill(Block *);
SDL_Surface	*block_get_surface(Block *);
void		 block_get_rectangle(Block *, SDL_Rect *);
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
	Uint8 width;
	Uint8 height;
	Uint8 offset_x;
	Uint8 offset_y;
	int difficulty;
	char bgfilename[256];
	SDL_Surface *bg;
	/* cubes */
	Cube **cubes;
	/* blocks */
	Uint16 block_speed;
	int block_speed_factor;
	Block **blocks;
	Uint16 block_count;
	Block *current_block;
	Block *next_block;
	/* drops */
	Drop **drops;
	Uint16 drop_count;
	Uint16 drop_speed;
	Drop **drop_map;
	Uint16 drop_map_size;
	/* controls */
	Uint8 moving_left;
	Uint8 moving_right;
	Uint32 lateral_tick;
	Uint16 lateral_speed;
	/* water outputs */
	WaterOutput **outputs;
	Uint8 output_count;
	/* texts */
	Text **texts;
	int text_count;
	Text *status_t;
	Text *score_t;
	/* player stuff */
	int score;
	int paused;
	int gameover;
} Board;

Board		*board_new(Uint8, Uint8, int);
void		 board_kill(Board *);
void		 board_loadbg(Board *, char *);
void		 board_refresh(Board *);
void		 board_update(Board *, Uint32);
void		 board_toggle_pause(Board *);
void		 board_gameover(Board *);
void		 board_prepopulate(Board *, int);
/* rboard_cubes.c */
void		 board_add_cube(Board *);
void		 board_refresh_cubes(Board *);
void		 board_dump_cube_map(Board *);
void		 board_spread_water(Board *, Cube *, Cube *, int);
void		 board_update_water(Board *, Uint32);
void		 board_update_cubes(Board *, Uint32);
Cube		*board_get_cube(Board *, Sint16, Sint16);
void		 board_run_avalanche(Board *, Cube *);
void		 board_run_avalanche_column(Board *, Cube *);
int		 board_get_area_type(Board *, Sint16, Sint16);
/* rboard_blocks.c */
void		 board_refresh_blocks(Board *);
void		 board_refresh_next(Board *);
void		 board_update_blocks(Board *, Uint32);
void		 board_update_single_block(Board *, Uint32, Uint16);
void		 board_add_block(Board *, Block *);
void		 board_launch_next_block(Board *);
void		 board_load_next_block(Board *);
void		 board_change_next_block(Board *);
void		 board_move_current_block_left(Board *);
void		 board_move_current_block_right(Board *);
void		 board_set_block_speed(Board *, Uint32);
Uint8		 board_move_check(Board *, Block *, Sint8, Sint8);
void		 board_rotate_cw(Board *);
void		 board_update_map(Board *);
void		 board_dump_block_map(Board *);
/* rboard_drops.c */
Uint16		 board_add_drop(Board *, Drop *);
void		 board_launch_new_drop(Board *, Sint16 x, Sint16 y);
void		 board_refresh_drops(Board *);
void		 board_update_drops(Board *, Uint32);
void		 board_update_drop_map(Board *);
void		 board_dump_drop_map_bmp(Board *);
/* rboard_output.c */
void		 board_random_output(Board *);
void		 board_register_output(Board *, WaterOutput *);
void		 board_update_outputs(Board *, Uint32);

/* text related */
Text		*board_add_text(Board *, unsigned char *, int, int);

/* WaterOutput functions */
WaterOutput	*wateroutput_new(Uint8, Sint16, Sint16);
void		 wateroutput_kill(WaterOutput *);
void		 wateroutput_update(WaterOutput *, Board *, Uint32);

/* events.c */
Uint8		 handle_events(SDL_Event *);
void		 wait_for_keymouse(void);
int		 cancellable_delay(int);

/* engine_sdl.c */
int		 surface_fadein(SDL_Surface *, int);
int		 surface_fadeout(SDL_Surface *);
void		 r_setpixel(Uint16, Uint16, Uint8, Uint8, Uint8);
SDL_Surface	*loadimage(char *);

/* strlcpy.c */
size_t		 strlcpy(char *dst, const char *src, size_t siz);


/* menus.c */
int		 main_menu(void);
