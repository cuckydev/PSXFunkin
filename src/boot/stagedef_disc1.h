#include "week1/week1.h"
#include "week2/week2.h"

static const StageDef stage_defs[StageId_Max] = {
	//Week 1
	{ //StageId_1_1 (Bopeebo)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(1,1),FIXED_DEC(13,10)},
		XA_Bopeebo, 0
	},
	{ //StageId_1_2 (Fresh)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(13,10),FIXED_DEC(18,10)},
		XA_Fresh, 2
	},
	{ //StageId_1_3 (Dadbattle)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		{FIXED_DEC(13,10),FIXED_DEC(15,10),FIXED_DEC(23,10)},
		XA_Dadbattle, 0
	},
	{ //StageId_1_4 (Tutorial)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(1,1),FIXED_DEC(1,1)},
		XA_Tutorial, 2
	},
	
	{ //StageId_2_1 (Spookeez)
		//Overlay
		"\\WEEK2\\WEEK2.EXE;1", Week2_SetPtr,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(17,10),FIXED_DEC(24,10)},
		XA_Spookeez, 0
	},
	{ //StageId_2_2 (South)
		//Overlay
		"\\WEEK2\\WEEK2.EXE;1", Week2_SetPtr,
		
		//Song info
		{FIXED_DEC(11,10),FIXED_DEC(15,10),FIXED_DEC(22,10)},
		XA_South, 2
	},
	{ //StageId_2_3 (Monster)
		//Overlay
		"\\WEEK2\\WEEK2.EXE;1", Week2_SetPtr,
		
		//Song info
		{FIXED_DEC(13,10),FIXED_DEC(13,10),FIXED_DEC(16,10)},
		XA_Monster, 0
	},
};
