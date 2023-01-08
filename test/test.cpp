#include <stdio.h>
#include <stdlib.h>

#include <signal.h>


#include "camera.h"


void signalHandler(int sig)
{
    if (SIGINT == sig)
    {
        g_Running = false;
    }
}


bool g_Running = true;

int main(int argc, char* argv[])
{
    signal(SIGINT, signalHandler);

    Camera cam("/dev/video0");

    cam.StartCapture();

    uint8_t *buffer = new uint8_t[cam.GetFrameSize()];

    while(g_Running)
    {
        cam.ReadFrame(buffer, nullptr, nullptr);
    }

    return 0;
}