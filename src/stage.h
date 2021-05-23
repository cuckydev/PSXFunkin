#ifndef _STAGE_H
#define _STAGE_H

#include "psx.h"
#include "io.h"
#include "gfx.h"

#include "object.h"

//Stage fixed point implementation
typedef s32 fixed_t;

typedef struct
{
	fixed_t x, y, w, h;
} RECT_FIXED;

#define FIXED_SHIFT (10)
#define FIXED_UNIT  (1 << FIXED_SHIFT)
#define FIXED_LAND  (FIXED_UNIT - 1)
#define FIXED_UAND  (~FIXED_LAND)

#define FIXED_DEC(d, f) (((d) << FIXED_SHIFT) / (f))

#define FIXED_MUL(x, y) (((s64)(x) * (y)) >> FIXED_SHIFT)
#define FIXED_DIV(x, y) (((x) << FIXED_SHIFT) / (y))

//Stage enums
typedef enum
{
	CharId_Boyfriend,
	CharId_Dad,
	
	CharId_Max
} CharId;

typedef enum
{
	CharAnim_Idle,
	CharAnim_Left,  CharAnim_LeftAlt,
	CharAnim_Down,  CharAnim_DownAlt,
	CharAnim_Up,    CharAnim_UpAlt,
	CharAnim_Right, CharAnim_RightAlt,
	CharAnim_Peace,
	
	CharAnim_Max
} CharAnim;

typedef enum
{
	StageId_1_1, //Bopeebo
	StageId_1_2, //Fresh
	StageId_1_3, //Dadbattle
	
	StageId_3_1, //Pico
	StageId_3_2, //Philly
	StageId_3_3, //Blammed
	
	StageId_4_1, //Satin Panties
	StageId_4_2, //High
	StageId_4_3, //MILF
	
	StageId_4_4, //Test
	
	StageId_Max
} StageId;

typedef enum
{
	StageDiff_Easy,
	StageDiff_Normal,
	StageDiff_Hard,
} StageDiff;

//Character definitions
#define ASCR_REPEAT 0xFF
#define ASCR_CHGANI 0xFE
#define ASCR_BACK   0xFD

typedef struct
{
	u8 spd;
	const u8 *script;
} CharAnimDef;

typedef struct
{
	u8 tex;
	u16 src[4];
	s16 off[2];
} CharFrame;

typedef struct
{
	fixed_t focus_height;
	
	const char *arc_tex;
	u8 texs;
	const char **tex;
	
	const CharFrame *frame;
	
	const CharAnimDef anim[CharAnim_Max];
} CharDef;

//Stage definitions
typedef struct
{
	CharId id;
	fixed_t x, y;
} StageDef_Char;

typedef struct
{
	//Characters
	StageDef_Char mchar, ochar;
	
	//Song info
	fixed_t bpm;
	fixed_t speed[3];
	
	u8 week, week_song;
	u8 music_pack, music_channel;
} StageDef;

//Character state
typedef struct
{
	//Character textures and definition
	const CharDef *char_def;
	const CharAnimDef *anim_def;
	const CharFrame *frame_def;
	CharId char_id;
	
	IO_Data arc_tex;
	u8 texs;
	IO_Data *load_tex;
	
	//Character state
	fixed_t x, y;
	
	Gfx_Tex tex;
	u8 tex_i;
	
	CharAnim anim;
	const u8 *anim_p;
	u8 anim_spd, anim_time;
	
	u8 frame;
	u8 load_tex_i;
} Character;

//Stage state
#define SECTION_FLAG_ALTANIM  (1 << 0) //Mom/Dad in Week 5
#define SECTION_FLAG_OPPFOCUS (1 << 1) //Focus on opponent

typedef struct
{
	u16 end;
	u8 flag, pad;
} Section;

#define NOTE_FLAG_OPPONENT    (1 << 2) //Note is opponent's
#define NOTE_FLAG_SUSTAIN     (1 << 3) //Note is a sustain note
#define NOTE_FLAG_SUSTAIN_END (1 << 4) //Draw as sustain ending (this sucks)
#define NOTE_FLAG_HIT         (1 << 7) //Note has been hit

typedef struct
{
	u16 pos; //1/24 steps
	u8 type, pad;
} Note;

typedef struct
{
	//HUD textures
	Gfx_Tex tex_hud0, tex_hud1;
	
	//Stage data
	Gfx_Tex tex_back0, tex_back1;
	
	const StageDef *stage_def;
	StageId stage_id;
	
	IO_Data chart_data;
	Section *sections;
	Note *notes;
	
	fixed_t speed;
	fixed_t crochet, step_crochet;
	fixed_t early_safe, late_safe;
	fixed_t note_speed;
	
	//Stage state
	struct
	{
		fixed_t x, y, zoom;
		fixed_t tx, ty, td;
	} camera;
	fixed_t bump;
	
	Character character[2];
	
	Section *cur_section; //Current section
	Note *cur_note; //First visible and hittable note, used for drawing and hit detection
	
	fixed_t note_scroll;
	
	u16 song_step;
	boolean just_step, vocal_active;
	
	u8 arrow_hitan[4]; //Arrow hit animation for presses
	
	s16 health;
	s32 score;
	u16 combo;
	
	//Object lists
	ObjectList objlist_fg, objlist_bg;
} Stage;

extern Stage stage;

//Stage drawing functions
void Stage_DrawTex(Gfx_Tex *tex, RECT *src, RECT_FIXED *dst, fixed_t zoom);

//Stage functions
void Stage_Load(StageId id, StageDiff difficulty);
void Stage_Unload();
void Stage_Tick();

#endif
