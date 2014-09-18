/*****************************
The MIT License (MIT)

Copyright (c) 2014 Institute of Biomedical Engineering

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
********************************/

#ifndef FINDELECTRODES_H
#define FINDELECTRODES_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <iostream>

#define R 0
#define L 1
#define T 2
#define B 3

class findelectrodes
{
public:
    findelectrodes(const cv::Mat &image, int orientation);
    ~findelectrodes();
    void find();
    void drawDebug();

    std::vector<cv::Point2f> stripe8_0;
    std::vector<cv::Point2f> stripe8_1;
    std::vector<cv::Point2f> stripe8_2;
    std::vector<cv::Point2f> stripe8_3;
    std::vector<cv::Point2f> stripe12_0;
    std::vector<cv::Point2f> stripe12_1;
    std::vector<cv::Point2f> stripe12_2;
    std::vector<cv::Point2f> stripe12_3;

private:
    void sortCentroids(std::vector<cv::Point2f> &centroids);
    void checkid(std::vector<int> id, std::vector<cv::Point2f> &centroids);

    cv::Mat img;
    cv::Mat thresh;

    int ori;
    std::vector<std::vector<cv::Point> > contours;
    std::vector<int> stripes;
    std::vector<cv::Vec4i> hierarchy;
};

#endif // FINDELECTRODES_H
