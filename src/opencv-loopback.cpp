#include "opencv-loopback.hpp"

OpenCVLoopback::OpenCVLoopback(const std::string& devicePath, int width, int height, double fps)
    : devicePath_(devicePath), width_(width), height_(height), fps_(fps), fd_(-1)
{

    fd_ = open(devicePath_.c_str(), O_RDWR);
    if (fd_ < 0)
    {
        std::cerr << "Failed to open virtual device: " << devicePath_ << " (" << strerror(errno) << ")" << std::endl;
        return;
    }

    // Set format and frame size
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = width_;
    fmt.fmt.pix.height = height_;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR32;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    V4L2_IOCTL_CHECK(fd_, VIDIOC_S_FMT, &fmt);

    // Set frame rate
    struct v4l2_streamparm stream_parm;
    memset(&stream_parm, 0, sizeof(stream_parm));
    stream_parm.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    stream_parm.parm.output.timeperframe.numerator = 1;
    stream_parm.parm.output.timeperframe.denominator = static_cast<__u32>(fps_);
    V4L2_IOCTL_CHECK(fd_, VIDIOC_S_PARM, &stream_parm);

    // Request V4L2 buffers
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    req.memory = V4L2_MEMORY_MMAP;
    V4L2_IOCTL_CHECK(fd_, VIDIOC_REQBUFS, &req);

    // mmap the buffers
    buffers_.resize(req.count);
    for (unsigned int i = 0; i < req.count; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        V4L2_IOCTL_CHECK(fd_, VIDIOC_QUERYBUF, &buf);

        buffers_[i].length = buf.length;
        buffers_[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset);
        if (buffers_[i].start == MAP_FAILED)
        {
            std::cerr << "mmap failed: " << strerror(errno) << std::endl;
            cleanup();
            return;
        }
    }

    // Queue the buffers
    for (unsigned int i = 0; i < req.count; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        V4L2_IOCTL_CHECK(fd_, VIDIOC_QBUF, &buf);
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    V4L2_IOCTL_CHECK(fd_, VIDIOC_STREAMON, &type);
}

OpenCVLoopback::~OpenCVLoopback()
{
    cleanup();
}

bool OpenCVLoopback::isReady() const
{
    return fd_ != -1;
}

void OpenCVLoopback::write(const cv::Mat& frame)
{
    if (!isReady()) return;

    cv::Mat output_bgra;
    if (frame.channels() == 4)
    {
        output_bgra = frame;
    }
    else
    {
        cv::cvtColor(frame, output_bgra, cv::COLOR_BGR2BGRA);
    }

    struct v4l2_buffer out_buf;
    memset(&out_buf, 0, sizeof(out_buf));
    out_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    out_buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd_, VIDIOC_DQBUF, &out_buf) < 0)
    {
        std::cerr << "DQBUF failed: " << strerror(errno) << std::endl;
        return;
    }

    memcpy(buffers_[out_buf.index].start, output_bgra.data, output_bgra.total() * output_bgra.elemSize());

    if (ioctl(fd_, VIDIOC_QBUF, &out_buf) < 0)
    {
        std::cerr << "QBUF failed: " << strerror(errno) << std::endl;
        return;
    }
}

void OpenCVLoopback::cleanup()
{
    if (fd_ == -1)
        return;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ioctl(fd_, VIDIOC_STREAMOFF, &type);

    for (auto& buf : buffers_)
    {
        munmap(buf.start, buf.length);
    }

    close(fd_);
    fd_ = -1;
}
