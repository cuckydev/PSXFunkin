#include "week1/week1.h"

static const StageDef stage_defs[StageId_Max] = {
	//Week 1
	{ //StageId_1_1 (Bopeebo)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(1,1),FIXED_DEC(13,10)},
		1,
		XA_Bopeebo, 0,
	},
	{ //StageId_1_2 (Fresh)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(13,10),FIXED_DEC(18,10)},
		2,
		XA_Fresh, 2,
	},
	{ //StageId_1_3 (Dadbattle)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		{FIXED_DEC(13,10),FIXED_DEC(15,10),FIXED_DEC(23,10)},
		3,
		XA_Dadbattle, 0,
	},
	{ //StageId_1_4 (Tutorial)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(1,1),FIXED_DEC(1,1)},
		4,
		XA_Tutorial, 2,
	},
};
