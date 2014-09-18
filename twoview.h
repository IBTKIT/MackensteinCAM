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

#ifndef TwoView_H
#define TwoView_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <iostream>
#include <cstdio>
#include <fstream>
#include <cmath>

#include "matches.h"
#include "aruco/findmarker.h"
#include "findelectrodes.h"

class TwoView
{
public:
    TwoView(cv::Mat img0, cv::Mat img1 ,int orient0, int orient1, std::string path_, std::string save_);
    ~TwoView();
    void addPoint(int imgNo,unsigned int x, unsigned int y);
    void removePoint(int imgNo,unsigned int x, unsigned int y);
    int removePointInStripe(int imgNo,unsigned int x, unsigned int y, match *m);
    void removeAllPoints();
    void addMatch(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, int id, std::string description);
    void addMatch(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1);
    void modifySubimgsMatch(unsigned int x, unsigned int y);
    void addFundamentalmarkers();
    void addElectrodemarkers();
    cv::Mat getImageWithPoints(int imgNo);
    void toggleDrawEpi();
    cv::Mat getSubimage();
    cv::Mat getNextSubimage();
    void calculateFundamentalMat( int mehtod, int markers);
    void triangulateElectrodes();
    void triangulateFundamentalmarkers();
    void triangulate(match *m);
    void writeMatlabfile();
    void writeElectrodefile();
    int calculateDistances( std::vector< std::vector<int> > idlist);
    void calculateAngles( std::vector< std::vector<int> > idlist);
    void setSize(int size);
    match* getPointstorage();
    void setPointstorage(int storage);
    void setSavefile(std::string save_);
    void saveMatches();
    void loadMatches();
    void matchelectrodes();

    match* fundamentalMatches;
    match* anatomicalMarkers;
    match* stripe8_0;
    match* stripe8_1;
    match* stripe8_2;
    match* stripe8_3;
    match* stripe12_0;
    match* stripe12_1;
    match* stripe12_2;
    match* stripe12_3;
    
	int encounterederrors;

private:
    void drawImages();
    void drawEpilines();
    void drawSubimage(int pos);
    void getValidpoints( std::vector<cv::Point2f> &points0, std::vector<cv::Point2f> &points1, match *m);
    void getValid3dpoints( std::vector<cv::Point3f> &points0, std::vector<cv::Point3f> &points1, match *m);
    void setValid3dpoints( cv::Mat mat0, cv::Mat mat1, match *m);

    int drawEpi;
    unsigned int SubimgPos;
    unsigned int nextSubimgPos;
    std::vector< cv::Point2i > manual;
    cv::Point2f newPoint[2];
    cv::Mat orig_img0;
    cv::Mat orig_img1;
    cv::Mat orig_img0bw;
    cv::Mat orig_img1bw;
    int orientation0;
    int orientation1;
    cv::Mat drawn_img0;
    cv::Mat drawn_img1;
    cv::Mat withEpilines;
    cv::Mat subImage;
    cv::Mat fundamental;
    cv::Mat intrinsic_LTRT;
    cv::Mat intrinsic_LTRT_inv;
    cv::Mat intrinsic_LLRR;
    cv::Mat intrinsic_LLRR_inv;
    cv::Mat distCoeffs_LLRR;
    cv::Mat distCoeffs_LTRT;
    cv::Mat essential;
    cv::Mat cameraP0;
    std::vector<cv::Mat> cameraP1;
    int circleRadius;
    int fontScale;
    int pointStorage; //0 = Fundamental, 1 = Electrodes
    std::string path;
    std::string save;
    findmarker* markerfinder;
    findelectrodes *electrodes0;
    findelectrodes *electrodes1;
    cv::Mat tmp;
};

#endif // TwoView_H
