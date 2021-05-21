#ifndef _STAGE_H
#define _STAGE_H

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

//Stage functions
void Stage_Load(StageId id, StageDiff difficulty);
void Stage_Unload();
void Stage_Tick();

#endif
