/*
	
	Simple STR Player Library by Lameguy64 
	(?) 2014 Meido-Tek Productions/Lame Studios
	
	Original PsyQ sample programmed by:
		Yutaka
		Suzu
		Masa
		Ume
	
	Code heavily refined by:
		Lameguy64
	
	What Lameguy did to the original code:
		- Removed all of the icky yucky UTF-16 junk
		- Fixed all crap-English comments
		- Greatly improved code formatting
		- Renamed variables with better names
		- Buffer arrays are now initialized only when the playback routine is called
	
	Libraries Required:
		libetc
		libgte
		libgpu
		libcd
	
	Function list:
	
		int PlayStr(int xres, int yres, int xpos, int ypos, STRFILE *str)
			
			Parameters:
				xres, yres		- Video resolution.
				xpos, ypos		- Framebuffer offset on where to draw the video.
				STRFILE *str	- STRFILE entry to play.
			
			Notes:
				Just make sure that you have at least 192KB of free memory before calling
				the	PlayStr function otherwise, the console will crash. As for the video
				resolution, it must be equal or less than 256 as the second buffer
				is located directly below the first buffer.
		
	Note:
		
		If compiling the sample fails, open your psyq.ini file located in \psyq\bin and
		append the following into the stdlib line:
		
		libds.lib libpress.lib
	
*/

#include "psx.h"
#include <libpress.h>

#define IS_RGB24	0	// 0:16-bit playback, 1:24-bit playback (recommended for quality)
#define RING_SIZE	32	// Ring Buffer size (32 sectors seems good enough)

#if IS_RGB24==1
	#define PPW			3/2	// pixels per short word
	#define DCT_MODE	3	// Decode mode for DecDCTin routine
#else
	#define PPW			1
	#define DCT_MODE    2
#endif


// A simple struct to make STR handling a bit easier
typedef struct {
	char	FileName[32];
	int		Xres;
	int		Yres;
	int		NumFrames;
} STRFILE;

// Decode environment
typedef struct {
	u_long	*VlcBuff_ptr[2];	// Pointers to the VLC buffers
	u_short	*ImgBuff_ptr[2];	// Pointers to the frame slice buffers
	RECT	rect[2];			// VRAM parameters on where to draw the frame data to
	RECT	slice;				// Frame slice parameters for loading into VRAM
	int		VlcID;				// Current VLC buffer ID
	int		ImgID;				// Current slice buffer ID
	int 	RectID;				// Current video buffer ID
	int		FrameDone;			// Frame decode completion flag
} STRENV;

// A bunch of internal variables
static STRENV strEnv;

static int	strScreenWidth=0,strScreenHeight=0;
static int	strFrameX=0,strFrameY=0;
static int	strNumFrames=0;

static int strFrameWidth=0,strFrameHeight=0;	// Frame size of STR file
static int strPlayDone=0;						// Playback completion flag

// Main function prototypes
int PlayStr(int xres, int yres, int xpos, int ypos, STRFILE *str);

// Internal function prototypes
static void strDoPlayback(STRFILE *str);
static void strCallback();
static void strNextVlc(STRENV *strEnv);
static void strSync(STRENV *strEnv, int mode);
static u_long *strNext(STRENV *strEnv);
static void strKickCD(CdlLOC *loc);


int PlayStr(int xres, int yres, int xpos, int ypos, STRFILE *str) {
	
	/*
		Main STR playback routine.
		
		Returns:
			0 - Playback failed or was skipped.
			1 - Playback was finished.
	*/
	
	strNumFrames=str->NumFrames;
	strScreenWidth=xres;
	strScreenHeight=yres;
	strFrameX=xpos;
	strFrameY=ypos;
	
	strPlayDone=0;
	strDoPlayback(str);
	
	if (strPlayDone == 0)
		return(0);
	else
		return(1);
	
}

