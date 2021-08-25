psxavenc: psxavenc.c mdec.c filefmt.c decoding.c cdrom.c libpsxav/adpcm.c libpsxav/cdrom.c
	$(CC) -O3 -o $@ $^ -lavcodec -lavformat -lavutil -lswresample -lswscale
all: psxavenc
