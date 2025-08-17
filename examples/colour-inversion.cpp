#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include "../src/opencv-loopback.hpp"

int main()
{
    cv::VideoCapture cap;
    cap.open(0);

    // Get the actual properties from the camera
    int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps == 0)
    {
        fps = 30;
    }

    std::cout << "Input camera properties: " << width << "x" << height << " at " << fps << " FPS." << std::endl;

    // Create an instance of our OpenCVLoopback class
    OpenCVLoopback vcam("/dev/video10", width, height, fps);
    if (!vcam.isReady())
    {
        std::cerr << "Virtual camera is not ready. Exiting." << std::endl;
        return -1;
    }

    cv::Mat frame;
    cv::Mat processed_frame;

    while (true)
    {
        // Read from the input camera
        cap.read(frame);
        if (frame.empty())
        {
            std::cerr << "Failed to read frame from camera. Exiting." << std::endl;
            break;
        }

        // Simple processing pipeline
        cv::bitwise_not(frame, processed_frame);

        // ROUTE TO GMEET/ZOOM
        vcam.write(processed_frame);

        // OPTIONAL: Preview on screen
        cv::imshow("Processed Preview", processed_frame);
        if (cv::waitKey(1) == 'q')
        {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
