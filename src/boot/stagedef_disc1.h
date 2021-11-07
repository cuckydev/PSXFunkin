#include "week1/week1.h"
#include "week2/week2.h"
#include "week3/week3.h"
#include "week4/week4.h"

static const StageDef stage_defs[StageId_Max] = {
	//Week 1
	{ //StageId_1_1 (Bopeebo)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		XA_Bopeebo, 0
	},
	{ //StageId_1_2 (Fresh)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		XA_Fresh, 2
	},
	{ //StageId_1_3 (Dadbattle)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		XA_Dadbattle, 0
	},
	{ //StageId_1_4 (Tutorial)
		//Overlay
		"\\WEEK1\\WEEK1.EXE;1", Week1_SetPtr,
		
		//Song info
		XA_Tutorial, 2
	},
	
	{ //StageId_2_1 (Spookeez)
		//Overlay
		"\\WEEK2\\WEEK2.EXE;1", Week2_SetPtr,
		
		//Song info
		XA_Spookeez, 0
	},
	{ //StageId_2_2 (South)
		//Overlay
		"\\WEEK2\\WEEK2.EXE;1", Week2_SetPtr,
		
		//Song info
		XA_South, 2
	},
	{ //StageId_2_3 (Monster)
		//Overlay
		"\\WEEK2\\WEEK2.EXE;1", Week2_SetPtr,
		
		//Song info
		XA_Monster, 0
	},
	
	{ //StageId_3_1 (Pico)
		//Overlay
		"\\WEEK3\\WEEK3.EXE;1", Week3_SetPtr,
		
		//Song info
		XA_Pico, 0
	},
	{ //StageId_3_2 (Philly Nice)
		//Overlay
		"\\WEEK3\\WEEK3.EXE;1", Week3_SetPtr,
		
		//Song info
		XA_Philly, 2
	},
	{ //StageId_3_3 (Blammed)
		//Overlay
		"\\WEEK3\\WEEK3.EXE;1", Week3_SetPtr,
		
		//Song info
		XA_Blammed, 0
	},
	
	{ //StageId_4_1 (Satin Panties)
		//Overlay
		"\\WEEK4\\WEEK4.EXE;1", Week4_SetPtr,
		
		//Song info
		XA_SatinPanties, 0
	},
	{ //StageId_4_2 (High)
		//Overlay
		"\\WEEK4\\WEEK4.EXE;1", Week4_SetPtr,
		
		//Song info
		XA_High, 2
	},
	{ //StageId_4_3 (MILF)
		//Overlay
		"\\WEEK4\\WEEK4.EXE;1", Week4_SetPtr,
		
		//Song info
		XA_MILF, 0
	},
	{ //StageId_4_4 (Test)
		//Overlay
		"\\WEEK4\\WEEK4.EXE;1", Week4_SetPtr,
		
		//Song info
		XA_Test, 2
	},
};
