#include "stage.h"

#include "mem.h"
#include "audio.h"
#include "pad.h"
#include "main.h"
#include "random.h"

#include "menu.h"
#include "trans.h"
#include "loadscr.h"

#include "object/combo.h"

//Stage constants
//#define STAGE_PERFECT //Play all notes perfectly

//Stage definitions
#include "character/bf.h"
#include "character/bfweeb.h"
#include "character/gf.h"
#include "character/dad.h"
#include "character/mom.h"
#include "character/tank.h"

#include "stage/dummy.h"
#include "stage/week4.h"
#include "stage/week7.h"

static const StageDef stage_defs[StageId_Max] = {
	{ //StageId_1_1 (Bopeebo)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),    FIXED_DEC(0,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(1,1),FIXED_DEC(13,10)},
		1, 1,
		XA_Bopeebo, 0,
		
		StageId_1_2, STAGE_LOAD_FLAG
	},
	{ //StageId_1_2 (Fresh)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(13,10),FIXED_DEC(18,10)},
		1, 2,
		XA_Fresh, 2,
		
		StageId_1_3, STAGE_LOAD_FLAG
	},
	{ //StageId_1_3 (Dadbattle)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(13,10),FIXED_DEC(15,10),FIXED_DEC(23,10)},
		1, 3,
		XA_Dadbattle, 0,
		
		StageId_1_3, 0
	},
	{ //StageId_1_4 (Tutorial)
		//Characters
		{Char_BF_New,  FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New,   FIXED_DEC(0,1),  FIXED_DEC(-15,1)}, //TODO
		{Char_GF_New,    FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(1,1),FIXED_DEC(1,1)},
		1, 4,
		XA_Tutorial, 2,
		
		StageId_1_4, 0
	},
	
	{ //StageId_2_1 (Spookeez)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(17,10),FIXED_DEC(24,10)},
		2, 1,
		XA_Spookeez, 0,
		
		StageId_2_2, STAGE_LOAD_FLAG
	},
	{ //StageId_2_2 (South)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(11,10),FIXED_DEC(15,10),FIXED_DEC(22,10)},
		2, 2,
		XA_South, 2,
		
		StageId_2_3
	},
	{ //StageId_2_3 (Monster)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(13,10),FIXED_DEC(13,10),FIXED_DEC(16,10)},
		2, 3,
		XA_Monster, 0,
		
		StageId_2_3, 0
	},
	
	{ //StageId_3_1 (Pico)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(12,10),FIXED_DEC(14,10),FIXED_DEC(16,10)},
		3, 1,
		XA_Pico, 0,
		
		StageId_3_2, STAGE_LOAD_FLAG
	},
	{ //StageId_3_2 (Philly)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(13,10),FIXED_DEC(2,1)},
		3, 2,
		XA_Philly, 2,
		
		StageId_3_3, STAGE_LOAD_FLAG
	},
	{ //StageId_3_3 (Blammed)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(12,10),FIXED_DEC(15,10),FIXED_DEC(23,10)},
		3, 3,
		XA_Blammed, 0,
		
		StageId_3_3, 0
	},
	
	{ //StageId_4_1 (Satin Panties)
		//Characters
		{Char_BF_New,   FIXED_DEC(120,1),   FIXED_DEC(40,1)},
		{Char_Mom_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Week4_New,
		
		//Song info
		{FIXED_DEC(13,10),FIXED_DEC(16,10),FIXED_DEC(18,10)},
		4, 1,
		XA_SatinPanties, 0,
		
		StageId_4_2, STAGE_LOAD_FLAG
	},
	{ //StageId_4_2 (High)
		//Characters
		{Char_BF_New,   FIXED_DEC(120,1),   FIXED_DEC(40,1)},
		{Char_Mom_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Week4_New,
		
		//Song info
		{FIXED_DEC(13,10),FIXED_DEC(18,10),FIXED_DEC(2,1)},
		4, 2,
		XA_High, 2,
		
		StageId_4_3, STAGE_LOAD_FLAG
	},
	{ //StageId_4_3 (MILF)
		//Characters
		{Char_BF_New,   FIXED_DEC(120,1),   FIXED_DEC(40,1)},
		{Char_Mom_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Week4_New,
		
		//Song info
		{FIXED_DEC(14,10),FIXED_DEC(17,10),FIXED_DEC(26,10)},
		4, 3,
		XA_MILF, 0,
		
		StageId_4_3, 0
	},
	{ //StageId_4_4 (Test)
		//Characters
		{Char_BF_New,     FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Tank_New,  FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,       FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(16,10),FIXED_DEC(16,10),FIXED_DEC(16,10)},
		4, 4,
		XA_Test, 2,
		
		StageId_4_4, 0
	},
	
	{ //StageId_5_1 (Cocoa)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(13,10),FIXED_DEC(13,10),FIXED_DEC(13,10)},
		5, 1,
		XA_Cocoa, 0,
		
		StageId_5_2, STAGE_LOAD_FLAG
	},
	{ //StageId_5_2 (Eggnog)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(14,10),FIXED_DEC(16,10),FIXED_DEC(19,10)},
		5, 2,
		XA_Eggnog, 2,
		
		StageId_5_3, STAGE_LOAD_FLAG
	},
	{ //StageId_5_3 (Winter Horrorland)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(11,10),FIXED_DEC(13,10)},
		5, 3,
		XA_WinterHorrorland, 0,
		
		StageId_5_3, 0
	},
	
	{ //StageId_6_1 (Senpai)
		//Characters
		{Char_BFWeeb_New, FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New,   FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_BFWeeb_New,       FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(12,10),FIXED_DEC(13,10)},
		6, 1,
		XA_Senpai, 0,
		
		StageId_6_2, STAGE_LOAD_FLAG | STAGE_LOAD_OPPONENT
	},
	{ //StageId_6_2 (Roses)
		//Characters
		{Char_BFWeeb_New, FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New,   FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_BFWeeb_New,       FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(12,10),FIXED_DEC(13,10),FIXED_DEC(15,10)},
		6, 2,
		XA_Roses, 2,
		
		StageId_6_3, STAGE_LOAD_FLAG | STAGE_LOAD_OPPONENT | STAGE_LOAD_STAGE
	},
	{ //StageId_6_3 (Thorns)
		//Characters
		{Char_BFWeeb_New, FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New,   FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_BFWeeb_New,       FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(11,10),FIXED_DEC(13,10),FIXED_DEC(15,10)},
		6, 3,
		XA_Thorns, 0,
		
		StageId_6_3, 0
	},
	
	{ //StageId_7_1 (Ugh)
		//Characters
		{Char_BF_New,    FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Tank_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,      FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Week7_New,
		
		//Song info
		{FIXED_DEC(125,100),FIXED_DEC(18,10),FIXED_DEC(23,10)},
		7, 1,
		XA_Ugh, 0,
		
		StageId_7_2, STAGE_LOAD_FLAG
	},
	{ //StageId_7_2 (Guns)
		//Characters
		{Char_BF_New,    FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Tank_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,      FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Week7_New,
		
		//Song info
		{FIXED_DEC(14,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		7, 2,
		XA_Guns, 2,
		
		StageId_7_3, STAGE_LOAD_FLAG | STAGE_LOAD_PLAYER | STAGE_LOAD_OPPONENT | STAGE_LOAD_GIRLFRIEND
	},
	{ //StageId_7_3 (Stress)
		//Characters
		{Char_BF_New,    FIXED_DEC(105,1),  FIXED_DEC(100,1)}, //TODO: carry gf
		{Char_Tank_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,      FIXED_DEC(0,1),  FIXED_DEC(-15,1)}, //TODO: pico funny
		
		//Stage background
		Back_Week7_New,
		
		//Song info
		{FIXED_DEC(175,100),FIXED_DEC(22,10),FIXED_DEC(26,10)},
		7, 3,
		XA_Stress, 0,
		
		StageId_7_3, 0
	},
	
	{ //StageId_Kapi_1 (Wocky)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),    FIXED_DEC(0,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(22,10),FIXED_DEC(22,10),FIXED_DEC(22,10)},
		0x80, 1,
		XA_Wocky, 0,
		
		StageId_Kapi_2, STAGE_LOAD_FLAG
	},
	{ //StageId_Kapi_2 (Beathoven)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(21,10),FIXED_DEC(21,10),FIXED_DEC(21,10)},
		0x80, 2,
		XA_Beathoven, 2,
		
		StageId_Kapi_3, STAGE_LOAD_FLAG
	},
	{ //StageId_Kapi_3 (Hairball)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(19,10),FIXED_DEC(19,10),FIXED_DEC(19,10)},
		0x80, 3,
		XA_Hairball, 0,
		
		StageId_Kapi_4, STAGE_LOAD_FLAG
	},
	{ //StageId_Kapi_4 (Nyaw)
		//Characters
		{Char_BF_New,   FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Dad_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,     FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(25,10),FIXED_DEC(25,10),FIXED_DEC(25,10)},
		0x80, 4,
		XA_Nyaw, 2,
		
		StageId_Kapi_4, 0
	},
	
	{ //StageId_Clwn_1 (Improbable Outset)
		//Characters
		{Char_BF_New,    FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Tank_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,      FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(2,1),FIXED_DEC(25,10),FIXED_DEC(3,1)},
		0x81, 1,
		XA_ImprobableOutset, 0,
		
		StageId_Clwn_2, STAGE_LOAD_FLAG | STAGE_LOAD_OPPONENT
	},
	{ //StageId_Clwn_2 (Madness)
		//Characters
		{Char_BF_New,    FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Tank_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,      FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(2,1),FIXED_DEC(25,10),FIXED_DEC(3,1)},
		0x81, 2,
		XA_Madness, 2,
		
		StageId_Clwn_3, 0
	},
	{ //StageId_Clwn_3 (Hellclown)
		//Characters
		{Char_BF_New,    FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Tank_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,      FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(2,1),FIXED_DEC(25,10),FIXED_DEC(3,1)},
		0x81, 3,
		XA_Hellclown, 0,
		
		StageId_Clwn_3, 0
	},
	{ //StageId_Clwn_4 (Expurgation)
		//Characters
		{Char_BF_New,    FIXED_DEC(105,1),  FIXED_DEC(100,1)},
		{Char_Tank_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,      FIXED_DEC(0,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(34,10),FIXED_DEC(34,10),FIXED_DEC(34,10)},
		0x81, 4,
		XA_Expurgation, 2,
		
		StageId_Clwn_4, 0
	},
};

//Stage state
Stage stage;

//Stage music functions
void Stage_StartVocal()
{
	if (!(stage.flag & STAGE_FLAG_VOCAL_ACTIVE))
	{
		Audio_ChannelXA(stage.stage_def->music_channel);
		stage.flag |= STAGE_FLAG_VOCAL_ACTIVE;
	}
}

void Stage_CutVocal()
{
	if (stage.flag & STAGE_FLAG_VOCAL_ACTIVE)
	{
		Audio_ChannelXA(stage.stage_def->music_channel + 1);
		stage.flag &= ~STAGE_FLAG_VOCAL_ACTIVE;
	}
}

//Stage camera functions
void Stage_FocusCharacter(Character *ch, fixed_t div)
{
	//Use character focus settings to update target position and zoom
	stage.camera.tx = ch->x + ch->focus_x;
	stage.camera.ty = ch->y + ch->focus_y;
	stage.camera.tz = ch->focus_zoom;
	stage.camera.td = div;
}

void Stage_ScrollCamera()
{
	//Get delta position
	fixed_t dx = stage.camera.tx - stage.camera.x;
	fixed_t dy = stage.camera.ty - stage.camera.y;
	fixed_t dz = stage.camera.tz - stage.camera.zoom;
	
	//Scroll based off current divisor
	stage.camera.x += FIXED_MUL(dx, stage.camera.td);
	stage.camera.y += FIXED_MUL(dy, stage.camera.td);
	stage.camera.zoom += FIXED_MUL(dz, stage.camera.td);
	
	//Update other camera stuff
	stage.camera.bzoom = FIXED_MUL(stage.camera.zoom, stage.bump);
}

//Stage section functions
void Stage_ChangeBPM(u16 bpm, u16 step)
{
	//Update last BPM
	stage.last_bpm = bpm;
	
	//Update timing base
	if (stage.step_crochet)
		stage.time_base += FIXED_DIV(((fixed_t)step - stage.step_base) << FIXED_SHIFT, stage.step_crochet);
	stage.step_base = step;
	
	//Get new crochet
	fixed_t bpm_dec = ((fixed_t)bpm << FIXED_SHIFT) / 24;
	stage.step_crochet = FIXED_DIV(bpm_dec, FIXED_DEC(125,100)); //15/24
	
	//Get new crochet based values
	stage.note_speed = FIXED_MUL(FIXED_DIV(FIXED_DEC(140,1), stage.step_crochet), stage.speed);
	
	if (stage.kade)
	{
		stage.early_safe = stage.late_safe = stage.step_crochet / 6; //166 ms
		stage.late_sus_safe = (stage.late_safe * 3) >> 1;
		stage.early_sus_safe = stage.early_safe >> 1;
	}
	else
	{
		stage.late_safe = stage.step_crochet / 6; //10 frames
		stage.early_safe = stage.late_safe >> 1;
		stage.late_sus_safe = stage.late_safe;
		stage.early_sus_safe = stage.early_safe;
	}
}

Section *Stage_GetPrevSection(Section *section)
{
	if (section > stage.sections)
		return section - 1;
	return NULL;
}

u16 Stage_GetSectionLength(Section *section)
{
	Section *prev = Stage_GetPrevSection(section);
	if (prev == NULL)
		return section->end;
	return section->end - prev->end;
}

u16 Stage_GetSectionStart(Section *section)
{
	Section *prev = Stage_GetPrevSection(section);
	if (prev == NULL)
		return 0;
	return prev->end;
}

//Section scroll structure
typedef struct
{
	fixed_t start;   //Seconds
	fixed_t length;  //Seconds
	u16 start_step;  //Sub-steps
	u16 length_step; //Sub-steps
	
	fixed_t size; //Note height
} SectionScroll;

void Stage_GetSectionScroll(SectionScroll *scroll, Section *section)
{
	//Get BPM
	u16 bpm = section->flag & SECTION_FLAG_BPM_MASK;
	fixed_t bpm_dec = ((fixed_t)bpm << FIXED_SHIFT) / 24;
	
	//Get section step info
	scroll->start_step = Stage_GetSectionStart(section);
	scroll->length_step = section->end - scroll->start_step;
	
	//Get section time length            15/24
	scroll->length = FIXED_DIV(FIXED_DEC(125,100) * scroll->length_step, bpm_dec);
	
	//Get note height
	scroll->size = FIXED_MUL(stage.speed, scroll->length * (12 * 140) / scroll->length_step) + FIXED_UNIT;
}

//Note hit detection
static const CharAnim note_anims[4][2] = {
	{CharAnim_Left,  CharAnim_LeftAlt},
	{CharAnim_Down,  CharAnim_DownAlt},
	{CharAnim_Up,    CharAnim_UpAlt},
	{CharAnim_Right, CharAnim_RightAlt},
};

void Stage_MissNote(void)
{
	if (stage.combo)
	{
		//Kill combo
		if (stage.combo > 5)
			stage.gf->set_anim(stage.gf, CharAnim_Down); //Cry if we lost a large combo
		stage.combo = 0;
		
		//Create combo object telling of our lost combo
		Obj_Combo *combo = Obj_Combo_New(
			stage.player->x + stage.player->focus_x,
			stage.player->y + stage.player->focus_y,
			0xFF,
			0
		);
		if (combo != NULL)
			ObjectList_Add(&stage.objlist_fg, (Object*)combo);
	}
}

void Stage_HitNote(fixed_t offset)
{
	//Get hit type
	if (offset < 0)
		offset = -offset;
	
	u8 hit_type;
	if (stage.kade)
	{
		if (offset > stage.late_safe * 81 / 100) //135ms
			hit_type = 3; //SHIT
		else if (offset > stage.late_safe * 54 / 100) //90ms
			hit_type = 2; //BAD
		else if (offset > stage.late_safe * 27 / 100) //45ms
			hit_type = 1; //GOOD
		else
			hit_type = 0; //SICK
		
		if (hit_type == 3)
		{
			//Lose health and score
			Stage_CutVocal();
			stage.health -= 600;
			stage.score -= 30;
			
			if (stage.combo)
			{
				//Kill combo
				if (stage.combo > 5)
					stage.gf->set_anim(stage.gf, CharAnim_Down); //Cry if we lost a large combo
				stage.combo = 0;
				
				//Create combo object telling of our lost combo
				Obj_Combo *combo = Obj_Combo_New(
					stage.player->x + stage.player->focus_x,
					stage.player->y + stage.player->focus_y,
					3,
					0
				);
				if (combo != NULL)
					ObjectList_Add(&stage.objlist_fg, (Object*)combo);
			}
		}
		else
		{
			//Increment combo and score
			stage.combo++;
			
			static const s32 score_inc[] = {
				 35, //SICK
				 20, //GOOD
				  0, //BAD
				-30, //SHIT
			};
			stage.score += score_inc[hit_type];
			stage.flag |= STAGE_FLAG_SCORE_REFRESH;
			
			//Restore vocals and health
			static const s16 health_inc[] = {
				 400, //SICK
				   0, //GOOD
				-300, //BAD
				-600, //SHIT
			};
			Stage_StartVocal();
			stage.health += health_inc[hit_type];
			
			//Create combo object telling of our combo
			Obj_Combo *combo = Obj_Combo_New(
				stage.player->x + stage.player->focus_x,
				stage.player->y + stage.player->focus_y,
				hit_type,
				stage.combo >= 10 ? stage.combo : 0xFFFF
			);
			if (combo != NULL)
				ObjectList_Add(&stage.objlist_fg, (Object*)combo);
		}
	}
	else
	{
		if (offset > stage.late_safe * 9 / 10)
			hit_type = 3; //SHIT
		else if (offset > stage.late_safe * 3 / 4)
			hit_type = 2; //BAD
		else if (offset > stage.late_safe / 5)
			hit_type = 1; //GOOD
		else
			hit_type = 0; //SICK
		
		//Increment combo and score
		stage.combo++;
		
		static const s32 score_inc[] = {
			35, //SICK
			20, //GOOD
			10, //BAD
			 5, //SHIT
		};
		stage.score += score_inc[hit_type];
		stage.flag |= STAGE_FLAG_SCORE_REFRESH;
		
		//Restore vocals and health
		Stage_StartVocal();
		stage.health += 230;
		
		//Create combo object telling of our combo
		Obj_Combo *combo = Obj_Combo_New(
			stage.player->x + stage.player->focus_x,
			stage.player->y + stage.player->focus_y,
			hit_type,
			stage.combo >= 10 ? stage.combo : 0xFFFF
		);
		if (combo != NULL)
			ObjectList_Add(&stage.objlist_fg, (Object*)combo);
	}
}

void Stage_NoteCheck(u8 type)
{
	//Perform note check
	for (Note *note = stage.cur_note;; note++)
	{
		if (!(note->type & NOTE_FLAG_MINE))
		{
			//Check if note can be hit
			fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
			if (note_fp - stage.early_safe > stage.note_scroll)
				break;
			if (note_fp + stage.late_safe < stage.note_scroll)
				continue;
			if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
				continue;
			
			//Hit the note
			note->type |= NOTE_FLAG_HIT;
			
			stage.player->set_anim(stage.player, note_anims[type][0]);
			Stage_HitNote(stage.note_scroll - note_fp);
			stage.arrow_hitan[type] = 6;
			return;
		}
		else
		{
			//Check if mine can be hit
			fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
			if (note_fp - (stage.late_safe * 3 / 5) > stage.note_scroll)
				break;
			if (note_fp + (stage.late_safe * 2 / 5) < stage.note_scroll)
				continue;
			if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
				continue;
			
			//Hit the mine
			note->type |= NOTE_FLAG_HIT;
			
			if (stage.stage_id == StageId_Clwn_4)
				stage.health = -0x7000;
			else
				stage.health -= 2000;
			stage.player->set_anim(stage.player, note_anims[type][1]);
			stage.arrow_hitan[type] = 0;
		}
	}
	
	//Missed a note
	if (!stage.ghost)
	{
		stage.player->set_anim(stage.player, note_anims[type][1]);
		Stage_MissNote();
		
		stage.health -= stage.kade ? 1000 : 400;
		stage.score -= 1;
		stage.flag |= STAGE_FLAG_SCORE_REFRESH;
	}
}

void Stage_SustainCheck(u8 type)
{
	//Hold note animation
	if (stage.arrow_hitan[type] == 0)
		stage.arrow_hitan[type] = 1;
	
	//Perform note check
	for (Note *note = stage.cur_note;; note++)
	{
		//Check if note can be hit
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		if (note_fp - stage.early_sus_safe > stage.note_scroll)
			break;
		if (note_fp + stage.late_sus_safe < stage.note_scroll)
			continue;
		if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || !(note->type & NOTE_FLAG_SUSTAIN))
			continue;
		
		//Hit the note
		note->type |= NOTE_FLAG_HIT;
		
		stage.player->set_anim(stage.player, note_anims[type][0]);
		
		Stage_StartVocal();
		if (!stage.kade)
			stage.health += 230;
		stage.arrow_hitan[type] = 6;
		return;
	}
}

//Stage drawing functions
void Stage_DrawTex(Gfx_Tex *tex, RECT *src, RECT_FIXED *dst, fixed_t zoom)
{
	fixed_t l = (SCREEN_WIDTH2  << FIXED_SHIFT) + FIXED_MUL(dst->x, zoom);
	fixed_t t = (SCREEN_HEIGHT2 << FIXED_SHIFT) + FIXED_MUL(dst->y, zoom);
	fixed_t r = l + FIXED_MUL(dst->w, zoom);
	fixed_t b = t + FIXED_MUL(dst->h, zoom);
	
	l >>= FIXED_SHIFT;
	t >>= FIXED_SHIFT;
	r >>= FIXED_SHIFT;
	b >>= FIXED_SHIFT;
	
	RECT sdst = {
		l,
		t,
		r - l,
		b - t,
	};
	Gfx_DrawTex(tex, src, &sdst);
}

void Stage_DrawTexArb(Gfx_Tex *tex, RECT *src, POINT_FIXED *p0, POINT_FIXED *p1, POINT_FIXED *p2, POINT_FIXED *p3, fixed_t zoom)
{
	//Get screen-space points
	POINT s0 = {SCREEN_WIDTH2 + (FIXED_MUL(p0->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p0->y, zoom) >> FIXED_SHIFT)};
	POINT s1 = {SCREEN_WIDTH2 + (FIXED_MUL(p1->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p1->y, zoom) >> FIXED_SHIFT)};
	POINT s2 = {SCREEN_WIDTH2 + (FIXED_MUL(p2->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p2->y, zoom) >> FIXED_SHIFT)};
	POINT s3 = {SCREEN_WIDTH2 + (FIXED_MUL(p3->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p3->y, zoom) >> FIXED_SHIFT)};
	
	Gfx_DrawTexArb(tex, src, &s0, &s1, &s2, &s3);
}

//Stage HUD functions and constants
static const fixed_t note_x[8] = {
	//BF
	 FIXED_DEC(26,1),
	 FIXED_DEC(60,1),//+34
	 FIXED_DEC(94,1),
	FIXED_DEC(128,1),
	//Opponent
	FIXED_DEC(-128,1),
	 FIXED_DEC(-94,1),//+34
	 FIXED_DEC(-60,1),
	 FIXED_DEC(-26,1),
};
static const u16 note_key[] = {INPUT_LEFT, INPUT_DOWN, INPUT_UP, INPUT_RIGHT};
static const fixed_t note_y = (32 - SCREEN_HEIGHT2) << FIXED_SHIFT;

void Stage_DrawHealth(u8 i, s8 ox)
{
	//Check if we should use 'dying' frame
	s8 dying;
	if (ox < 0)
		dying = (stage.health >= 18000) * 24;
	else
		dying = (stage.health <= 2000) * 24;
	
	//Get src and dst
	fixed_t hx = (128 << FIXED_SHIFT) * (10000 - stage.health) / 10000;
	RECT src = {
		(i % 5) * 48 + dying,
		16 + (i / 5) * 24,
		24,
		24
	};
	RECT_FIXED dst = {
		hx + ox * FIXED_DEC(11,1) - FIXED_DEC(12,1),
		(SCREEN_HEIGHT2 - 32 + 4 - 12) << FIXED_SHIFT,
		src.w << FIXED_SHIFT,
		src.h << FIXED_SHIFT
	};
	if (stage.downscroll)
		dst.y = -dst.y - dst.h;
	
	//Draw health icon
	Stage_DrawTex(&stage.tex_hud1, &src, &dst, FIXED_MUL(stage.bump, stage.sbump));
}

void Stage_DrawNotes()
{
	//Initialize scroll state
	SectionScroll scroll;
	scroll.start = stage.time_base;
	
	Section *scroll_section = stage.section_base;
	Stage_GetSectionScroll(&scroll, scroll_section);
	
	//Push scroll back until cur_note is properly contained
	while (scroll.start_step > stage.cur_note->pos)
	{
		//Look for previous section
		Section *prev_section = Stage_GetPrevSection(scroll_section);
		if (prev_section == NULL)
			break;
		
		//Push scroll back
		scroll_section = prev_section;
		Stage_GetSectionScroll(&scroll, scroll_section);
		scroll.start -= scroll.length;
	}
	
	//Draw notes
	for (Note *note = stage.cur_note; note->pos != 0xFFFF; note++)
	{
		//Update scroll
		while (note->pos >= scroll_section->end)
		{
			//Push scroll forward
			scroll.start += scroll.length;
			Stage_GetSectionScroll(&scroll, ++scroll_section);
		}
		
		//Get note position
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		fixed_t time = (scroll.start - stage.song_time) + (scroll.length * (note->pos - scroll.start_step) / scroll.length_step);
		fixed_t y = note_y + FIXED_MUL(stage.speed, time * 140);
		
		//Check if went above screen
		if (y < ((-16 - SCREEN_HEIGHT2) << FIXED_SHIFT))
		{
			//Wait for note to exit late time
			if (note->type & NOTE_FLAG_SUSTAIN)
			{
				if (note_fp + stage.late_sus_safe >= stage.note_scroll)
					continue;
			}
			else
			{
				if (note_fp + stage.late_safe >= stage.note_scroll)
					continue;
			}
			
			//Miss note if player's note
			if (!(note->type & (NOTE_FLAG_OPPONENT | NOTE_FLAG_HIT | NOTE_FLAG_MINE)))
			{
				if (stage.kade)
				{
					if (!(note->type & NOTE_FLAG_SUSTAIN))
					{
						//Missed note
						Stage_CutVocal();
						Stage_MissNote();
						stage.health -= (note->type & NOTE_FLAG_SUSTAIN_END) ? 2000 : 1000;
						stage.score -= 1;
					}
					else
					{
						//Missed sustain piece
						Stage_CutVocal();
					}
				}
				else
				{
					//Missed note
					Stage_CutVocal();
					Stage_MissNote();
					stage.health -= 475;
				}
			}
			
			//Update current note
			stage.cur_note++;
		}
		else
		{
			//Don't draw if below screen
			RECT note_src;
			RECT_FIXED note_dst;
			if (y > (((SCREEN_HEIGHT2 + 16) << FIXED_SHIFT) + scroll.size) || note->pos == 0xFFFF)
				break;
			
			//Draw note
			if (note->type & NOTE_FLAG_SUSTAIN)
			{
				//Check for sustain clipping
				fixed_t clip;
				y -= scroll.size;
				if ((note->type & (NOTE_FLAG_OPPONENT | NOTE_FLAG_HIT)) || ((stage.pad_held & note_key[note->type & 0x3]) && (note_fp + stage.late_sus_safe >= stage.note_scroll)))
				{
					clip = ((32 - SCREEN_HEIGHT2) << FIXED_SHIFT) - y;
					if (clip < 0)
						clip = 0;
				}
				else
				{
					clip = 0;
				}
				
				//Draw sustain
				if (note->type & NOTE_FLAG_SUSTAIN_END)
				{
					if (clip < (24 << FIXED_SHIFT))
					{
						note_src.x = 160;
						note_src.y = ((note->type & 0x3) << 5) + 4 + (clip >> FIXED_SHIFT);
						note_src.w = 32;
						note_src.h = 28 - (clip >> FIXED_SHIFT);
						
						note_dst.x = note_x[note->type & 0x7] - FIXED_DEC(16,1);
						note_dst.y = y + clip;
						note_dst.w = note_src.w << FIXED_SHIFT;
						note_dst.h = (note_src.h << FIXED_SHIFT);
						
						if (stage.downscroll)
						{
							note_dst.y = -note_dst.y;
							note_dst.h = -note_dst.h;
						}
						Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
					}
				}
				else
				{
					if (clip < scroll.size)
					{
						note_src.x = 160;
						note_src.y = ((note->type & 0x3) << 5);
						note_src.w = 32;
						note_src.h = 16;
						
						note_dst.x = note_x[note->type & 0x7] - FIXED_DEC(16,1);
						note_dst.y = y + clip;
						note_dst.w = note_src.w << FIXED_SHIFT;
						note_dst.h = scroll.size - clip;
						
						if (stage.downscroll)
							note_dst.y = -note_dst.y - note_dst.h;
						Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
					}
				}
			}
			else if (note->type & NOTE_FLAG_MINE)
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note body
				note_src.x = 192 + ((note->type & 0x1) << 5);
				note_src.y = (note->type & 0x2) << 4;
				note_src.w = 32;
				note_src.h = 32;
				
				note_dst.x = note_x[note->type & 0x7] - FIXED_DEC(16,1);
				note_dst.y = y - FIXED_DEC(16,1);
				note_dst.w = note_src.w << FIXED_SHIFT;
				note_dst.h = note_src.h << FIXED_SHIFT;
				
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				
				if (stage.stage_id == StageId_Clwn_4)
				{
					//Draw note halo
					note_src.x = 160;
					note_src.y = 128 + (((frame_count >> 1) & 0x3) << 3);
					note_src.w = 32;
					note_src.h = 8;
					
					note_dst.y -= FIXED_DEC(6,1);
					note_dst.h >>= 2;
					
					Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				}
				else
				{
					//Draw note fire
					note_src.x = 192 + (((frame_count >> 1) & 0x1) << 5);
					note_src.y = 64 + (((frame_count >> 1) & 0x2) * 24);
					note_src.w = 32;
					note_src.h = 48;
					
					if (stage.downscroll)
					{
						note_dst.y += note_dst.h;
						note_dst.h = note_dst.h * -3 / 2;
					}
					else
					{
						note_dst.h = note_dst.h * 3 / 2;
					}
					Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				}
			}
			else
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note
				note_src.x = 32 + ((note->type & 0x3) << 5);
				note_src.y = 0;
				note_src.w = 32;
				note_src.h = 32;
				
				note_dst.x = note_x[note->type & 0x7] - FIXED_DEC(16,1);
				note_dst.y = y - FIXED_DEC(16,1);
				note_dst.w = note_src.w << FIXED_SHIFT;
				note_dst.h = note_src.h << FIXED_SHIFT;
				
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			}
		}
	}
}

//Stage loads
void Stage_LoadPlayer(void)
{
	//Load player character
	Character_Free(stage.player);
	stage.player = stage.stage_def->pchar.new(stage.stage_def->pchar.x, stage.stage_def->pchar.y);
}

void Stage_LoadOpponent(void)
{
	//Load opponent character
	Character_Free(stage.opponent);
	stage.opponent = stage.stage_def->ochar.new(stage.stage_def->ochar.x, stage.stage_def->ochar.y);
}

void Stage_LoadGirlfriend(void)
{
	//Load girlfriend character
	Character_Free(stage.gf);
	stage.gf = stage.stage_def->gchar.new(stage.stage_def->gchar.x, stage.stage_def->gchar.y);
}

void Stage_LoadStage(void)
{
	//Load back
	if (stage.back != NULL)
		stage.back->free(stage.back);
	stage.back = stage.stage_def->back();
}

void Stage_LoadChart(void)
{
	//Load stage data
	char chart_path[64];
	if (stage.stage_def->week & 0x80)
	{
		//Use mod path convention
		static const char *mod_format[] = {
			"\\KAPI\\KAPI.%d%c.CHT;1", //Kapi
			"\\CLWN\\CLWN.%d%c.CHT;1" //Tricky
		};
		
		sprintf(chart_path, mod_format[stage.stage_def->week & 0x7F], stage.stage_def->week_song, "ENH"[stage.stage_diff]);
	}
	else
	{
		//Use standard path convention
		sprintf(chart_path, "\\WEEK%d\\%d.%d%c.CHT;1", stage.stage_def->week, stage.stage_def->week, stage.stage_def->week_song, "ENH"[stage.stage_diff]);
	}
	
	if (stage.chart_data != NULL)
		Mem_Free(stage.chart_data);
	stage.chart_data = IO_Read(chart_path);
	stage.sections = (Section*)((u8*)stage.chart_data + 2);
	stage.notes = (Note*)((u8*)stage.chart_data + *((u16*)stage.chart_data));
	
	stage.cur_section = stage.sections;
	stage.cur_note = stage.notes;
	
	stage.speed = stage.stage_def->speed[stage.stage_diff];
	
	stage.step_crochet = 0;
	stage.time_base = 0;
	stage.step_base = 0;
	stage.section_base = stage.cur_section;
	Stage_ChangeBPM(stage.cur_section->flag & SECTION_FLAG_BPM_MASK, 0);
}

void Stage_LoadMusic(void)
{
	//Find music file and begin seeking to it
	Audio_GetXAFile(&stage.music_file, stage.stage_def->music_track);
	IO_SeekFile(&stage.music_file);
	
	//Initialize music state
	stage.note_scroll = FIXED_DEC(-8 * 24,1);
}

void Stage_LoadState(void)
{
	//Initialize stage state
	stage.flag = 0;
	
	stage.gf_speed = 1 << 2;
	
	stage.health = 10000;
	stage.combo = 0;
	stage.score = 0;
	strcpy(stage.score_text, "0");
	
	stage.state = StageState_Play;
	
	ObjectList_Free(&stage.objlist_fg);
	stage.objlist_fg = NULL;
	ObjectList_Free(&stage.objlist_bg);
	stage.objlist_bg = NULL;
}

//Stage functions
void Stage_Load(StageId id, StageDiff difficulty, boolean story)
{
	//Get stage definition
	stage.stage_def = &stage_defs[stage.stage_id = id];
	stage.stage_diff = difficulty;
	stage.story = story;
	
	//Load HUD textures
	Gfx_LoadTex(&stage.tex_hud0, IO_Read("\\STAGE\\HUD0.TIM;1"), GFX_LOADTEX_FREE);
	Gfx_LoadTex(&stage.tex_hud1, IO_Read("\\STAGE\\HUD1.TIM;1"), GFX_LOADTEX_FREE);
	
	//Load stage background
	Stage_LoadStage();
	
	//Load characters
	Stage_LoadPlayer();
	Stage_LoadOpponent();
	Stage_LoadGirlfriend();
	
	//Load stage chart
	Stage_LoadChart();
	
	//Initialize stage state
	stage.story = story;
	stage.flag = 0;
	
	stage.pad_held = stage.pad_press = 0;
	memset(stage.arrow_hitan, 0, sizeof(stage.arrow_hitan));
	
	Stage_LoadState();
	
	//Initialize camera
	if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
		Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
	else
		Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
	stage.camera.x = stage.camera.tx;
	stage.camera.y = stage.camera.ty;
	stage.camera.zoom = stage.camera.tz;
	
	stage.bump = FIXED_UNIT;
	stage.sbump = FIXED_UNIT;
	
	//Load music
	Stage_LoadMusic();
}

void Stage_Unload()
{
	//Unload stage background
	if (stage.back != NULL)
		stage.back->free(stage.back);
	stage.back = NULL;
	
	//Unload stage data
	Mem_Free(stage.chart_data);
	stage.chart_data = NULL;
	
	//Free objects
	ObjectList_Free(&stage.objlist_fg);
	ObjectList_Free(&stage.objlist_bg);
	
	//Free characters
	Character_Free(stage.player);
	stage.player = NULL;
	Character_Free(stage.opponent);
	stage.opponent = NULL;
	Character_Free(stage.gf);
	stage.gf = NULL;
}

void Stage_NextLoad()
{
	u8 load = stage.stage_def->next_load;
	if (load == 0)
	{
		//Do stage transition if full reload
		stage.trans = StageTrans_NextSong;
		Trans_Start();
	}
	else
	{
		//Get stage definition
		stage.stage_def = &stage_defs[stage.stage_id = stage.stage_def->next_stage];
		
		//Load stage background
		if (load & STAGE_LOAD_STAGE)
			Stage_LoadStage();
		
		//Load characters
		if (load & STAGE_LOAD_PLAYER)
			Stage_LoadPlayer();
		if (load & STAGE_LOAD_OPPONENT)
			Stage_LoadOpponent();
		if (load & STAGE_LOAD_GIRLFRIEND)
			Stage_LoadGirlfriend();
		
		//Load stage chart
		Stage_LoadChart();
		
		//Initialize stage state
		Stage_LoadState();
		
		//Load music
		Stage_LoadMusic();
	}
}

void Stage_Tick()
{
	SeamLoad:;
	
	//Tick transition
	if (pad_state.press & PAD_START)
	{
		//Return to menu
		stage.trans = (stage.state == StageState_Play) ? StageTrans_Menu : StageTrans_Reload;
		Trans_Start();
	}
	
	if (Trans_Tick())
	{
		switch (stage.trans)
		{
			case StageTrans_Menu:
				//Load appropriate menu
				Stage_Unload();
				
				LoadScr_Start();
				if (stage.stage_id <= StageId_LastVanilla)
				{
					if (stage.story)
						Menu_Load(MenuPage_Story);
					else
						Menu_Load(MenuPage_Freeplay);
				}
				else
				{
					Menu_Load(MenuPage_Mods);
				}
				LoadScr_End();
				
				gameloop = GameLoop_Menu;
				return;
			case StageTrans_NextSong:
				//Load next song
				Stage_Unload();
				
				LoadScr_Start();
				Stage_Load(stage.stage_def->next_stage, stage.stage_diff, stage.story);
				LoadScr_End();
				break;
			case StageTrans_Reload:
				//Reload song
				Stage_Unload();
				
				LoadScr_Start();
				Stage_Load(stage.stage_id, stage.stage_diff, stage.story);
				LoadScr_End();
				break;
		}
	}
	
	switch (stage.state)
	{
		case StageState_Play:
		{
			//Clear per-frame flags
			stage.flag &= ~(STAGE_FLAG_JUST_STEP | STAGE_FLAG_SCORE_REFRESH);
			
			//Get song position
			boolean playing;
			
			if (stage.note_scroll < 0)
			{
				//Song hasn't started yet
				fixed_t next_scroll = stage.note_scroll + stage.step_crochet / 60; //TODO: PAL
				
				//3 2 1 GO - pre song start
				
				//Update song
				if (next_scroll >= 0)
				{
					//Song has started
					playing = true;
					Audio_PlayXA_File(&stage.music_file, 0x40, stage.stage_def->music_channel, 0);
					stage.note_scroll = 0;
					stage.song_time = 0;
					stage.flag |= STAGE_FLAG_JUST_STEP;
				}
				else
				{
					//Still scrolling
					playing = false;
					if (((stage.note_scroll / 12) & FIXED_UAND) != ((next_scroll / 12) & FIXED_UAND))
						stage.flag |= STAGE_FLAG_JUST_STEP;
					stage.note_scroll = next_scroll;
					
					//Extrapolate song time from note scroll
					stage.song_time = FIXED_DIV(stage.note_scroll, stage.step_crochet);
				}
			}
			else if (Audio_PlayingXA())
			{
				RecalcSongPosition:;
				//Get playing song position
				fixed_t song_time = (Audio_TellXA_Milli() << FIXED_SHIFT) / 1000;
				if (song_time < stage.time_base)
					song_time = stage.time_base;
				
				playing = true;
				
				//Update scroll
				fixed_t next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(song_time - stage.time_base, stage.step_crochet);
				
				if (next_scroll > stage.note_scroll) //Skipping?
				{
					if (((stage.note_scroll / 12) & FIXED_UAND) != ((next_scroll / 12) & FIXED_UAND))
						stage.flag |= STAGE_FLAG_JUST_STEP;
					stage.note_scroll = next_scroll;
					stage.song_time = song_time;
				}
			}
			else
			{
				//Song has ended
				fixed_t next_scroll = stage.note_scroll + stage.step_crochet / 60; //TODO: PAL
				playing = false;
				
				//Update scroll
				stage.note_scroll = next_scroll;
				
				//Extrapolate song time from note scroll
				stage.song_time = stage.time_base + FIXED_DIV(stage.note_scroll - ((fixed_t)stage.step_base << FIXED_SHIFT), stage.step_crochet);
				
				//Transition to menu or next song
				if (stage.story && stage.stage_def->next_stage != stage.stage_id)
				{
					Stage_NextLoad();
					goto SeamLoad;
				}
				else
				{
					stage.trans = StageTrans_Menu;
					Trans_Start();
				}
			}
			
			//Get song step
			stage.song_step = (stage.note_scroll >> FIXED_SHIFT) / 12;
			
			//Update section
			if (stage.note_scroll >= 0)
			{
				//Check if current section has ended
				u16 end = stage.cur_section->end;
				if ((stage.note_scroll >> FIXED_SHIFT) >= end)
				{
					//Increment section pointer
					stage.cur_section++;
					
					//Update BPM
					u16 next_bpm = stage.cur_section->flag & SECTION_FLAG_BPM_MASK;
					Stage_ChangeBPM(next_bpm, end);
					stage.section_base = stage.cur_section;
					
					//Recalculate song position based off new BPM
					if (playing)
						goto RecalcSongPosition;
				}
			}
			
			//Handle bump
			stage.bump = FIXED_UNIT + FIXED_MUL(stage.bump - FIXED_UNIT, FIXED_DEC(95,100));
			stage.sbump = FIXED_UNIT + FIXED_MUL(stage.sbump - FIXED_UNIT, FIXED_DEC(60,100));
			
			if (playing && (stage.flag & STAGE_FLAG_JUST_STEP))
			{
				//Check if screen should bump
				boolean is_bump_step = (stage.song_step & 0xF) == 0;
				
				//M.I.L.F bumps
				if (stage.stage_id == StageId_4_3 && stage.song_step >= (168 << 2) && stage.song_step < (200 << 2))
					is_bump_step = (stage.song_step & 0x3) == 0;
				
				//Bump screen
				if (is_bump_step)
					stage.bump = FIXED_DEC(103,100);
				
				//Bump health every 4 steps
				if ((stage.song_step & 0x3) == 0)
					stage.sbump = FIXED_DEC(103,100);
			}
			
			//Scroll camera
			if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
				Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
			else
				Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
			Stage_ScrollCamera();
			
			//Handle player note presses
			#ifndef STAGE_PERFECT
				if (playing)
				{
					stage.pad_held = pad_state.held;
					stage.pad_press = pad_state.press;
					
					if (pad_state.press & INPUT_LEFT)
						Stage_NoteCheck(0);
					if (pad_state.press & INPUT_DOWN)
						Stage_NoteCheck(1);
					if (pad_state.press & INPUT_UP)
						Stage_NoteCheck(2);
					if (pad_state.press & INPUT_RIGHT)
						Stage_NoteCheck(3);
					
					if (pad_state.held & INPUT_LEFT)
						Stage_SustainCheck(0);
					if (pad_state.held & INPUT_DOWN)
						Stage_SustainCheck(1);
					if (pad_state.held & INPUT_UP)
						Stage_SustainCheck(2);
					if (pad_state.held & INPUT_RIGHT)
						Stage_SustainCheck(3);
				}
				else
				{
					stage.pad_held = 0;
					stage.pad_press = 0;
				}
			#endif
			
			//Process notes
			for (Note *note = stage.cur_note;; note++)
			{
				if (note->pos > (stage.note_scroll >> FIXED_SHIFT))
					break;
				
				//Opponent note hits
				if (playing && (note->type & NOTE_FLAG_OPPONENT) && !(note->type & NOTE_FLAG_HIT))
				{
					//Opponent hits note
					Stage_StartVocal();
					stage.opponent->set_anim(stage.opponent, note_anims[note->type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
					note->type |= NOTE_FLAG_HIT;
				}
			}
			
			#ifdef STAGE_PERFECT
				//Do perfect note checks
				if (playing)
				{
					u8 hit[4] = {0, 0, 0, 0};
					for (Note *note = stage.cur_note;; note++)
					{
						//Check if note can be hit
						fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
						if (note_fp - stage.early_safe > stage.note_scroll)
							break;
						if (note_fp + stage.late_safe < stage.note_scroll)
							continue;
						if (note->type & (NOTE_FLAG_OPPONENT | NOTE_FLAG_MINE))
							continue;
						
						//Handle note hit
						if (!(note->type & NOTE_FLAG_SUSTAIN))
						{
							if (note->type & NOTE_FLAG_HIT)
								continue;
							if (stage.note_scroll >= note_fp)
								hit[note->type & 0x3] |= 1;
							else
								hit[note->type & 0x3] |= 2;
						}
						else if (!(hit[note->type & 0x3] & 2))
							hit[note->type & 0x3] |= 4;
					}
					
					//Handle input
					stage.pad_held = 0;
					stage.pad_press = 0;
					
					for (u8 i = 0; i < 4; i++)
					{
						if (hit[i] & 1)
						{
							stage.pad_press |= note_key[i];
							Stage_NoteCheck(i);
						}
						if (hit[i] & 4)
						{
							stage.pad_held |= note_key[i];
							Stage_SustainCheck(i);
						}
					}
				}
			#endif
			
			//Display score
			RECT score_src = {80, 224, 40, 10};
			RECT_FIXED score_dst = {FIXED_DEC(14,1), (SCREEN_HEIGHT2 - 42) << FIXED_SHIFT, FIXED_DEC(40,1), FIXED_DEC(10,1)};
			if (stage.downscroll)
				score_dst.y = -score_dst.y - score_dst.h;
			
			Stage_DrawTex(&stage.tex_hud0, &score_src, &score_dst, stage.bump);
			
			//Get string representing number
			if (stage.flag & STAGE_FLAG_SCORE_REFRESH)
			{
				if (stage.score != 0)
					sprintf(stage.score_text, "%d0", stage.score);
				else
					strcpy(stage.score_text, "0");
			}
			
			//Draw number
			score_src.y = 240;
			score_src.w = 8;
			score_dst.x += FIXED_DEC(40,1);
			score_dst.w = FIXED_DEC(8,1);
			
			for (const char *p = stage.score_text; ; p++)
			{
				//Get character
				char c = *p;
				if (c == '\0')
					break;
				
				//Draw character
				if (c == '-')
					score_src.x = 160;
				else //Should be a number
					score_src.x = 80 + ((c - '0') << 3);
				
				Stage_DrawTex(&stage.tex_hud0, &score_src, &score_dst, stage.bump);
				
				//Move character right
				score_dst.x += FIXED_DEC(7,1);
			}
			
			//Perform health checks
			if (stage.health <= 0)
			{
				//Player has died
				stage.health = 0;
				stage.state = StageState_Dead;
			}
			if (stage.health > 20000)
				stage.health = 20000;
			
			//Draw health heads
			Stage_DrawHealth(stage.player->health_i,    1);
			Stage_DrawHealth(stage.opponent->health_i, -1);
			
			//Draw health bar
			RECT health_fill = {0, 0, 256 - (256 * stage.health / 20000), 8};
			RECT health_back = {0, 8, 256, 8};
			RECT_FIXED health_dst = {FIXED_DEC(-128,1), (SCREEN_HEIGHT2 - 32) << FIXED_SHIFT, 0, FIXED_DEC(8,1)};
			if (stage.downscroll)
				health_dst.y = -health_dst.y - health_dst.h;
			
			health_dst.w = health_fill.w << FIXED_SHIFT;
			Stage_DrawTex(&stage.tex_hud1, &health_fill, &health_dst, stage.bump);
			health_dst.w = health_back.w << FIXED_SHIFT;
			Stage_DrawTex(&stage.tex_hud1, &health_back, &health_dst, stage.bump);
			
			//Draw stage notes
			Stage_DrawNotes();
			
			//Draw note HUD
			RECT note_src = {0, 0, 32, 32};
			RECT_FIXED note_dst = {0, note_y - FIXED_DEC(16,1), FIXED_DEC(32,1), FIXED_DEC(32,1)};
			if (stage.downscroll)
				note_dst.y = -note_dst.y - note_dst.h;
			
			for (u8 i = 0; i < 4; i++)
			{
				//BF
				note_dst.x = note_x[i] - FIXED_DEC(16,1);
				
				if (stage.arrow_hitan[i] != 0)
				{
					//Play hit animation
					note_src.x = (i + 1) << 5;
					note_src.y = 128 - (((stage.arrow_hitan[i] + 1) >> 1) << 5);
					stage.arrow_hitan[i]--;
				}
				else
				{
					note_src.x = 0;
					note_src.y = i << 5;
				}
				Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				
				//Opponent
				note_dst.x = note_x[i | 0x4] - FIXED_DEC(16,1);
				
				note_src.x = 0;
				note_src.y = i << 5;
				Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			}
			
			//Hardcoded stage stuff
			switch (stage.stage_id)
			{
				case StageId_1_2: //Fresh GF bop
					switch (stage.song_step)
					{
						case 16 << 2:
							stage.gf_speed = 2 << 2;
							break;
						case 48 << 2:
							stage.gf_speed = 1 << 2;
							break;
						case 80 << 2:
							stage.gf_speed = 2 << 2;
							break;
						case 112 << 2:
							stage.gf_speed = 1 << 2;
							break;
					}
					break;
				default:
					break;
			}
			
			//Draw stage foreground
			if (stage.back->draw_fg != NULL)
				stage.back->draw_fg(stage.back);
			
			//Tick foreground objects
			ObjectList_Tick(&stage.objlist_fg);
			
			//Tick characters
			stage.player->tick(stage.player);
			stage.opponent->tick(stage.opponent);
			
			//Draw stage middle
			if (stage.back->draw_md != NULL)
				stage.back->draw_md(stage.back);
			
			//Tick girlfriend
			stage.gf->tick(stage.gf);
			
			//Tick background objects
			ObjectList_Tick(&stage.objlist_bg);
			
			//Draw stage background
			if (stage.back->draw_bg != NULL)
				stage.back->draw_bg(stage.back);
			
			/*
			//Draw curtains
			RECT curtain_src = {0, 0, 128, 256};
			RECT_FIXED curtain1_dst = {
				FIXED_DEC(-300,1) - stage.camera.x,
				FIXED_DEC(-350,1) - stage.camera.y,
				FIXED_DEC(200,1),
				FIXED_DEC(400,1)
			};
			RECT_FIXED curtainr_dst = {
				FIXED_DEC(300,1) - stage.camera.x,
				FIXED_DEC(-350,1) - stage.camera.y,
				FIXED_DEC(-200,1),
				FIXED_DEC(400,1)
			};
			
			Stage_DrawTex(&stage.tex_back1, &curtain_src, &curtain1_dst, FIXED_MUL(FIXED_DEC(95,100), stage.bump));
			Stage_DrawTex(&stage.tex_back1, &curtain_src, &curtainr_dst, FIXED_MUL(FIXED_DEC(95,100), stage.bump));
			
			//Draw stage
			RECT stagel_src = {0, 0, 256, 128};
			RECT_FIXED stage1_dst = {
				FIXED_DEC(-500,1) - stage.camera.x,
				FIXED_DEC(32,1) - stage.camera.y,
				FIXED_DEC(500,1),
				FIXED_DEC(250,1)
			};
			RECT stager_src = {0, 128, 256, 128};
			RECT_FIXED stager_dst = {
				-stage.camera.x,
				FIXED_DEC(32,1) - stage.camera.y,
				FIXED_DEC(500,1),
				FIXED_DEC(250,1)
			};
			
			Stage_DrawTex(&stage.tex_back0, &stagel_src, &stage1_dst, stage.bump);
			Stage_DrawTex(&stage.tex_back0, &stager_src, &stager_dst, stage.bump);
			*/
			break;
		}
		case StageState_Dead: //Start BREAK animation and reading extra data from CD
		{
			//Stop music immediately
			Audio_StopXA();
			
			//Unload stage data
			Mem_Free(stage.chart_data);
			stage.chart_data = NULL;
			
			//Free background
			stage.back->free(stage.back);
			stage.back = NULL;
			
			//Free objects
			ObjectList_Free(&stage.objlist_fg);
			ObjectList_Free(&stage.objlist_bg);
			
			//Free opponent and girlfriend
			Character_Free(stage.opponent);
			stage.opponent = NULL;
			Character_Free(stage.gf);
			stage.gf = NULL;
			
			//Reset stage state
			stage.flag = 0;
			stage.bump = stage.sbump = FIXED_UNIT;
			
			//Change background colour to black
			Gfx_SetClear(0, 0, 0);
			
			//Run death animation, focus on player, and change state
			stage.player->set_anim(stage.player, PlayerAnim_Dead0);
			
			Stage_FocusCharacter(stage.player, 0);
			stage.song_time = 0;
			
			stage.state = StageState_DeadLoad;
		}
	//Fallthrough
		case StageState_DeadLoad:
		{
			//Scroll camera and tick player
			if (stage.song_time < FIXED_UNIT)
				stage.song_time += FIXED_UNIT / 60;
			stage.camera.td = FIXED_DEC(-2, 100) + FIXED_MUL(stage.song_time, FIXED_DEC(45, 1000));
			if (stage.camera.td > 0)
				Stage_ScrollCamera();
			stage.player->tick(stage.player);
			
			//Drop mic and change state if CD has finished reading and animation has ended
			if (IO_IsReading() || stage.player->animatable.anim != PlayerAnim_Dead1)
				break;
			
			stage.player->set_anim(stage.player, PlayerAnim_Dead2);
			stage.camera.td = FIXED_DEC(25, 1000);
			stage.state = StageState_DeadDrop;
			break;
		}
		case StageState_DeadDrop:
		{
			//Scroll camera and tick player
			Stage_ScrollCamera();
			stage.player->tick(stage.player);
			
			//Enter next state once mic has been dropped
			if (stage.player->animatable.anim == PlayerAnim_Dead3)
			{
				stage.state = StageState_DeadRetry;
				Audio_PlayXA_Track(XA_GameOver, 0x40, 1, true);
			}
			break;
		}
		case StageState_DeadRetry:
		{
			//Randomly twitch
			if (stage.player->animatable.anim == PlayerAnim_Dead3)
			{
				if (RandomRange(0, 29) == 0)
					stage.player->set_anim(stage.player, PlayerAnim_Dead4);
				if (RandomRange(0, 29) == 0)
					stage.player->set_anim(stage.player, PlayerAnim_Dead5);
			}
			
			//Scroll camera and tick player
			Stage_ScrollCamera();
			stage.player->tick(stage.player);
			break;
		}
		default:
			break;
	}
}
