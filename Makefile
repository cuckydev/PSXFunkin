TARGET = funkin
TYPE = ps-exe

SRCS = src/main.c \
       src/mem.c \
       src/mutil.c \
	   src/timer.c \
       src/io.c \
       src/gfx.c \
       src/audio.c \
	   src/movie.c \
       src/pad.c \
       src/random.c \
       src/archive.c \
       src/font.c \
       src/trans.c \
       src/loadscr.c \
       src/menu.c \
       src/stage.c \
       src/stage/dummy.c \
       src/stage/week1.c \
       src/stage/week4.c \
       src/stage/week7.c \
       src/animation.c \
       src/character.c \
       src/character/bf.c \
       src/character/bfweeb.c \
       src/character/gf.c \
       src/character/speaker.c \
       src/character/dad.c \
       src/character/mom.c \
       src/character/senpai.c \
       src/character/tank.c \
       src/object.c \
       src/object/combo.c \
       mips/common/crt0/crt0.s

CPPFLAGS += -Wall -mno-check-zero-division
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
