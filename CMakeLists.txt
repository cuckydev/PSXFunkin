cmake_minimum_required(VERSION 3.16.3)

option(STDMEM "Use standard libc memory allocators instead of the fast custom one" OFF)
option(GLES "Use OpenGL ES 2.0 instead of OpenGL Core 3.2" OFF)

project(funkin LANGUAGES C)

add_executable(funkin WIN32 # The `WIN32` here is so we don't get a command prompt window on Windows
	"src/main.c"
	"src/main.h"
	"src/mem.c"
	"src/mem.h"
	"src/mutil.c"
	"src/mutil.h"
	"src/random.c"
	"src/random.h"
	"src/archive.c"
	"src/archive.h"
	"src/font.c"
	"src/font.h"
	"src/trans.c"
	"src/trans.h"
	"src/loadscr.c"
	"src/loadscr.h"
	"src/menu.c"
	"src/menu.h"
	"src/stage.c"
	"src/stage.h"
	"src/pc/psx.c"
	"src/psx.h"
	"src/pc/io.c"
	"src/io.h"
	"src/pc/gfx.c"
	"src/gfx.h"
	"src/pc/audio.c"
	"src/audio.h"
	"src/pc/pad.c"
	"src/pad.h"
	"src/pc/timer.c"
	"src/timer.h"
	"src/stage/dummy.c"
	"src/stage/dummy.h"
	"src/stage/week1.c"
	"src/stage/week1.h"
	"src/stage/week2.c"
	"src/stage/week2.h"
	"src/stage/week3.c"
	"src/stage/week3.h"
	"src/stage/week4.c"
	"src/stage/week4.h"
	"src/stage/week5.c"
	"src/stage/week5.h"
	"src/stage/week7.c"
	"src/stage/week7.h"
	"src/animation.c"
	"src/animation.h"
	"src/character.c"
	"src/character.h"
	"src/character/bf.c"
	"src/character/bf.h"
	"src/character/bfweeb.c"
	"src/character/bfweeb.h"
	"src/character/gf.c"
	"src/character/gf.h"
	"src/character/speaker.c"
	"src/character/speaker.h"
	"src/character/dad.c"
	"src/character/dad.h"
	"src/character/spook.c"
	"src/character/spook.h"
	"src/character/pico.c"
	"src/character/pico.h"
	"src/character/mom.c"
	"src/character/mom.h"
	"src/character/xmasp.c"
	"src/character/xmasp.h"
	"src/character/senpai.c"
	"src/character/senpai.h"
	"src/character/tank.c"
	"src/character/tank.h"
	"src/object.c"
	"src/object.h"
	"src/object/combo.c"
	"src/object/combo.h"
)

if (GLES)
	target_compile_definitions(funkin PRIVATE PSXF_GLES)

	target_link_libraries(funkin PRIVATE GLESv2)
	# Raspberry Pi Broadcom GLESv2 driver.
	# Would be nice to test someday, but it doesn't work during X sessions,
	# and this port cannot be ran from the terminal (yet).
	# For now, we'll just have to settle with the open-source Mesa driver.
	#target_link_libraries(funkin PRIVATE m "/opt/vc/lib/libbrcmGLESv2.so")
else()
	target_sources(funkin PRIVATE
		"src/pc/glad/glad.c"
		"src/pc/glad/glad.h"
	)

	target_link_libraries(funkin PRIVATE ${CMAKE_DL_LIBS})
endif()

# Make it so the code knows it's targetting PC
target_compile_definitions(funkin PRIVATE PSXF_PC)

# Use standard memory allocators if requested to
if(STDMEM)
	target_compile_definitions(funkin PRIVATE PSXF_STDMEM)
endif()

# Link GLFW3
find_package(glfw3 REQUIRED)
target_link_libraries(funkin PRIVATE glfw)
