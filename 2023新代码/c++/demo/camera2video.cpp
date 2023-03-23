#include "util/stop_watch.hpp"
#include <iostream>            // for standard I/O
#include <opencv2/core.hpp>    // Basic OpenCV structures (cv::Mat)
#include <opencv2/videoio.hpp> // Video write
#include <string>              // for strings
#include "util/mat_util.hpp"

using namespace cv;

int main(int argc, char *argv[]) {

    VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cout << "Could not open the input video: " << endl;
        return -1;
    }


    double rate = capture.get(CAP_PROP_FPS);
    double width = capture.get(CAP_PROP_FRAME_WIDTH);
    double height = capture.get(CAP_PROP_FRAME_HEIGHT);
    std::cout << "Camera Param: frame rate = " << rate << " width = " << width
              << " height = " << height << std::endl;

    const string NAME = "camera.avi";
    VideoWriter outputVideo;
    outputVideo.open(
            NAME, VideoWriter::fourcc('M', 'J', 'P', 'G'), capture.get(CAP_PROP_FPS),
            Size((int)capture.get(CAP_PROP_FRAME_WIDTH),
                 (int)capture.get(CAP_PROP_FRAME_HEIGHT)),
            false);

    if (!outputVideo.isOpened()) {
        std::cout << "Could not open the output video for write: " << endl;
        return -1;
    }

    Mat frame;
    while (1) {
        if (!capture.read(frame)) {
            std::cout << "no video frame" << std::endl;
            continue;
        }
        cv::Mat gray_frame = rbg2gray(frame);
        StopWatch file_save;
        file_save.tic();
        outputVideo << gray_frame;
        std::cout << "Camera save 2 flile time[" << file_save.toc() << "]"
                  << std::endl;
    }

    capture.release();
    return 0;
}