#ifndef _HW_CAMERA_H_
#define _HW_CAMERA_H_


#include <stdint.h>
#include <sys/time.h>

#include "camera_def.h"

#define _USE_MODE_MMAP


struct buffer {
        void   *start;
        uint32_t  length;
};

class Camera
{
private:
    int mFD;
    int mIdx;
    char mDevName[40];


    uint32_t mPixelFormat;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mFrameSize;
    buffer *mBuffers;

    void open();
    void close();

#ifdef _USE_MODE_MMAP
    void initMmap();
#else
    void initRead();
#endif
public:
    Camera(const char* devName);
    ~Camera();


    void StartCapture();
    void StopCapture();

    bool ReadFrame(void* buffer, uint32_t* sequence, struct timeval* timeStamp);
    uint32_t GetFrameSize(void);
};


#endif