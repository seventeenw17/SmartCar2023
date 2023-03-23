#pragma once
#include <opencv2/core.hpp> // Basic OpenCV structures (cv::Mat)
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include "../common/model_config.hpp"
#include <paddle_api.h>
#include <paddle_image_preprocess.h>
#include "core/capture.hpp"
using namespace std;
using namespace cv;
using namespace paddle::lite_api;
typedef paddle::lite_api::Tensor Tensor;
typedef paddle::lite_api::DataLayoutType LayoutType;
typedef paddle::lite::utils::cv::FlipParam FlipParam;
typedef paddle::lite::utils::cv::TransParam TransParam;
typedef paddle::lite::utils::cv::ImageFormat ImageFormat;
typedef paddle::lite::utils::cv::ImagePreprocess ImagePreprocess;
// AI 模型图像预处理函数，这个函数有predictor类接口进行调用
// 用户不必直接使用它
inline void fpga_preprocess(cv::Mat img, std::shared_ptr<ModelConfig> &config,
                            std::unique_ptr<Tensor> &tensor) {

    int width = img.cols;
    int height = img.rows;

    if (height > 1080) {
        float fx = img.cols / config->input_width;
        float fy = img.rows / config->input_height;
        Mat resize_mat;
        resize(img, resize_mat, Size(config->input_width, config->input_height), fx,
               fy);
        img = resize_mat;
        height = img.rows;
        width = img.cols;
    }

    uint8_t *src = (uint8_t *)malloc(3 * width * height);
    if (img.isContinuous()) {
        memcpy(src, img.data, 3 * width * height * sizeof(uint8_t));
    } else {
        uint8_t *img_data = img.data;
        for (int i = 0; i < img.rows; ++i) {
            src = src + i * (width * 3);
            img_data = img_data + i * (width * 3);
            memcpy(src, img_data, width * 3 * sizeof(uint8_t));
        }
    }
    TransParam tparam;
    tparam.ih = img.rows;
    tparam.iw = img.cols;
    tparam.oh = config->input_height;
    tparam.ow = config->input_width;

    ImagePreprocess image_preprocess(
            ImageFormat::BGR,
            config->format == "RGB" ? ImageFormat::RGB : ImageFormat::BGR, tparam);
    image_preprocess.image_to_tensor(src, tensor.get(), LayoutType::kNHWC,
                                     config->means, config->scales);
    free(src);
}
// 传统图像预处理的模块
class TraditionalPreprocessor {
public:
    TraditionalPreprocessor() {}
    Mat ImagePreprocess(Mat &frame) {
        Mat imageGray,imageBinary;
        //GaussianBlur(frame, output, cv::Size(3, 3), 0, 0);		//高斯降噪

        cvtColor(frame, imageGray, COLOR_BGR2GRAY);  				//RGB转灰度图
        // imshow("imageGray", imageGray);

        blur(imageGray, imageGray, Size(3, 3));						//平滑图像

        //int th = GetOSTUThreshold(gray);
        threshold(imageGray, imageBinary, 0, 255, THRESH_OTSU);		//OTSU二值化方法
        // imshow("imageBinary", imageBinary);

        return imageBinary;
    }

};

