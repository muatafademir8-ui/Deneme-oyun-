#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspsdk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PSP_MODULE_INFO("Musti Racing", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define printf pspDebugScreenPrintf

static int running = 1;

typedef struct {
    int y;
    int lane;
    int active;
} TrafficCar;

static TrafficCar traffic[5];
static int playerLane = 1;
static int score = 0;
static int speed = 1;
static int frameCount = 0;
static int crashed = 0;

int exit_callback(int arg1, int arg2, void *common) {
    running = 0;
    return 0;
}

int CallbackThread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int SetupCallbacks(void) {
    int thid = sceKernelCreateThread("Callback Thread", CallbackThread,
                                     0x11, 0xFA0, 0, 0);
    if (thid >= 0) sceKernelStartThread(thid, 0, 0);
    return thid;
}

static void reset_game(void) {
    int i;
    playerLane = 1;
    score = 0;
    speed = 1;
    frameCount = 0;
    crashed = 0;
    for (i = 0; i < 5; i++) {
        traffic[i].active = 0;
        traffic[i].y = 0;
        traffic[i].lane = 1;
    }
}

static void draw_screen(void) {
    int row, lane;
    pspDebugScreenClear();
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenSetXY(13, 1);
    printf("MUSTI RACING");
    pspDebugScreenSetXY(2, 3);
    printf("PUAN: %d       HIZ: %d", score, speed);

    /* Three-lane road */
    for (row = 5; row < 29; row++) {
        pspDebugScreenSetXY(8, row);
        printf("|");
        pspDebugScreenSetXY(19, row);
        printf("|");
        pspDebugScreenSetXY(30, row);
        printf("|");
        pspDebugScreenSetXY(41, row);
        printf("|");
        if ((row + frameCount / 3) % 4 == 0) {
            pspDebugScreenSetXY(14, row); printf(".");
            pspDebugScreenSetXY(25, row); printf(".");
            pspDebugScreenSetXY(36, row); printf(".");
        }
    }

    for (lane = 0; lane < 5; lane++) {
        if (traffic[lane].active && traffic[lane].y >= 5 && traffic[lane].y < 29) {
            int x = 10 + traffic[lane].lane * 11;
            pspDebugScreenSetXY(x, traffic[lane].y);
            pspDebugScreenSetTextColor(0xFF00FFFF);
            printf("[T]");
            pspDebugScreenSetTextColor(0xFFFFFFFF);
        }
    }

    pspDebugScreenSetXY(10 + playerLane * 11, 25);
    pspDebugScreenSetTextColor(0xFF00FF00);
    printf("[A]");
    pspDebugScreenSetTextColor(0xFFFFFFFF);

    pspDebugScreenSetXY(2, 31);
    printf("D-PAD: Direksiyon   X: Gaz   O: Fren");
    pspDebugScreenSetXY(2, 32);
    printf("START: Yeniden baslat   HOME: Cikis");

    if (crashed) {
        pspDebugScreenSetTextColor(0xFF0000FF);
        pspDebugScreenSetXY(15, 15);
        printf("KAZA YAPTIN!");
        pspDebugScreenSetXY(9, 17);
        printf("START ile yeniden basla");
        pspDebugScreenSetTextColor(0xFFFFFFFF);
    }
}

int main(void) {
    SceCtrlData pad;
    int i;

    SetupCallbacks();
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    pspDebugScreenInit();
    reset_game();

    while (running) {
        sceCtrlReadBufferPositive(&pad, 1);

        if (crashed) {
            if (pad.Buttons & PSP_CTRL_START) reset_game();
            draw_screen();
            sceDisplayWaitVblankStart();
            sceKernelDelayThread(50000);
            continue;
        }

        if ((pad.Buttons & PSP_CTRL_LEFT) && playerLane > 0) playerLane--;
        if ((pad.Buttons & PSP_CTRL_RIGHT) && playerLane < 2) playerLane++;

        if (pad.Buttons & PSP_CTRL_CROSS) {
            if (speed < 5) speed++;
        } else if (pad.Buttons & PSP_CTRL_CIRCLE) {
            if (speed > 1) speed--;
        } else if (speed > 1) {
            speed--;
        }

        frameCount++;

        /* Spawn traffic periodically */
        if (frameCount % (12 - speed) == 0) {
            for (i = 0; i < 5; i++) {
                if (!traffic[i].active) {
                    traffic[i].active = 1;
                    traffic[i].y = 5;
                    traffic[i].lane = rand() % 3;
                    break;
                }
            }
        }

        for (i = 0; i < 5; i++) {
            if (traffic[i].active) {
                if (frameCount % (6 - (speed > 4 ? 3 : speed)) == 0)
                    traffic[i].y++;
                if (traffic[i].y >= 25 && traffic[i].y <= 26 &&
                    traffic[i].lane == playerLane) {
                    crashed = 1;
                }
                if (traffic[i].y > 28) {
                    traffic[i].active = 0;
                    score += 10;
                }
            }
        }

        draw_screen();
        sceDisplayWaitVblankStart();
        sceKernelDelayThread(50000);
    }

    sceKernelExitGame();
    return 0;
}
