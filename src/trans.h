#ifndef _TRANS_H
#define _TRANS_H

#include "psx.h"

//Transition functions
void Trans_Set(void);
void Trans_Clear(void);
void Trans_Start(void);
boolean Trans_Tick(void);
boolean Trans_Idle(void);

#endif
