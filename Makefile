TARGET = funkin
TYPE = ps-exe

SRCS = src/boot/main.c \
       src/boot/mutil.c \
       src/boot/random.c \
       src/boot/archive.c \
       src/boot/font.c \
       src/boot/trans.c \
       src/boot/loadscr.c \
       src/boot/menu.c \
       src/boot/stage.c \
       src/boot/psx/psx.c \
       src/boot/psx/io.c \
       src/boot/psx/gfx.c \
       src/boot/psx/audio.c \
       src/boot/psx/pad.c \
       src/boot/psx/timer.c \
       src/boot/psx/movie.c \
       src/boot/animation.c \
       src/boot/character.c \
       src/boot/object.c \
       src/boot/object/combo.c \
       src/boot/object/splash.c \
       src/menu/menu.c \
       src/week1/week1.c \
       src/week2/week2.c \
       src/week3/week3.c \
       mips/common/crt0/crt0.s

OVERLAYSCRIPT  ?= overlay.ld
OVERLAYSECTION ?= .menu .week1 .week2 .week3

CPPFLAGS += -Wall -Wextra -pedantic -Isrc/ -mno-check-zero-division
LDFLAGS += -Wl,--start-group
# TODO: remove unused libraries
LDFLAGS += -lapi
#LDFLAGS += -lc
LDFLAGS += -lc2
#LDFLAGS += -lcard
LDFLAGS += -lcd
#LDFLAGS += -lcomb
LDFLAGS += -lds
LDFLAGS += -letc
LDFLAGS += -lgpu
#LDFLAGS += -lgs
#LDFLAGS += -lgte
#LDFLAGS += -lgun
#LDFLAGS += -lhmd
#LDFLAGS += -lmath
#LDFLAGS += -lmcrd
#LDFLAGS += -lmcx
LDFLAGS += -lpad
LDFLAGS += -lpress
#LDFLAGS += -lsio
LDFLAGS += -lsnd
LDFLAGS += -lspu
#LDFLAGS += -ltap
LDFLAGS += -flto -Wl,--end-group

include mips/common.mk

all: \
	iso/menu/menu.exe \
	iso/week1/week1.exe \
	iso/week2/week2.exe \
	iso/week3/week3.exe \

iso/%.exe:
	tools/funkinoverlaypak/funkinoverlaypak $@ $^

iso/menu/menu.exe: Overlay.menu iso/menu/back.tim iso/menu/ng.tim iso/menu/story.tim iso/menu/title.tim iso/font/bold.tim iso/font/arial.tim
iso/week1/week1.exe: Overlay.week1 iso/stage/hud0.tim iso/stage/hud1.tim iso/week1/back0.tim iso/week1/back1.tim
iso/week2/week2.exe: Overlay.week2 iso/stage/hud0.tim iso/stage/hud1.tim iso/week2/back0.tim iso/week2/back1.tim iso/week2/back2.tim
iso/week3/week3.exe: Overlay.week3 iso/stage/hud0.tim iso/stage/hud1.tim iso/week3/back0.tim iso/week3/back1.tim iso/week3/back2.tim iso/week3/back3.tim iso/week3/back4.tim iso/week3/back5.tim
