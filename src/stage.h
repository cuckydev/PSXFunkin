/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _STAGE_H
#define _STAGE_H

#include "io.h"
#include "gfx.h"
#include "pad.h"

#include "fixed.h"
#include "character.h"
#include "player.h"
#include "object.h"

#include "network.h"

//Stage constants
#define INPUT_LEFT  (PAD_LEFT  | PAD_SQUARE)
#define INPUT_DOWN  (PAD_DOWN  | PAD_CROSS)
#define INPUT_UP    (PAD_UP    | PAD_TRIANGLE)
#define INPUT_RIGHT (PAD_RIGHT | PAD_CIRCLE)

#define STAGE_FLAG_JUST_STEP     (1 << 0) //Song just stepped this frame
#define STAGE_FLAG_VOCAL_ACTIVE  (1 << 1) //Song's vocal track is currently active
#define STAGE_FLAG_SCORE_REFRESH (1 << 2) //Score text should be refreshed

#define STAGE_LOAD_PLAYER     (1 << 0) //Reload player character
#define STAGE_LOAD_OPPONENT   (1 << 1) //Reload opponent character
#define STAGE_LOAD_GIRLFRIEND (1 << 2) //Reload girlfriend character
#define STAGE_LOAD_STAGE      (1 << 3) //Reload stage
#define STAGE_LOAD_FLAG       (1 << 7)

//Stage enums
typedef enum
{
	StageId_1_1, //Bopeebo
	StageId_1_2, //Fresh
	StageId_1_3, //Dadbattle
	StageId_1_4, //Tutorial
	
	StageId_2_1, //Spookeez
	StageId_2_2, //South
	StageId_2_3, //Monster
	
	StageId_3_1, //Pico
	StageId_3_2, //Philly
	StageId_3_3, //Blammed
	
	StageId_4_1, //Satin Panties
	StageId_4_2, //High
	StageId_4_3, //MILF
	
	StageId_4_4, //Test
	
	StageId_5_1, //Cocoa
	StageId_5_2, //Eggnog
	StageId_5_3, //Winter Horrorland
	
	StageId_6_1, //Senpai
	StageId_6_2, //Roses
	StageId_6_3, //Thorns
	
	StageId_7_1, //Ugh
	StageId_7_2, //Guns
	StageId_7_3, //Stress
	
	StageId_Kapi_1, //Wocky
	StageId_Kapi_2, //Beathoven
	StageId_Kapi_3, //Hairball
	StageId_Kapi_4, //Nyaw
	
	StageId_Clwn_1, //Improbable Outset
	StageId_Clwn_2, //Madness
	StageId_Clwn_3, //Hellclown
	StageId_Clwn_4, //Expurgation
	
	StageId_2_4, //Clucked
	
	StageId_Max
} StageId;

#define StageId_LastVanilla StageId_7_3

typedef enum
{
	StageDiff_Easy,
	StageDiff_Normal,
	StageDiff_Hard,
} StageDiff;

typedef enum
{
	StageMode_Normal,
	StageMode_Swap,
	StageMode_2P,
	StageMode_Net1,
	StageMode_Net2,
} StageMode;

typedef enum
{
	StageTrans_Menu,
	StageTrans_NextSong,
	StageTrans_Reload,
	StageTrans_Disconnect,
} StageTrans;

//Stage background
typedef struct StageBack
{
	//Stage background functions
	void (*draw_fg)(struct StageBack*);
	void (*draw_md)(struct StageBack*);
	void (*draw_bg)(struct StageBack*);
	void (*free)(struct StageBack*);
} StageBack;

//Stage definitions
typedef struct
{
	//Characters
	struct
	{
		Character* (*new)();
		fixed_t x, y;
	} pchar, ochar, gchar;
	
	//Stage background
	StageBack* (*back)();
	
	//Song info
	fixed_t speed[3];
	
	u8 week, week_song;
	u8 music_track, music_channel;
	
	StageId next_stage;
	u8 next_load;
} StageDef;

//Stage state
#define SECTION_FLAG_OPPFOCUS (1 << 15) //Focus on opponent
#define SECTION_FLAG_BPM_MASK 0x7FFF //1/24

typedef struct
{
	u16 end; //1/12 steps
	u16 flag;
} Section;

#define NOTE_FLAG_OPPONENT    (1 << 2) //Note is opponent's
#define NOTE_FLAG_SUSTAIN     (1 << 3) //Note is a sustain note
#define NOTE_FLAG_SUSTAIN_END (1 << 4) //Is either end of sustain
#define NOTE_FLAG_ALT_ANIM    (1 << 5) //Note plays alt animation
#define NOTE_FLAG_MINE        (1 << 6) //Note is a mine
#define NOTE_FLAG_HIT         (1 << 7) //Note has been hit

typedef struct
{
	u16 pos; //1/12 steps
	u16 type;
} Note;

typedef struct
{
	Character *character;
	
	fixed_t arrow_hitan[4]; //Arrow hit animation for presses
	
	s16 health;
	u16 combo;
	
	boolean refresh_score;
	s32 score, max_score;
	char score_text[13];
	
	u16 pad_held, pad_press;
} PlayerState;

typedef struct
{
	//Stage settings
	boolean ghost, downscroll, expsync;
	s32 mode;
	
	u32 offset;
	
	//HUD textures
	Gfx_Tex tex_hud0, tex_hud1;
	
	//Stage data
	const StageDef *stage_def;
	StageId stage_id;
	StageDiff stage_diff;
	
	IO_Data chart_data;
	Section *sections;
	Note *notes;
	size_t num_notes;
	
	fixed_t speed;
	fixed_t step_crochet, step_time;
	fixed_t early_safe, late_safe, early_sus_safe, late_sus_safe;
	
	//Stage state
	boolean story;
	u8 flag;
	StageTrans trans;
	
	struct
	{
		fixed_t x, y, zoom;
		fixed_t tx, ty, tz, td;
		fixed_t bzoom;
	} camera;
	fixed_t bump, sbump;
	
	StageBack *back;
	
	Character *player;
	Character *opponent;
	Character *gf;
	
	Section *cur_section; //Current section
	Note *cur_note; //First visible and hittable note, used for drawing and hit detection
	
	fixed_t note_scroll, song_time, interp_time, interp_ms, interp_speed;
	
	u16 last_bpm;
	
	fixed_t time_base;
	u16 step_base;
	Section *section_base;
	
	s16 song_step;
	
	u8 gf_speed; //Typically 4 steps, changes in Fresh
	
	PlayerState player_state[2];
	s32 max_score;
	
	enum
	{
		StageState_Play, //Game is playing as normal
		StageState_Dead,       //Start BREAK animation and reading extra data from CD
		StageState_DeadLoad,   //Wait for said data to be read
		StageState_DeadDrop,   //Mic drop
		StageState_DeadRetry,  //Retry prompt
		StageState_DeadDecide, //Decided
	} state;
	
	u8 note_swap;
	
	//Object lists
	ObjectList objlist_splash, objlist_fg, objlist_bg;
} Stage;

extern Stage stage;

//Stage drawing functions
void Stage_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 r, u8 g, u8 b);
void Stage_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom);
void Stage_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom);

//Stage functions
void Stage_Load(StageId id, StageDiff difficulty, boolean story);
void Stage_Unload();
void Stage_Tick();

#ifdef PSXF_NETWORK
void Stage_NetHit(Packet *packet);
void Stage_NetMiss(Packet *packet);
#endif

#endif
