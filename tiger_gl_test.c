
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "tiger_gl.h"

char touchDevice[128];

bool debugFlag = false;
bool stopThread = false;

TglWidget *tglStatus = NULL;

pthread_t pauseThread;

void handleSignal(int sig);

void *pausePrg(void *param);
void btnCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t pp);
void ckbCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t pp);
void radioCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t pp);
void spinnerCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t pp);

int main(int argc, char * argv[]) {

	if (signal(SIGINT, handleSignal) == SIG_ERR)
		printf("Can't catch SIGINT\n");

	if (signal(SIGTERM, handleSignal) == SIG_ERR)
		printf("Can't catch SIGTERM\n");

	// Change width and height to match your monitor.
	tglInit("/dev/fb0", 1024, 768);
	tglFindTouchDevice(touchDevice);		// Uses the lsinput command to find touch screen.
	printf("touchDevice %s\n", touchDevice);

	// Change width and height to match your monitor and set touch width and height.
	// I set the resolution to be higher than phyical size by modifing the /boot/config.txt file.
	//   Elecrow 5 inch Capacitive Touch Screen 800x480 TFT LCD Display HDMI Interface
	if (touchDevice != NULL)
		tglTouchInit(touchDevice, 1024, 768, 800, 480, 0, 0);
	else {
		printf("No touch device found.\n");
		exit(-1);
	}

	// Create the first screen x, y, width, height and bpp size.
	// I have only run this using the 32 bit bpp.
	int r = tglScreenCreate(0, 0, 1024, 768, 32);
	if (r)
		printf("Screen create failed. (%d)\n", r);
	
	if (pthread_create(&pauseThread, NULL, pausePrg, NULL)) {
        printf("Error: Can not create thread pause.\n");
        return 1;
    }

	tglDrawFillScreen(TGL_COLOR_BLACK);

	TglWidget *tglImage = tglWidgetImage(0, 0, 800, 600);

	TGLBITMAP * img = tglImageLoad("tiger.png");

	TglWidget *tglButton = tglWidgetButton("B1", 805, 0, 210, 140);
    tglWidgetAddIcon(tglButton, "icons/cam_64x64.png");
    tglWidgetSetData(tglButton, "Hello button 1.");
    tglWidgetSetFont(tglButton, "FONT_22X36");
    tglWidgetSetFgBgColor(tglButton, TGL_COLOR_BLACK, TGL_COLOR_LIGHTGRAY);

	TglWidget *tglButton2 = tglWidgetButton("B2", 805, 145, 210, 140);
    tglWidgetAddIcon(tglButton2, "icons/icon_film_64x64.png");
    tglWidgetSetData(tglButton2, "Hello button 2.");
    tglWidgetSetFont(tglButton2, "FONT_22X36");
    tglWidgetSetFgBgColor(tglButton2, TGL_COLOR_BLACK, TGL_COLOR_LIGHTGRAY);

	TglWidget *tglQuit = tglWidgetButton("Quit", 805, 580, 210, 140);
    tglWidgetSetData(tglQuit, "Hello quit button.");
    tglWidgetSetFont(tglQuit, "FreeSansBold_52X54");
    tglWidgetSetFgBgColor(tglQuit, TGL_COLOR_BLACK, TGL_COLOR_LIGHTGRAY);

	TglWidget *tglCheckbox = tglWidgetCheckbox("Checked It?", 237, 650, 210, 45);
    tglWidgetSetData(tglCheckbox, "Hello Checkbox.");
    tglWidgetSetFont(tglCheckbox, "FONT_12x20");
    tglWidgetSetFgBgColor(tglCheckbox, TGL_COLOR_WHITE, TGL_COLOR_BLACK);

    TglWidget *tglRadio1 = tglWidgetRadio("Radio1", 451, 650, 125, 45);
    tglWidgetSetData(tglRadio1, "Hello Radio 1.");
    tglWidgetSetFont(tglRadio1, "FONT_12x20");
    tglWidgetSetRadioGroup(tglRadio1, 100);
    tglWidgetSetFgBgColor(tglRadio1, TGL_COLOR_WHITE, TGL_COLOR_BLACK);

    TglWidget *tglRadio2 = tglWidgetRadio("Radio2", 581, 650, 125, 45);
    tglWidgetSetData(tglRadio2, "Hello Radio 2.");
    tglWidgetSetFont(tglRadio2, "FONT_12x20");
    tglWidgetSetRadioGroup(tglRadio2, 100);
    tglWidgetSetFgBgColor(tglRadio2, TGL_COLOR_WHITE, TGL_COLOR_BLACK);

	TglWidget *tglPbar = tglWidgetProgressBar(805, 290, 210, 45, true, TGL_COLOR_LIGHTGREEN);
    tglWidgetSetFont(tglPbar, "FONT_12x20");
	tglWidgetSetProgressBarNum(tglPbar, 45);

	TglWidget *tglSpinner = tglWidgetSpinner(805, 340, 210, 65, "Tom T. Hall,Harry,Sue,Mary,Kelly,Richard,Keith,Sharon,Gean");
    tglWidgetSetFont(tglSpinner, "FreeSerif_33X33");
	tglWidgetSetSelection(tglSpinner, 1);

	TglWidget *tglSpinner2 = tglWidgetSpinner(2, 650, 230, 55, "Kelly,was here,and,very much gone away.");
    tglWidgetSetFont(tglSpinner2, "FreeSerif_33X33");
    tglWidgetSetSelection(tglSpinner2, 2);

	tglStatus = tglWidgetLabel("status", 2, 605, 800, 40);
    tglWidgetSetFont(tglStatus, "FONT_16x26");
    tglWidgetSetFgBgColor(tglStatus, TGL_COLOR_RED, TGL_COLOR_WHITE);

	// The last argument is whether the event should occur at touch or touch release.
	// With the argument of TOUCH UP then you can abort a touch if you slide your
	// finger outside of a widget but do not silde it to another widget with TOUCH_UP set
	// or it will activate that widget.
	tglWidgetAddCallback(tglButton, btnCallback, TOUCH_DOWN);
	tglWidgetAddCallback(tglButton2, btnCallback, TOUCH_UP);
	tglWidgetAddCallback(tglQuit, btnCallback, TOUCH_UP);
	tglWidgetAddCallback(tglCheckbox, ckbCallback, TOUCH_UP);
    tglWidgetAddCallback(tglRadio1, radioCallback, TOUCH_UP);
    tglWidgetAddCallback(tglRadio2, radioCallback, TOUCH_UP);
    tglWidgetAddCallback(tglSpinner, spinnerCallback, TOUCH_UP);
    tglWidgetAddCallback(tglSpinner2, spinnerCallback, TOUCH_UP);

	tglWidgetRegister(tglImage, tglButton, tglButton2, tglSpinner2, tglQuit, tglPbar, tglSpinner, tglCheckbox, tglRadio1, tglRadio2, tglStatus);

	tglDrawVideoImage(tglImage, img);

	pthread_join(pauseThread, NULL);

	tglFbClose();

	return 0;
}