static void strDoPlayback(STRFILE *str) {
	
	/*
		Does the actual STR playback.
	*/
	
	int id;			// Display buffer ID
	DISPENV disp;	// Display environment
	CdlFILE file;	// File info of video file
	
	// Buffers initialized here so we won't waste too much memory for playing FMVs
	// (just make sure you have at least 192KB of free memory before calling this routine)
	u_long	RingBuff[RING_SIZE*SECTOR_SIZE];	// Ring buffer
	u_long	VlcBuff[2][str->Xres/2*str->Yres];	// VLC buffers
	u_short	ImgBuff[2][16*PPW*str->Yres];		// Frame 'slice' buffers
	
	// Set display mask so we won't see garbage while the stream is being prepared
	SetDispMask(0);
	
	// Get the CD location of the STR file to play
	if (CdSearchFile(&file, str->FileName) == 0) {
		#ifdef DEBUG
		printf("ERROR: I cannot find video file %s\n", str->FileName);
		#endif
		SetDispMask(1);
		return;
	}
	
	// Setup the buffer pointers
	strEnv.VlcBuff_ptr[0] = &VlcBuff[0][0];
	strEnv.VlcBuff_ptr[1] = &VlcBuff[1][0];
	strEnv.VlcID     = 0;
	strEnv.ImgBuff_ptr[0] = &ImgBuff[0][0];
	strEnv.ImgBuff_ptr[1] = &ImgBuff[1][0];
	strEnv.ImgID     = 0;
	
	// Setup the display buffers on VRAM
	strEnv.rect[0].x = strFrameX;	// First page
	strEnv.rect[0].y = strFrameY;
	strEnv.rect[1].x = strFrameX;	// Second page
	strEnv.rect[1].y = strFrameY+strScreenHeight;
	strEnv.RectID    = 0;
	
	// Set the parameters for uploading frame slices
	strEnv.slice.x = strFrameX;
	strEnv.slice.y = strFrameY;
	strEnv.slice.w	= 16*PPW;
	strEnv.FrameDone	= 0;
	
	// Reset the MDEC
	DecDCTReset(0);
	// Set callback routine
	DecDCToutCallback(strCallback);
	// Set ring buffer
	StSetRing(RingBuff, RING_SIZE);
	// Set streaming parameters
	StSetStream(IS_RGB24, 1, 0xffffffff, 0, 0);
	// Begin streaming!
	strKickCD(&file.pos);
	
	// Load the first frame of video before entering main loop
	strNextVlc(&strEnv);
	
	while (1) {
		
		// Decode the compressed frame data
		DecDCTin(strEnv.VlcBuff_ptr[strEnv.VlcID], DCT_MODE);
		
		// Prepare to receive the decoded image data from the MDEC
		DecDCTout((u_long*)strEnv.ImgBuff_ptr[strEnv.ImgID], strEnv.slice.w*strEnv.slice.h/2);
		
		// Get the next frame
		strNextVlc(&strEnv);
		
		// Wait for the frame to finish decoding
		strSync(&strEnv, 0);
				
		// Switch between the display buffers per frame
		id = strEnv.RectID? 0: 1;
		SetDefDispEnv(&disp, 0, strScreenHeight*id, strScreenWidth*PPW, strScreenHeight);
		
		// Set parameters for 24-bit color mode
		#if IS_RGB24 == 1
		disp.isrgb24 = IS_RGB24;
		disp.disp.w = disp.disp.w*2/3;
		#endif

		VSync(0);			// VSync to avoid screen tearing
		PutDispEnv(&disp);	// Apply the video parameters
		SetDispMask(1);		// Remove the display mask
		
		if(strPlayDone == 1) {
			break;
		}
		
		if(PadRead(1) & PADstart) {  // stop button pressed exit animation routine
			break;
		}
		
	}
	
	// Shutdown streaming
	DecDCToutCallback(0);
	StUnSetRing();
	CdControlB(CdlPause, 0, 0);
	
}
static void strCallback() {

	/*
		Callback routine which is called whenever a slice has finished decoding.
		All it does is transfer the decoded slice into VRAM.
	*/

	RECT TransferRect;
	int  id;
	
	// In 24-bit color, StCdInterrupt must be called in every callback
	#if IS_RGB24==1
	extern int StCdIntrFlag;
	if (StCdIntrFlag) {
		StCdInterrupt();
		StCdIntrFlag = 0;
	}
	#endif
	
	id = strEnv.ImgID;
	TransferRect = strEnv.slice;
	
	// Switch slice buffers
	strEnv.ImgID = strEnv.ImgID? 0:1;
	
	// Step to next slice
	strEnv.slice.x += strEnv.slice.w;
	
	// Frame not yet decoded completely?
	if (strEnv.slice.x < strEnv.rect[strEnv.RectID].x + strEnv.rect[strEnv.RectID].w) {
	
		// Prepare for next slice
		DecDCTout((u_long*)strEnv.ImgBuff_ptr[strEnv.ImgID], strEnv.slice.w*strEnv.slice.h/2);
	
	} else { // Frame has been decoded completely
	
		// Set the FrameDone flag
		strEnv.FrameDone = 1;

		// Switch display buffers
		strEnv.RectID = strEnv.RectID? 0: 1;
		strEnv.slice.x = strEnv.rect[strEnv.RectID].x;
		strEnv.slice.y = strEnv.rect[strEnv.RectID].y;
		
	}

	// Transfer the slice into VRAM
	LoadImage(&TransferRect, (u_long *)strEnv.ImgBuff_ptr[id]);

}
static void strNextVlc(STRENV *strEnv) {
	
	/*
		Performs VLC decoding and grabs a frame from the stream.
	*/

	int		cnt=WAIT_TIME;
	u_long	*next;
	u_long	*strNext();

	// Grab a frame from the stream
	while ((next = strNext(strEnv)) == 0) {
	
		if (--cnt == 0)	// Timeout handler
			return;
			
	}

	// Switch VLC buffers
	strEnv->VlcID = strEnv->VlcID? 0: 1;

	// Decode the VLC
	DecDCTvlc(next, strEnv->VlcBuff_ptr[strEnv->VlcID]);

	// Free the ring buffer
	StFreeRing(next);

}
static u_long *strNext(STRENV *strEnv) {
	
	/*
		Grabs a frame of video from the stream.
	*/
	
	u_long		*addr;
	StHEADER	*sector;
	int			cnt = WAIT_TIME;

	// Grab a frame
	while (StGetNext((u_long **)&addr,(u_long **)&sector)) {
	
		if (--cnt == 0)	// Timeout handler
			return(0);
			
	}

	// If the frame's number has reached number of frames the video has,
	// set the strPlayDone flag.
	if (sector->frameCount >= strNumFrames)
		strPlayDone = 1;
	
	
	// if the resolution is differ to previous frame, clear frame buffer
	if (strFrameWidth != sector->width || strFrameHeight != sector->height) {
		
		RECT    rect;
		setRECT(&rect, 0, 0, strScreenWidth * PPW, strScreenHeight*2);
		ClearImage(&rect, 0, 0, 0);

		strFrameWidth  = sector->width;
		strFrameHeight = sector->height;
		
	}
	

	// set STRENV according to the data on the STR format
	strEnv->rect[0].w = strEnv->rect[1].w = strFrameWidth*PPW;
	strEnv->rect[0].h = strEnv->rect[1].h = strFrameHeight;
	strEnv->slice.h   = strFrameHeight;

	return(addr);
	
}
static void strSync(STRENV *strEnv, int mode) {
	
	/*
		Waits for the frame to finish decoding.
	*/
	
    u_long cnt = WAIT_TIME;

    // Wait for the frame to finish decoding
    while (strEnv->FrameDone == 0) {
        if (--cnt == 0) { // Timeout handler
            // If a timeout occurs, force switching buffers
			#ifdef DEBUG
			printf("ERROR: A frame cannot be played!\n");
			#endif
            strEnv->FrameDone = 1;
            strEnv->RectID = strEnv->RectID? 0: 1;
            strEnv->slice.x = strEnv->rect[strEnv->RectID].x;
            strEnv->slice.y = strEnv->rect[strEnv->RectID].y;
        }
    }
	
    strEnv->FrameDone = 0;
	
}
static void strKickCD(CdlLOC *loc) {
	
	/*
		Begins CD streaming.
	*/
	
	u_char param=CdlModeSpeed;
	
	loop:
	
	// Seek to the STR file to play
	while (CdControl(CdlSetloc, (u_char *)loc, 0) == 0);
	while (CdControl(CdlSetmode, &param, 0) == 0);
	
	VSync(3);  // Wait for 3 screen cycles before changing drive speed
	
	// Start streaming
	if(CdRead2(CdlModeStream|CdlModeSpeed|CdlModeRT) == 0)
		goto loop;	// If it fails, try again
	
}