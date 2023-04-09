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
 * @file freezone_detection.cpp
 * @author Leo ()
 * @brief 泛行区检测与路径规划
 * @version 0.1
 * @date 2022-07-04
 *
 * @copyright Copyright (c) 2022
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

class FreezoneDetection
{
public:
    float rateTurnFreezone = 0.5; //泛行区躲避禁行标志的转弯曲率
    bool slowDown = false;        //减速使能

    /**
     * @brief 泛行区检测初始化
     *
     */
    void reset(void)
    {
        freezoneStep = FreezoneStep::FreezoneNone; //泛行区行使状态
        counterSession = 0;                        //图像场次计数器
        counterRec = 0;                            //标志检测计数器
        pointsStopRepair.clear();                  //泛行区禁行标志补线点集
        leftChanel = false;                        //禁行标志左右通道补线标志
    }

    /**
     * @brief 泛行区AI检测与路径规划
     *
     * @param track
     * @param predict
     * @return true
     * @return false
     */
    bool freezoneDetection(TrackRecognition &track, vector<PredictResult> predict)
    {
        _pointStop = POINT(0, 0);
        slowDown = false;

        switch (freezoneStep)
        {
        case FreezoneStep::FreezoneNone: //[01] 检测泛行区标志
        {
            POINT freezone = searchFreezoneSign(predict);
            if (freezone.x > 0)
                counterRec++;

            if (counterRec)
            {
                counterSession++;
                if (counterRec >= 3 && counterSession < 8)
                {
                    freezoneStep = FreezoneStep::FreezoneEnable; //使能
                    counterRes = 0;
                    counterRec = 0;
                    counterSession = 0;
                }
                else if (counterSession >= 8)
                {
                    counterRec = 0;
                    counterSession = 0;
                }
            }
            break;
        }
        case FreezoneStep::FreezoneEnable: //[02] 泛行区使能
        {
            if (track.pointsEdgeLeft.size() > ROWSIMAGE / 2 && track.pointsEdgeRight.size() > ROWSIMAGE / 2) //如果没进泛行区，提前退出
            {
                counterSession++;
                if (counterSession > 50) //推出泛行区状态
                {
                    freezoneStep = FreezoneStep::FreezoneNone;
                    counterSession = 0;
                    counterRec = 0;
                }
            }

            if (track.pointsEdgeLeft.size() > 5 && track.pointsEdgeRight.size() > 5) //入泛行区切行，防止转向
            {
                counterRes++;
                if (counterRes > 10)
                {
                    track.pointsEdgeLeft.clear();
                    track.pointsEdgeRight.clear();
                }
                else
                {
                    uint16_t rowBreakLeft = searchBreakLeft(track.pointsEdgeLeft); //左上拐点搜索

                    uint16_t rowBreakRight = searchBreakRight(track.pointsEdgeRight); //右下补线点搜索
                    track.pointsEdgeLeft.resize(track.pointsEdgeLeft.size() / 3);

                    if (rowBreakLeft > 0 && rowBreakLeft < track.pointsEdgeLeft.size())
                        track.pointsEdgeLeft.resize(rowBreakLeft);
                    else
                        track.pointsEdgeLeft.resize(track.pointsEdgeLeft.size() / 3);

                    if (rowBreakRight > 0)
                        track.pointsEdgeRight.resize(rowBreakRight);
                    else
                        track.pointsEdgeRight.resize(track.pointsEdgeRight.size() / 3);
                }
            }

            _pointStop = searchStopSign(predict);
            if (_pointStop.x > 0)
            {
                counterRec++;
                if (counterRec >= 5)
                {
                    freezoneStep = FreezoneStep::FreezoneEnter; //入泛行区使能
                    pointsStopRepair.clear();
                    counterRec = 0;
                    counterSession = 0;
                    leftChanel = false; //禁行标志左右通道补线标志
                }
            }
            break;
        }
        case FreezoneStep::FreezoneEnter: //[03] 进入泛行区
        {
            if (track.pointsEdgeLeft.size() > ROWSIMAGE / 4 && track.pointsEdgeRight.size() > ROWSIMAGE / 4)
            {
                if ((track.pointsEdgeLeft[0].y < COLSIMAGE * 0.35 && !leftChanel) || (track.pointsEdgeRight[0].y > COLSIMAGE * 0.65 && leftChanel))
                    counterSession++;
                if (counterSession > 2)
                {
                    freezoneStep = FreezoneStep::FreezoneNone;
                    counterSession = 0;
                    counterRec = 0;
                    slowDown = true;
                }
            }
            else
                counterSession = 0;

            _pointStop = searchStopSign(predict);
            if (_pointStop.y > COLSIMAGE * 0.2 && _pointStop.y < COLSIMAGE / 2 && _pointStop.x > ROWSIMAGE / 3) //禁行标志在【左边】，且进入车辆转向范围
            {
                POINT startPoint;
                if (_pointStop.x < ROWSIMAGE - 10)
                    startPoint = POINT(ROWSIMAGE - 10, _pointStop.y / 2); //补线起点
                else
                    startPoint = POINT(ROWSIMAGE - 1, _pointStop.y / 2);          //补线起点
                POINT endPoint = POINT(20, (_pointStop.y + COLSIMAGE * 0.6) / 2); //补线终点：右
                POINT midPoint = _pointStop;                                      //补线中点
                vector<POINT> input = {startPoint, midPoint, endPoint};
                vector<POINT> repair = Bezier(0.04, input);

                track.pointsEdgeLeft = repair;
                track.pointsEdgeRight = repair;
                for (int i = 0; i < repair.size(); i++)
                {
                    track.pointsEdgeRight[i].y = COLSIMAGE - 1;
                }
                pointsStopRepair.push_back(_pointStop);
                leftChanel = false; //禁行标志左右通道补线标志
            }
            else if (_pointStop.y > COLSIMAGE / 2 && _pointStop.y < COLSIMAGE * 0.8 && _pointStop.x > ROWSIMAGE / 3) //禁行标志在【右边】，且进入车辆转向范围
            {
                POINT startPoint;
                if (_pointStop.x < ROWSIMAGE - 10)
                    startPoint = POINT(ROWSIMAGE - 10, _pointStop.y); //补线起点
                else
                    startPoint = POINT(ROWSIMAGE - 1, _pointStop.y); //补线起点

                POINT endPoint = POINT(20, _pointStop.y / 2); //补线终点：右
                POINT midPoint = _pointStop;                  //补线中点
                vector<POINT> input = {startPoint, midPoint, endPoint};
                vector<POINT> repair = Bezier(0.04, input);

                track.pointsEdgeLeft = repair;
                track.pointsEdgeRight = repair;
                for (int i = 0; i < repair.size(); i++)
                {
                    track.pointsEdgeLeft[i].y = 0;
                }
                pointsStopRepair.push_back(_pointStop);
                leftChanel = true; //禁行标志左右通道补线标志
            }

            else if (_pointStop.x == 0 && pointsStopRepair.size() > 0)
            {
                counterRec++;
                if (counterRec > 5)
                {
                    freezoneStep = FreezoneStep::FreezoneExit;
                    counterRec = 0;
                    counterSession = 0;

                    if (pointsStopRepair.size() > 0) //提升车辆过禁行标志后的转向速度和程度
                    {
                        if (rateTurnFreezone < 0 && rateTurnFreezone >= -1.0)
                        {
                            int size = (int)(pointsStopRepair.size() * (rateTurnFreezone + 1));
                            pointsStopRepair.resize(size);
                        }
                        else if (rateTurnFreezone > 0 && rateTurnFreezone <= 1)
                        {
                            vector<POINT> extraPoints;

                            int size = (int)(pointsStopRepair.size() * rateTurnFreezone);
                            for (int i = 0; i < size; i++)
                            {
                                extraPoints.push_back(pointsStopRepair[i]);
                            }

                            for (int i = 0; i < extraPoints.size(); i++)
                                pointsStopRepair.push_back(extraPoints[i]);
                        }
                    }
                }
            }
            break;
        }

        case FreezoneStep::FreezoneExit: //[04] 出泛行区
        {
            if (track.pointsEdgeLeft.size() > ROWSIMAGE / 4 && track.pointsEdgeRight.size() > ROWSIMAGE / 4)
            {
                if ((track.pointsEdgeLeft[0].y < COLSIMAGE * 0.35 && !leftChanel) || (track.pointsEdgeRight[0].y > COLSIMAGE * 0.65 && leftChanel))
                    counterSession++;
                if (counterSession > 5)
                {
                    freezoneStep = FreezoneStep::FreezoneSlow;
                    counterSession = 0;
                    counterRec = 0;
                    slowDown = true;
                }
            }
            else
                counterSession = 0;

            if (leftChanel && pointsStopRepair.size() > 0) //走禁行标志左边
            {
                _pointStop = pointsStopRepair[0];
                _pointStop.y = COLSIMAGE - _pointStop.y;
                POINT startPoint;
                if (_pointStop.x < ROWSIMAGE - 10)
                    startPoint = POINT(ROWSIMAGE - 10, _pointStop.y); //补线起点
                else
                    startPoint = POINT(ROWSIMAGE - 1, _pointStop.y); //补线起点

                int colTurn = COLSIMAGE - (COLSIMAGE - _pointStop.y) / 4;
                POINT endPoint = POINT(20, colTurn); //补线终点：右
                POINT midPoint = _pointStop;         //补线中点
                vector<POINT> input = {startPoint, midPoint, endPoint};
                vector<POINT> repair = Bezier(0.04, input);

                track.pointsEdgeLeft = repair;
                track.pointsEdgeRight = repair;
                for (int i = 0; i < repair.size(); i++)
                {
                    track.pointsEdgeRight[i].y = COLSIMAGE - 1;
                }
                pointsStopRepair.erase(pointsStopRepair.begin()); //删除第一组转向数据
            }
            else if (!leftChanel && pointsStopRepair.size() > 0) //走禁行标志右边
            {
                _pointStop = pointsStopRepair[0];
                _pointStop.y = COLSIMAGE - _pointStop.y;
                POINT startPoint;
                if (_pointStop.x < ROWSIMAGE - 10)
                    startPoint = POINT(ROWSIMAGE - 10, _pointStop.y); //补线起点
                else
                    startPoint = POINT(ROWSIMAGE - 1, _pointStop.y); //补线起点

                POINT midPoint = _pointStop;                     //补线中点
                POINT endPoint = POINT(20, _pointStop.y * 0.65); //补线终点：右
                vector<POINT> input = {startPoint, midPoint, endPoint};
                vector<POINT> repair = Bezier(0.04, input);

                track.pointsEdgeLeft = repair;
                track.pointsEdgeRight = repair;
                for (int i = 0; i < repair.size(); i++)
                {
                    track.pointsEdgeLeft[i].y = 0;
                }
                pointsStopRepair.erase(pointsStopRepair.begin()); //删除第一组转向数据
            }

            break;
        }
        case FreezoneStep::FreezoneSlow: //[04] 出泛行区
        {
            counterSession++;
            if (counterSession > 30)
            {
                freezoneStep = FreezoneStep::FreezoneNone;
                counterSession = 0;
                counterRec = 0;
                slowDown = true;
                track.pointsEdgeLeft.resize(track.pointsEdgeLeft.size() * 0.6);
                track.pointsEdgeRight.resize(track.pointsEdgeRight.size() * 0.6);
            }
            break;
        }
        }

        if (freezoneStep == FreezoneStep::FreezoneNone) //返回泛行区控制模式标志
            return false;
        else
            return true;
    }

