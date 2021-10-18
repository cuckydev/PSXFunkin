/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _GL_H
#define _GL_H

//GL constants
#define PSXF_GL_MODERN 0
#define PSXF_GL_LEGACY 1
#define PSXF_GL_ES 2

//GL functions
void GL_Init(void);
void GL_Quit(void);
void GL_Flip(void);

#endif
