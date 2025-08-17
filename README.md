# OpenCV-V4L2-Loopback
opencv-v4l2-loopback is a C++ project that aims to provides a low-latency video streaming bridge between OpenCV and a Linux V4L2 virtual camera device. It's designed to provide an easy to use drop in API for OpenCV to allow you to stream the output of your video processing pipelines directly to applications like Google Meet or Zoom.

The core functionality is encapsulated in the class, OpenCVLoopback, which handles all the low-level V4L2 API calls and buffer management. This approach bypasses the cv::VideoWriter backend for greater reliability and performance.

This can also be used with OpenCVs libcamera backend which can be found [here](https://github.com/advait-0/opencv), with usage instructions [here](https://gist.github.com/advait-0/0d514a9c4328a28a29b52b297d555c43).

## Features

- Zero-Copy Streaming: Uses mmap for efficient, zero-copy data transfer from your application to the kernel buffer.

- Low Latency: Optimized to minimize buffer queues and reduce pipeline latency.

- Reusable Class: The OpenCVLoopback class is a simple, modern C++ wrapper that can be easily integrated into any OpenCV project.

## Installation
Installation
Prerequisites

1. OpenCV: You need a working installation of OpenCV (version 4.x or later) with a supported compiler. On Ubuntu, you can install it via:
```bash
    sudo apt-get update
    sudo apt-get install libopencv-dev
```

2. v4l2loopback Kernel Module: This module is needed for creating the virtual video device.
```bash
    sudo apt-get install v4l2loopback-dkms v4l-utils
```

Loading the Virtual Device

Before running the application, you must load the v4l2loopback kernel module with the correct parameters. This configures the virtual device to accept the BGRA pixel format used by the application.

```bash
sudo modprobe v4l2loopback devices=1 video_nr=10 exclusive_caps=1 card_label="OpenCV-Loopback-Cam" pixel_format=BGR4
```

devices=1: Creates a single virtual device.

video_nr=10: Creates the device at /dev/video10.

exclusive_caps=1: Sets the device to be an exclusive output source.

"pixel_format=BGR4": The critical parameter that enables the BGRA format for the virtual device.

You can verify the device is correctly loaded with:
```bash
v4l2-ctl --list-devices
```

## Usage
### Building the Project

The project consists of a reusable class in `src/opencv_loopback`.cpp and a demonstration in `examples/demo.cpp`.

You can compile the example with g++ using pkg-config to automatically find OpenCV's headers and libraries.
Similarly replace the example path with your own code's path to run the loopback with your own logic.

```bash
g++ -o demo examples/demo.cpp src/opencv_loopback.cpp `pkg-config --cflags --libs opencv4`
```

(Note: If your system uses a different package name for OpenCV, you may need to use opencv or opencv5 instead of opencv4.)

While the program is running, you can open another application (e.g., a web browser for Google Meet, or Zoom) and select "OpenCV-Loopback-Cam" from the list of video devices. You should see the inverted video feed. The demo application's imshow window also provides a local preview.