    /**
     * @brief 识别结果图像绘制
     *
     */
    void drawImage(TrackRecognition track, Mat &image)
    {
        // 赛道边缘
        for (int i = 0; i < track.pointsEdgeLeft.size(); i++)
        {
            circle(image, Point(track.pointsEdgeLeft[i].y, track.pointsEdgeLeft[i].x), 1,
                   Scalar(0, 255, 0), -1); //绿色点
        }
        for (int i = 0; i < track.pointsEdgeRight.size(); i++)
        {
            circle(image, Point(track.pointsEdgeRight[i].y, track.pointsEdgeRight[i].x), 1,
                   Scalar(0, 255, 255), -1); //黄色点
        }

        //显示加油站状态
        string state = "None";
        switch (freezoneStep)
        {
        case FreezoneEnable:
            state = "Enable";
            break;
        case FreezoneEnter:
            state = "Enter";
            break;
        case FreezoneExit:
            state = "Exit";
            break;
        }
        putText(image, state, Point(COLSIMAGE / 2 - 10, 20), cv::FONT_HERSHEY_TRIPLEX, 0.3, cv::Scalar(0, 255, 0), 1, CV_AA);

        if (_pointStop.x > 0)
            circle(image, Point(_pointStop.y, _pointStop.x), 3, Scalar(200, 200, 200), -1);
    }

private:
    POINT _pointStop;
    enum FreezoneStep
    {
        FreezoneNone = 0, //未触发
        FreezoneEnable,   //泛行区进入使能
        FreezoneEnter,    //入泛行区
        FreezoneExit,     //出泛行区
        FreezoneSlow      //出泛行区减速
    };

