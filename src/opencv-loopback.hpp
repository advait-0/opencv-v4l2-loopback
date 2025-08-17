#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct V4L2Buffer
{
    void *start;
    size_t length;
};

// Error checking macro
#define V4L2_IOCTL_CHECK(fd, req, arg) \
    if (ioctl(fd, req, arg) < 0)
    { \
        std::cerr << "ioctl failed (" << __FILE__ << ":" << __LINE__ << "): " << strerror(errno) << std::endl; \
        cleanup(); \
        return; \
    }

class OpenCVLoopback
{
public:
    OpenCVLoopback(const std::string& devicePath, int width, int height, double fps);
    ~OpenCVLoopback();

    bool isReady() const;
    void write(const cv::Mat& frame);

private:
    std::string devicePath_;
    int width_, height_;
    double fps_;
    int fd_;
    std::vector<V4L2Buffer> buffers_;

    void cleanup();
};
