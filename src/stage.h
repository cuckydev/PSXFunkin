#ifndef _STAGE_H
#define _STAGE_H

#include "io.h"
#include "gfx.h"

#include "fixed.h"
#include "character.h"
#include "player.h"
#include "object.h"

//Stage enums
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

//Stage definitions
typedef struct
{
	//Characters
	struct
	{
		u8 id;
		fixed_t x, y;
	} pchar, ochar;
	
	//Song info
	fixed_t bpm;
	fixed_t speed[3];
	
	u8 week, week_song;
	u8 music_track, music_channel;
} StageDef;

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
		fixed_t tx, ty, tz, td;
	} camera;
	fixed_t bump, sbump;
	
	Player *player;
	Character *opponent;
	
	Section *cur_section; //Current section
	Note *cur_note; //First visible and hittable note, used for drawing and hit detection
	
	fixed_t note_scroll;
	
	u16 song_step;
	boolean just_step;
	
	u8 arrow_hitan[4]; //Arrow hit animation for presses
	
	s16 health;
	s32 score;
	u16 combo;
	
	enum
	{
		StageState_Play, //Game is playing as normal
		StageState_Dead,       //Start BREAK animation and reading extra data from CD
		StageState_DeadLoad,   //Wait for said data to be read
		StageState_DeadDrop,   //Mic drop
		StageState_DeadRetry,  //Retry prompt
		StageState_DeadDecide, //Decided
	} state;
	
	//Music state
	CdlFILE music_file;
	boolean vocal_active;
	
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