    FreezoneStep freezoneStep = FreezoneStep::FreezoneNone; //泛行区行使状态
    uint16_t counterSession = 0;                            //图像场次计数器
    uint16_t counterRec = 0;                                //标志检测计数器
    uint16_t counterRes = 0;
    vector<POINT> pointsStopRepair; //泛行区禁行标志补线点集
    bool leftChanel = false;        //禁行标志左右通道补线标志

    /**
     * @brief 搜索泛行区标志
     *
     * @param predict
     * @return POINT
     */
    POINT searchFreezoneSign(vector<PredictResult> predict)
    {
        POINT freezone(0, 0);
        for (int i = 0; i < predict.size(); i++)
        {
            if (predict[i].label == LABEL_FREEZONE) //标志检测
            {
                return POINT(predict[i].y + predict[i].height / 2, predict[i].x + predict[i].width / 2);
            }
        }

        return freezone;
    }

    /**
     * @brief 搜索禁行标志
     *
     * @param predict
     * @return POINT
     */
    POINT searchStopSign(vector<PredictResult> predict)
    {
        POINT freezone(0, 0);
        for (int i = 0; i < predict.size(); i++)
        {
            if (predict[i].label == LABEL_STOP) //标志检测
            {
                return POINT(predict[i].y + predict[i].height / 2, predict[i].x + predict[i].width / 2);
            }
        }

        return freezone;
    }