// p=pressure value.
void spinnerCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t p) {
	printf("Selected: %s\n", tglWidgetGetSelection(tw));
}

// p=pressure value.
void radioCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t p) {
}

// p=pressure value.
void ckbCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t p) {
}

// p=pressure value.
void btnCallback(TglWidget *tw, uint16_t x, uint16_t y, uint16_t p) {
    // printf("tt.c btnCallback %s %d, %d, %d\n", tw->text, x, y, p);
    // printf("tt.c button data: '%s'\n", (char *)tglWidgetGetData(tw));

    if (strcmp(tw->text, "B1") == 0) {
		tglWidgetSetLabelText(tglStatus, "B1 was touched.");
		printf("B1 was touched with %d pressure.\n", p);
		printf("Button B1 data: '%s'\n", (char *)tglWidgetGetData(tw));
    } else if (strcmp(tw->text, "B2") == 0) {
		tglWidgetSetLabelText(tglStatus, "B2 was touched.");
		printf("B2 was touched with %d pressure.\n", p);
		printf("Button B2 data: '%s'\n", (char *)tglWidgetGetData(tw));
    } else if (strcmp(tw->text, "Quit") == 0) {
		tglWidgetSetLabelText(tglStatus, "Quiting program, please stand by.");
		printf("Quit was touched with %d pressure.\n", p);
		printf("Button Quit data: '%s'\n", (char *)tglWidgetGetData(tw));
		sleep(1);
		stopThread = true;
    }
}

void *pausePrg(void *param) {

	struct timespec t;

	t.tv_sec = 0;
	t.tv_nsec = 50000000;

	while (stopThread == false) {
		nanosleep(&t, NULL);
	}
	
	return 0;
}

void handleSignal(int sig) {

    char *signal_name = NULL;

    if (debugFlag == true) {

        printf("Entering signaling handler.\n");

        // Find out which signal we're handling
        switch (sig) {
            case SIGHUP:
                signal_name = "SIGHUP";
                break;
            case SIGTERM:
                signal_name = "SIGHUP";
                break;
            case SIGINT:
                signal_name = "SIGINT";
                break;
            default:
                fprintf(stderr, "Caught wrong signal: %d\n", sig);
                break;
        }
    }

    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);

    if (debugFlag == true)
        printf("Signal %s received.\n", signal_name);
}
