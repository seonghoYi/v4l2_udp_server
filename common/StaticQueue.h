#ifndef _COMMON_STATICQUEUE_H_
#define _COMMON_STATICQUEUE_H_


#include <memory>

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>


template <typename T, size_t N = 1>
class StaticQueue
{
private:
    T* mBuffer;

    uint32_t mIn;
    uint32_t mOut;
    uint32_t mItemSize;
    uint32_t mLength;

public:
    StaticQueue();
    virtual ~StaticQueue();

    bool push(const T& item);
    bool push(T&& item);

    bool pop(T& item);

    bool empty(void);
    void flush(void);
    uint32_t available(void);
};

template <typename T, size_t N>
StaticQueue<T, N>::StaticQueue()
    :
    mIn(0),
    mOut(0),
    mLength(N),
    mItemSize(sizeof(T))
{
    mBuffer = new T[mLength];
    if (mBuffer == nullptr)
    {
        printf("StaticQueue memory allocation fail\n");
    }
}

template <typename T, size_t N>
StaticQueue<T, N>::~StaticQueue()
{
    delete[] mBuffer;
    mBuffer = nullptr;
}

template <typename T, size_t N>
bool StaticQueue<T, N>::push(const T& item)
{
    bool ret = true;

    mBuffer[mIn] = item;
    mIn = (mIn + 1) % mLength;

    if (mIn == mOut)
    {
        ret = false;
        return ret;
    }
    
    return ret;
}

template <typename T, size_t N>
bool StaticQueue<T, N>::push(T&& item)
{
    bool ret = true;

    mBuffer[mIn] = std::move(item);
    mIn = (mIn + 1) % mLength;

    if (mIn == mOut)
    {
        ret = false;
        return ret;
    }

    return ret;
}

template <typename T, size_t N>
bool StaticQueue<T, N>::pop(T& item)
{
    bool ret = true;

    item = mBuffer[mOut];
    mOut = (mOut + 1) % mLength;

    if (mIn == mOut)
    {
        ret = false;
        return ret;
    }

    return ret;
}

template <typename T, size_t N>
bool StaticQueue<T, N>::empty(void)
{
    return mIn == mOut;
}

template <typename T, size_t N>
uint32_t StaticQueue<T, N>::available(void)
{
    return (mIn - mOut) % mLength;
}

template <typename T, size_t N>
void StaticQueue<T, N>::flush(void)
{
    mIn = 0;
    mOut = 0;
}

#endif