    /**
     * @brief 搜索十字赛道突变行（左下）
     *
     * @param pointsEdgeLeft
     * @return uint16_t
     */
    uint16_t searchBreakLeft(vector<POINT> pointsEdgeLeft)
    {
        uint16_t rowBreakLeft = 0;
        uint16_t counter = 0;
        uint16_t counterBottom = 0; //底行过滤计数器

        for (int i = 0; i < pointsEdgeLeft.size() - 20; i++) //寻找左边跳变点
        {
            if (pointsEdgeLeft[i].y > 0)
                counterBottom++;
            else
                counterBottom = 0;

            if (counterBottom > 2)
                return i - 2;
        }

        return rowBreakLeft;
    }

    /**
     * @brief 搜索十字赛道突变行（右下）
     *
     * @param pointsEdgeRight
     * @return uint16_t
     */
    uint16_t searchBreakRight(vector<POINT> pointsEdgeRight)
    {
        uint16_t rowBreakRight = 0;
        uint16_t counter = 0;
        uint16_t counterBottom = 0; //底行过滤计数器

        for (int i = 0; i < pointsEdgeRight.size() - 10; i++) //寻找左边跳变点
        {
            if (pointsEdgeRight[i].y > COLSIMAGE - 20)
                counterBottom++;
            if (counterBottom > 3)
            {
                if (pointsEdgeRight[i].y <= pointsEdgeRight[rowBreakRight].y)
                {
                    rowBreakRight = i;
                    counter = 0;
                }
                else if (pointsEdgeRight[i].y > pointsEdgeRight[rowBreakRight].y) //突变点计数
                {
                    counter++;
                    if (counter > 5)
                        return rowBreakRight;
                }
            }
        }

        return rowBreakRight;
    }
};
