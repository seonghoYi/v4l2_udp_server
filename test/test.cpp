#include <stdio.h>
#include <stdlib.h>

#include <signal.h>


#include "camera.h"

#include "StaticQueue.h"


bool g_Running = true;

void signalHandler(int sig)
{
    if (SIGINT == sig)
    {
        g_Running = false;
    }
}

int main(int argc, char* argv[])
{
    signal(SIGINT, signalHandler);

    StaticQueue<int, 100> queue;
    Camera cam("/dev/video0");

    cam.StartCapture();

    uint8_t *buffer = new uint8_t[cam.GetFrameSize()];
/*
    while(g_Running)
    {
        cam.ReadFrame(buffer, nullptr, nullptr);
    }
*/
    for (int i=0; i<200; i++)
    {
        queue.push(std::move(i));
    }

    for (int i=0; i<200; i++)
    {
        int item;
        queue.pop(item);
        printf("%d\n", item);
    }
    return 0;
}