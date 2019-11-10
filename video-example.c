
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "tiger_gl.h"

#define BUF_MAX_SIZE    262144      // 256 * 1024

typedef struct _params_ {
    char ip[32];
    int port;
} Params;

bool recordVideoFlag = false;

char touchDevice[128];
TglWidget *tglImage = NULL;

bool readVideoFlag = false;
pthread_t vidThread;

void *readVideo(void *param);
void handleSignal(int sig);

void btnCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t pp);

/* This fragment of a program reads a video stream and places each frame into
 * a TglImage type.
 * The video being read is from a modified raspivid program that places a 
 * 7 byte header on each video frame.  The headers first 3 bytes are KW: and
 * the next 4 bytes is the frame length in bytes. See the restart_cam.sh script,
 * which is being run from a RPI zero W.
 * The video stream is MJPEG becaus I could not find a H.264 decoder I could use or
 * compile.
 * I have used Xuggle to decode H.264 video but it is for Java and this is a C example.
 */

int main( int argc, char *argv[]) {

	if (argc < 3) {
		printf("Usage: %s ipAddress port\n", argv[0]);
		exit(1);
	}

	if (signal(SIGINT, handleSignal) == SIG_ERR)
		printf("Can't catch SIGINT\n");
	if (signal(SIGTERM, handleSignal) == SIG_ERR)
		printf("Can't catch SIGTERM\n");

	Params *p = (Params *)calloc(1, sizeof(Params));

    strcpy(p->ip, argv[1]);
    p->port = atoi(argv[2]);

    if (pthread_create(&vidThread, NULL, readVideo, (void *)p)) {
        printf("Error: Can not create thread readVideo.\n");
        return 1;
    }

	tglInit("/dev/fb0", 1024, 768);
	tglFindTouchDevice(touchDevice);
	printf("touchDevice %s\n", touchDevice);

	if (touchDevice != NULL)
		tglTouchInit(touchDevice, 1024, 768, 800, 480, 0, 0);
	else {
		printf("No touch device found.\n");
		exit(-1);
	}

	// create main screen.
	int r = tglScreenCreate(0, 0, 1024, 768, 32);
	if (r) {
		printf("Screen create failed. (%d)\n", r);
		exit(-1);
	}

	// Create a TglImage widget.
	// Notice I am using a small video size to help with FPS.
	// The large the frame size the smaller FPS max can be.
	tglImage = tglWidgetImage(0, 0, 800, 768);

	// Create a quit button.
	TglWidget *tglQuit = tglWidgetButton("Quit", 805, 580, 210, 140);
	tglWidgetSetFont(tglQuit, "FONT_22X36");
	tglWidgetSetFgBgColor(tglQuit, TGL_COLOR_BLACK, TGL_COLOR_LIGHTGRAY);

	// Add a call back to the button.
	tglWidgetAddCallback(tglQuit, btnCallback, TOUCH_UP);

	// Register the two widgets.
	tglWidgetRegister(tglImage, tglQuit);

	// join threads so the progream does not end as long as the readvideo thread is running.
    pthread_join(vidThread, NULL);

	tglFbClose();

	return 0;
}

// Converts raw frames to a TGLBITMAP
TGLBITMAP *readJPEGMemory(unsigned char *frame, int frameLength) {

	return (TGLBITMAP *)tglImageLoadMem(frame, frameLength);
}

/* This reads the video stream.  It first reads the 7 byte header
 * this give use the frame size in bytes.  We then read the complete
 * frame.
 */
int recvFrame(int sock, unsigned char *buf, int bufSize) {

    int frameSize = 0;
    unsigned char hBuf[7];
    int r = sizeof(hBuf);

	// Read header.
    // It does not always read the number of bytes asked for.
	// So we have to loop reading bytes until we get the complete header.

    int offset = 0;
    while (r > 0) {
        int n = recv(sock, &hBuf[offset], r, 0);
        r -= n;
        offset += n;
    }

    if (r == 0) {
		// If r is zero then get frame size.
        frameSize =  (hBuf[3] << 24) & 0xff000000;
        frameSize |= (hBuf[4] << 16) & 0x00ff0000;
        frameSize |= (hBuf[5] << 8)  & 0x0000ff00;
        frameSize |= hBuf[6]         & 0x000000ff;

		// Read stream until we have complete frame.
        offset = 0;
        int len = frameSize;
        while (len > 0) {
            r = recv(sock, &buf[offset], len, 0);
            len -= r;
            offset += r;
        }
    }
	// return numebr of bytes in frame.
    return frameSize;
}

void *readVideo(void *param) {

    struct sockaddr_in servaddr;

    Params *p = (Params *)param;

	// Open a TCP socket to the RPI zero W that has a camera.
	// We use the -o tcp://192.168.2.5:43210 option to raspivid
	// See restart_cam.sh script. The script restarts my raspivid
	// program if connection is broken.
    int rdSock = socket(AF_INET, SOCK_STREAM, 0);
    if (rdSock < 0) {
        printf("Socket creation faield.\n");
        return NULL;
    }

    int iTmp = 1;
	// Make the socket reuseable.
    setsockopt(rdSock, SOL_SOCKET, SO_REUSEADDR, &iTmp, sizeof(int));

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(p->ip);
    servaddr.sin_port = htons(p->port);

	// Connect the RPI zero W over wireless
    if (connect(rdSock, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
		printf("IP: %s, Port: %d\n", p->ip, p->port);
        exit(0);
    } else {
        printf("connected to the server..\n");
	}

	// Loop reading frames from remote system.
    unsigned char vBuf[BUF_MAX_SIZE];
    while ( readVideoFlag == false) {
        int r = recvFrame(rdSock, vBuf, BUF_MAX_SIZE);

		// Put image on console.
        TGLBITMAP *img = readJPEGMemory(vBuf, r);
		if (img != NULL) {
			// Have to flip image cause FreeImage uses postion
			// 0-0 in the bottom right of the screen and the
			// monitor 0-0 position is top left.
			tglImageFlipVertical(img);

			// Draws image to TglImage widget.
			// This also automatically updates screen.
			tglDrawVideoImage(tglImage, img);

			tglImageDelete(img);
		} else {
			printf("Can not create jpeg image.\n");
		}
    }

    return NULL;
}

// p=pressure value.
void btnCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t p) {

	if (strcmp(tw->text, "Quit") == 0) {
		readVideoFlag = true;
	}
}

void handleSignal(int sig) {

    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);

    readVideoFlag = true;
}
