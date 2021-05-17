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
	CharAnim_Left,  CharAnim_LeftMiss,
	CharAnim_Down,  CharAnim_DownMiss,
	CharAnim_Up,    CharAnim_UpMiss,
	CharAnim_Right, CharAnim_RightMiss,
	CharAnim_Peace,
	
	CharAnim_Max
} CharAnim;

typedef enum
{
	StageId_1_1, //Bopeebo
	StageId_1_2, //Fresh
	StageId_1_3, //Dadbattle
	
	StageId_4_1, //Satin Panties
	StageId_4_2, //High
	StageId_4_3, //MILF
	
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
