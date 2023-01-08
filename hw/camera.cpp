#include "camera.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <ctype.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include <linux/videodev2.h>


#define BUFFER_MAX  6


static void errnoExit(const char* msg, const char* devName)
{
    fprintf(stderr, "%s: %s error %d, %s\\n", devName, msg, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

static int xioctl(int fd, int request, void *arg)
{
    int r;

    do 
    {
        r = ioctl(fd, request, arg);
    }
    while (-1 == r && EINTR == errno);

    return r;
}

#ifdef _USE_MODE_MMAP
void Camera::initMmap()
{
    struct v4l2_requestbuffers req;

    memset(&req, 0x00, sizeof(req));

    req.count = BUFFER_MAX;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(mFD, VIDIOC_REQBUFS, &req))
    {
        errnoExit("VIDIOC_REQBUFS", mDevName);
    }

    if (req.count < 2)
    {
        errnoExit("Insufficient buffer size", mDevName);
    }

    mBuffers = new buffer[req.count];

    if (mBuffers == nullptr)
    {
        errnoExit("Out of memory", mDevName);
    }

    for (int i=0; i<req.count; ++i)
    {
        struct v4l2_buffer buf;

        memset(&buf, 0x00, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(mFD, VIDIOC_QUERYBUF, &buf))
        {
            errnoExit("VIDIOC_QUERYBUF", mDevName);
        }

        mBuffers[i].length = buf.length;
        mBuffers[i].start = mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              mFD, buf.m.offset);
        
        if (MAP_FAILED == mBuffers[i].start)
        {
            errnoExit("MMAP", mDevName);
        }
    }
}


#else
void Camera::initRead()
{

}

#endif



Camera::Camera(const char* devName)
    :
    mFD(-1),
    mIdx(-1)
{
    strcpy(mDevName, devName);

    while(*devName++)
    {
        if (isdigit(*devName) > 0) //may won't work more than 2 digits
        {
            mIdx = atoi(devName);
        }
    }
    open();
}

Camera::~Camera()
{
    close();
}

void Camera::open()
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;    


    mFD = ::open(mDevName, O_RDWR /* required */ | O_NONBLOCK, 0);
    
    if (mFD < 0)
    {
        errnoExit("Cannot open camera", mDevName);
    }


    if (-1 == xioctl(mFD, VIDIOC_QUERYCAP, &cap))
    {
        errnoExit("VIDIOC_QUERYCAP", mDevName);
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        errnoExit("No video capture device", mDevName);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        errnoExit("Does not support streaming", mDevName);
    }


    memset(&fmt, 0x00, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    xioctl(mFD, VIDIOC_G_FMT, &fmt);

    if (fmt.type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
        errnoExit("Unsupport type", mDevName);
    }

    if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
    {
        errnoExit("Unsupport pixel format", mDevName);
    }

    mWidth = fmt.fmt.pix.width;
    mHeight = fmt.fmt.pix.height;
    mPixelFormat = V4L2_PIX_FMT_YUYV;

    mFrameSize = mWidth*mHeight*2;

    initMmap();
}

void Camera::close()
{
    if (mBuffers != nullptr)
    {
        for (int i=0; i<BUFFER_MAX; ++i)
        {
            if (-1 == munmap(mBuffers[i].start, mBuffers[i].length))
            {
                errnoExit("munmap", mDevName);
            }
        }
        
        
        delete[] mBuffers;
    }


    if (-1 == ::close(mFD))
    {
        errnoExit("Cannot close camera", mDevName);
    }

    mFD = -1;
}

void Camera::StartCapture()
{
    enum v4l2_buf_type type;

    for (int i = 0; i < BUFFER_MAX; ++i) {
            struct v4l2_buffer buf;

            memset(&buf, 0x00, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (-1 == xioctl(mFD, VIDIOC_QBUF, &buf))
                    errnoExit("VIDIOC_QBUF", mDevName);
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(mFD, VIDIOC_STREAMON, &type))
            errnoExit("VIDIOC_STREAMON", mDevName);
}

void Camera::StopCapture()
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(mFD, VIDIOC_STREAMOFF, &type))
    {
        errnoExit("VIDIOC_STREAMOFF", mDevName);
    }
}

bool Camera::ReadFrame(void* buffer, uint32_t* sequence, struct timeval* timeStamp)
{
    bool ret;
    int r;
    fd_set fds;
    struct v4l2_buffer buf;

    memset(&buf, 0x00, sizeof(buf));

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    FD_ZERO(&fds);

    do
    {
        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        
        r = select(mFD+1, &fds, NULL, NULL, &timeout);
    } while(-1 == r && EINVAL == errno);

    if (-1 == xioctl(mFD, VIDIOC_DQBUF, &buf))
    {
        switch (errno)
        {
        case EAGAIN:
            ret = false;
            break;        
        default:
            errnoExit("VIDIOC_DQBUF", mDevName);
            break;
        }
    }

    memcpy(buffer, mBuffers[buf.index].start, buf.bytesused);

    if (sequence != nullptr)
    {
        *sequence = buf.sequence;
    }

    if (timeStamp != nullptr)
    {
        *timeStamp = buf.timestamp;
    }

    if (-1 == xioctl(mFD, VIDIOC_QBUF, &buf))
    {
        errnoExit("VIDIOC_QBUF", mDevName);
    }

    ret = true;

    return ret;
}

uint32_t Camera::GetFrameSize(void)
{
    return mFrameSize;
}