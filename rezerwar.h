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

void		*r_malloc(size_t);
void		 r_free(void *);
void		 r_checkmem();
void		 r_setpixel(Uint16, Uint16, Uint8, Uint8, Uint8);
void		 r_setline(Uint16, Uint16, Uint16, Uint8, Uint8, Uint8);


/* rcube.c */
typedef struct _cube {
	Sint8 current_position;
	Sint8 position_count;
	Sint16 x;
	Sint16 y;
	int type;
	int water;
} Cube;
Cube		*cube_new(Uint8);
Cube		*cube_new_random();
void		 cube_init_texture();
SDL_Surface	*cube_get_surface(Cube *);
void		 cube_get_rectangle(Cube *, SDL_Rect *);
void		 cube_rotate_cw(Cube *);
Uint8		 cube_get_plugs(Cube *);


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
	Uint8 size;
	Uint8 falling;
	Uint8 **positions;
	Cube **cubes;
	int cube_count;
	Uint8 current_position;
	Uint8 position_count;
	Sint8 x;
	Sint8 y;
	Uint8 prev_y;
	Uint32 tick;
	Uint8 type;
} Block;

void		 block_init_btex();
Block		*block_new(Uint8, Uint8);
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
	Uint16 cube_count;
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
} Board;

Board		*board_new(Uint8, Uint8);
void		 board_kill(Board *);
SDL_Surface	*loadimage(char *);
void		 board_loadbg(Board *, char *);
void		 board_refresh(Board *);
void		 board_update(Board *, Uint32);
/* rboard_cubes.c */
void		 board_add_cube(Board *);
void		 board_refresh_cubes(Board *);
void		 board_dump_cube_map(Board *);
/* rboard_blocks.c */
void		 board_refresh_blocks(Board *);
void		 board_refresh_next(Board *);
void		 board_update_blocks(Board *, Uint32);
void		 board_add_block(Board *, Block *);
void		 board_launch_next_block(Board *);
void		 board_load_next_block(Board *);
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

