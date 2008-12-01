#define MAXFPS			30
#define TICK			10

#define BOARD_LEFT		72
#define BOARD_TOP		64

#define BSIZE			16
#define SPEED_NORMAL		500
#define SPEED_FAST		50

#define BLOCK_TYPE_TEE		0
#define BLOCK_TYPE_ELL		1
#define BLOCK_TYPE_JAY		2
#define BLOCK_TYPE_ZEE		3
#define BLOCK_TYPE_ESS		4
#define BLOCK_TYPE_SQUARE	5
#define BLOCK_TYPE_BAR		6
#define BLOCK_TYPE_ONE		7
#define BLOCK_TYPE_TWO		8
#define BLOCK_TYPE_THREE	9
#define BLOCK_TYPE_CORNER	10

/* There is no reason why you would want the speed to be less than the
 * normal tickin of the game. */
#define DROP_SPEED		16
#define DROP_COLOR_R		15
#define DROP_COLOR_G		96
#define DROP_COLOR_B		190

/* Area types. */
#define ATYPE_FREE		0
#define ATYPE_BOARD_BOTTOM	1
#define ATYPE_DROP		2
#define ATYPE_BOARD_LEFT	3
#define ATYPE_BOARD_RIGHT	4
#define ATYPE_BLOCK		5

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
#define DIFF_SUPER_EASY		0
#define DIFF_EASY		1
#define	DIFF_MEDIUM		2
#define DIFF_HARD		3
#define DIFF_ULTRA		4

/**
 * @file rezerwar.h
 * @brief Main header
 *
 * This header provides all the structures and prototypes for the whole
 * game.
 */


void		*r_malloc(size_t);
void		 r_free(void *);
void		 r_checkmem();
void		 r_setpixel(Uint16, Uint16, Uint8, Uint8, Uint8);
void		 r_setline(Uint16, Uint16, Uint16, Uint8, Uint8, Uint8);


typedef struct _cube {
	Sint8 current_position;
	Sint16 x;
	Sint16 y;
	int type;
	int water;
	int network_integrity;
	int network_size;
	int trashed;
	struct _cube **network;
} Cube;
Cube		*cube_new(Uint8);
void		 cube_kill(Cube *);
Cube		*cube_new_random();
void		 cube_init_texture();
SDL_Surface	*cube_get_surface(Cube *);
void		 cube_get_rectangle(Cube *, SDL_Rect *);
void		 cube_rotate_cw(Cube *);
Uint8		 cube_get_plugs(Cube *);
int		 cube_plug_match(Cube *, Uint8);
int		 cube_get_plug_status(Cube *, Uint8, Cube *, Uint8);
void		 cube_network_add(Cube *, Cube *);
void		 cube_network_flush(Cube *);


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
Block		*block_new_tee();
Block		*block_new_ell();
Block		*block_new_jay();
Block		*block_new_zee();
Block		*block_new_ess();
Block		*block_new_square();
Block		*block_new_bar();
Block		*block_new_random();
void		 block_rotate_cw(Block *);

/* board.c */
typedef struct _board_data {
	/* main characteristics */
	Uint8 width;
	Uint8 height;
	Uint8 offset_x;
	Uint8 offset_y;
	char bgfilename[256];
	SDL_Surface *bg;
	/* cubes */
	Cube **cubes;
	/* blocks */
	Uint16 block_speed;
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
	/* player stuff */
	int score;
	int paused;
} Board;

Board		*board_new(Uint8, Uint8);
void		 board_kill(Board *);
void		 board_loadbg(Board *, char *);
void		 board_refresh(Board *);
void		 board_update(Board *, Uint32);
void		 board_toggle_pause(Board *);
/* rboard_cubes.c */
void		 board_add_cube(Board *);
void		 board_refresh_cubes(Board *);
void		 board_dump_cube_map(Board *);
void		 board_spread_water(Board *, Cube *, Cube *);
void		 board_update_water(Board *, Uint32);
void		 board_update_cubes(Board *, Uint32);
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

/* osd.c */
void		 osd_print(char *, int, int);
void		 osd_print_moving(char *, int, int, int);

