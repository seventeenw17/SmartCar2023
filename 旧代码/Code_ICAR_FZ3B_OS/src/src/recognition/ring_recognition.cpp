#pragma once

/**
 ********************************************************************************************************
 *                                               示例代码
 *                                             EXAMPLE  CODE
 *
 *                      (c) Copyright 2021; SaiShu.Lcc.; Leo; https://bjsstech.com
 *                                   版权所属[SASU-北京赛曙科技有限公司]
 *
 *            The code is for internal use only, not for commercial transactions(开源学习,请勿商用).
 *            The code ADAPTS the corresponding hardware circuit board(代码适配百度Edgeboard-FZ3B),
 *            The specific details consult the professional(欢迎联系我们,代码持续更正，敬请关注相关开源渠道).
 *********************************************************************************************************
 * @file ring_recognition.cpp
 * @author Leo (liaotengjun@saishukeji.com)
 * @brief 弯道识别与图像处理
 * @version 0.1
 * @date 2022-02-18
 * @copyright Copyright (c) 2022
 * @note 弯道处理步骤：
 *                      [01] 弯道类型识别：track_recognition.cpp
 *                      [02] 补线起止点搜索
 *                      [03] 边缘重计算
 *
 *
 */

#include <fstream>
#include <iostream>
#include <cmath>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "../../include/common.hpp"
#include "../../include/predictor.hpp"
#include "../recognition/track_recognition.cpp"

using namespace cv;
using namespace std;

class RingroadRecognition
{
public:
    /**
     * @brief 弯道识别与图像处理
     *
     * @param track 赛道识别结果
     * @param imagePath 输入图像
     */
    bool ringroadRecognition(TrackRecognition &track, vector<PredictResult> predict)
    {
        bool repaired = false;               //十字识别与补线结果
        crossroadType = CrossroadType::None; //十字道路类型
        pointBreakLU = POINT(0, 0);
        pointBreakLD = POINT(0, 0);
        pointBreakRU = POINT(0, 0);
        pointBreakRD = POINT(0, 0);

        uint16_t counterRec = 0;    //计数器
        uint16_t counterLinear = 0; //连续计数器
        _index = 0;

        if (track.pointsEdgeRight.size() < ROWSIMAGE / 2 || track.pointsEdgeLeft.size() < ROWSIMAGE / 2) //十字有效行限制
            return false;

void drawImage(TrackRecognition track, Mat &Image)
    {
        //绘制边缘点
        for (int i = 0; i < track.pointsEdgeLeft.size(); i++)
        {
            circle(Image, Point(track.pointsEdgeLeft[i].y, track.pointsEdgeLeft[i].x), 2,
                   Scalar(0, 255, 0), -1); //绿色点
        }
        for (int i = 0; i < track.pointsEdgeRight.size(); i++)
        {
            circle(Image, Point(track.pointsEdgeRight[i].y, track.pointsEdgeRight[i].x), 2,
                   Scalar(0, 255, 255), -1); //黄色点
        }
    }