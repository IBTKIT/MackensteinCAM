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

#include <iostream>
#include <stdio.h>
#include <string.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "twoview.h"
#include "matches.h"
#include "aruco/aruco.h"


//catch mouseevents of the first window
void mousecallback0(int event, int x, int y, int flags, void *s)
{
    switch(event)
    {
        case CV_EVENT_LBUTTONDOWN:
            ((TwoView*)s)->addPoint(0, 4*x, 4*y); //factor 4 is linked with window scaling in twoview.cpp. See comment #WINDOWSCALING
            cv::imshow("Image0", ((TwoView*)s)->getImageWithPoints(0));
            cv::imshow("Image1", ((TwoView*)s)->getImageWithPoints(1));
            break;
        case CV_EVENT_RBUTTONDOWN:
            ((TwoView*)s)->removePoint(0, 4*x, 4*y); //factor 4 is linked with window scaling in twoview.cpp. See comment #WINDOWSCALING
            cv::imshow("Image0", ((TwoView*)s)->getImageWithPoints(0));
            cv::imshow("Image1", ((TwoView*)s)->getImageWithPoints(1));
            break;
    }
}

//catch mouseevents of the second window
void mousecallback1(int event, int x, int y, int flags, void *s)
{
    switch(event)
    {
        case CV_EVENT_LBUTTONDOWN:
            ((TwoView*)s)->addPoint(1, 4*x, 4*y); //factor 4 is linked with window scaling in twoview.cpp. See comment #WINDOWSCALING
            cv::imshow("Image0", ((TwoView*)s)->getImageWithPoints(0));
            cv::imshow("Image1", ((TwoView*)s)->getImageWithPoints(1));
            break;
        case CV_EVENT_RBUTTONDOWN:
            ((TwoView*)s)->removePoint(1, 4*x, 4*y); //factor 4 is linked with window scaling in twoview.cpp. See comment #WINDOWSCALING
            cv::imshow("Image0", ((TwoView*)s)->getImageWithPoints(0));
            cv::imshow("Image1", ((TwoView*)s)->getImageWithPoints(1));
            break;
    }
}

//catch mouseevent of the refinement window
void mousecallbackRefine(int event, int x, int y, int flags, void *pair1)
{
    switch(event)
    {
        case CV_EVENT_LBUTTONDOWN:
            ((TwoView*)pair1)->modifySubimgsMatch(x, y);
            cv::imshow("Image0", ((TwoView*)pair1)->getImageWithPoints(0));
            cv::imshow("Image1", ((TwoView*)pair1)->getImageWithPoints(1));
            cv::imshow("Refine", ((TwoView*)pair1)->getSubimage());
            break;
    }
}

void printhelp(std::string cmd);

void join(TwoView *s1, TwoView *s2, TwoView *d);
void scale(TwoView* s);

int main(int argc, char *argv[])
{
    string path = ""; //realtive path for all files
    cv::Mat original_img0; //four images which are passed to the program
    cv::Mat original_img1;
    cv::Mat original_img2;
    cv::Mat original_img3;

    for(int i = 0; i < argc; i++) //checking arguments at startup
    {
        if(( !strcmp(argv[i], "-p") || !strcmp(argv[i], "-path") ) && i+1 < argc)
        {
            path = argv[i+1];
            if(path.length() >= 1 && path[path.length()-1] != '/' )
            {
                path.append("/");
            }
        }
        if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") || !strcmp(argv[i], "-help"))
        {
            printhelp(argv[0]);
        }
    }

    for(int i = 0; i < argc; i++) //...still checking arguments
    {
        if(!strcmp(argv[i], "-ll") && i+1 < argc)
        {
            original_img0 = cv::imread(path + argv[i+1]);
            std::cout << path + argv[i+1] << std::endl;
        }
        if(!strcmp(argv[i], "-lt") && i+1 < argc)
        {
            original_img1 = cv::imread(path + argv[i+1]);
            std::cout << path + argv[i+1] << std::endl;
        }
        if(!strcmp(argv[i], "-rt") && i+1 < argc)
        {
            original_img2 = cv::imread(path + argv[i+1]);
            std::cout << path + argv[i+1] << std::endl;
        }
        if(!strcmp(argv[i], "-rr") && i+1 < argc)
        {
            original_img3 = cv::imread(path + argv[i+1]);
            std::cout << path + argv[i+1] << std::endl;
        }
    }

    std::fstream fCAMS;  //opening camera calibration file and checking existance (obtained by calibration tool of opencv-lib)
    fCAMS.open( (path + "calibLLRR_CAMS.xml").c_str(), std::ios::in);
    if(fCAMS == NULL)
    {
        std::cout << "\nERROR: \"calibLLRR_CAMS.xml\" has to be placed in working directory\n\n";
        printhelp(argv[0]);
        return 1;
    }
    fCAMS.close();

    std::fstream fCAMH;  //opening camera calibration file and checking existance (obtained by calibration tool of opencv-lib)
    fCAMH.open( (path + "calibLTRT_CAMH.xml").c_str(), std::ios::in);
    if(fCAMH == NULL)
    {
        std::cout << "\nERROR: \"calibLTRT_CAMH.xml\" has to be placed in working directory\n\n";
        printhelp(argv[0]);
        return 1;
    }
    fCAMH.close();

    if( !original_img0.data ) //opening the four passed images
    {
        std::cout << "Error opening left image of left image pair (ll) .\n\n";
        printhelp(argv[0]);
        return 1;
    }

    if( !original_img1.data )
    {
        std::cout << "Error opening top image of left image pair (lt) .\n\n";
        printhelp(argv[0]);
        return 1;
    }

    if( !original_img2.data )
    {
        std::cout << "Error opening top image of right image pair (rt) .\n\n";
        printhelp(argv[0]);
        return 1;
    }

    if( !original_img3.data )
    {
        std::cout << "Error opening right image of right image pair (rr) .\n\n";
        printhelp(argv[0]);
        return 1;
    }

    cv::namedWindow("Image0", 0); //creating main windows for left and right image of working image pair
    cv::namedWindow("Image1", 0);

    //creating two instances of twoview class for each image pair, clone() creates a copy of an image: in opencv call by value acts like call by pointer/reference by default
    TwoView *pair1 = new TwoView(original_img0.clone(), original_img1.clone(), R, B, path, "pair1"); //original_img0: ll, original_img1: lt
    TwoView *pair2 = new TwoView(original_img3.clone(), original_img2.clone(), L, B, path, "pair2"); //original_img3: rr, original_img2: rt
    TwoView *s = pair1; //working image pair

    cv::imshow("Image0", s->getImageWithPoints(0)); //open main windows
    cv::imshow("Image1", s->getImageWithPoints(1));
    cv::setMouseCallback("Image0", mousecallback0, s); //enable mouse events for working (main) windows
    cv::setMouseCallback("Image1", mousecallback1, s);

    for(int key = 0; key != 'q';) //'q' exists the program, check for other entered keys in switch loop...
    {
        key = cv::waitKey(); //reading next key
        switch(key)
        {
          case 'e': // enable/disable drawing of epipolarlines
             pair1->toggleDrawEpi();
             pair2->toggleDrawEpi();
             cv::imshow("Image0", s->getImageWithPoints(0));
             cv::imshow("Image1", s->getImageWithPoints(1));
             break;
          case 'f': // calculate fundamental matrix
             pair1->calculateFundamentalMat(CV_FM_RANSAC, 1);  // markers: 0 = just fundamentalMatches, 1 = just electrodes, else = all
             pair2->calculateFundamentalMat(CV_FM_RANSAC, 1);  // !!!! change here as well !!!!
             cv::imshow("Image0", s->getImageWithPoints(0));
             cv::imshow("Image1", s->getImageWithPoints(1));
             break;
          case 'r': //open refinement window
            cv::namedWindow("Refine", 0);
            cv::setMouseCallback("Refine", mousecallbackRefine, s);
            cv::imshow("Refine", s->getNextSubimage());
            cv::imshow("Image0", s->getImageWithPoints(0));
            cv::imshow("Image1", s->getImageWithPoints(1));
            break;
          case 't': //triangulate points, write pointclouds to matlab files, join pointclouds and write final pointcloud to matlab file
            pair1->triangulateElectrodes();
            pair1->triangulateFundamentalmarkers();
            scale(pair1);
            pair1->writeMatlabfile();
            pair1->writeElectrodefile();
            pair2->triangulateElectrodes();
            pair2->triangulateFundamentalmarkers();
            scale(pair2);
            pair2->writeMatlabfile();
            pair2->writeElectrodefile();

            join(pair1, pair2, pair1);
            pair1->setSavefile("bothpairs");
            pair1->writeMatlabfile();
            pair1->writeElectrodefile();
            pair1->setSavefile("pair1");

			if ((pair1->encounterederrors != 0)||(pair2->encounterederrors != 0))
			{
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "" << std::endl;
				std::cout << "" << std::endl;
				std::cout << "" << std::endl;
				std::cout << "" << std::endl;
				std::cout << "" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "##############" << std::endl;
				std::cout << "" << std::endl;
				std::cout << "" << std::endl;
				std::cout << "ERRORS ENCOUNTERED. Please check command line output and adjust setup. Number of errors pair1:" << pair1->encounterederrors << ", pair2:" << pair2->encounterederrors << std::endl;
				std::cout << "" << std::endl;
				std::cout << "############## ############## ############## ############## ############## ############## ############## ############## ############## ##############" << std::endl;
			}
            break;
          case 's':
            s->saveMatches();
            break;
          case 'l':
            s->loadMatches();
            cv::imshow("Image0", s->getImageWithPoints(0));
            cv::imshow("Image1", s->getImageWithPoints(1));
            break;
          case 'c':
            s->removeAllPoints();
            cv::imshow("Image0", s->getImageWithPoints(0));
            cv::imshow("Image1", s->getImageWithPoints(1));
            break;
          case 'n':
            pair1->addFundamentalmarkers();
            pair2->addFundamentalmarkers();
            cv::imshow("Image0", s->getImageWithPoints(0));
            cv::imshow("Image1", s->getImageWithPoints(1));
            break;
          case 'm':
            pair1->matchelectrodes();
            pair2->matchelectrodes();
            cv::imshow("Image0", s->getImageWithPoints(0));
            cv::imshow("Image1", s->getImageWithPoints(1));
            break;
          case ' ': //toggle working image pair
            if( s == pair1)
            {
                s = pair2;
            }
            else
            {
                s = pair1;
            }
            cv::imshow("Image0", s->getImageWithPoints(0));
            cv::imshow("Image1", s->getImageWithPoints(1));
            cv::setMouseCallback("Image0", mousecallback0, s);
            cv::setMouseCallback("Image1", mousecallback1, s);
            break;
          case '0': //set electrode stripes for refinement and for adding/deleting electrode positions
            s->setPointstorage(0);
            break;
          case '1':
            s->setPointstorage(1);
            break;
          case '2':
            s->setPointstorage(2);
            break;
          case '3':
            s->setPointstorage(3);
            break;
          case '4':
            s->setPointstorage(4);
            break;
          case '5':
            s->setPointstorage(5);
            break;
          case '6':
            s->setPointstorage(6);
            break;
          case '7':
            s->setPointstorage(7);
            break;
          case '8':
            s->setPointstorage(8);//Anatomical markers
            break;
          case '9':
            s->setPointstorage(9);//ARUCO MARKERS
            break;
        }
    }
    return 0;
}

void printhelp(std::string cmd)
{
    std::cout << "Usage: " << cmd << " \n-rr \"right image of right image pair\" \n-rt \"top image of right image pair\"\n"
              << "Calibration files for both the cam that took LL, RR (\"calibLLRR_CAMS.xml\") as well as for the cam that took LT, RT (\"calibLTRT_CAMH.xml\")  must be placed in the working directory. "
              << " \n-ll \"left image of left image pair\" \n-lt \"top image of left image pair\"" << std::endl
              << "optional:\n -p \"path\" to working directory\n\n";
    std::cout << "Keys:\n" << "[space] - swap image pairs\n" << "m - search for electrode stripes\n"
              << "n - search for ArUco markers\n"
              << "f - calculate fundamental matrices\n" << "e - draw epipolar lines\n"
              << "t - triangulate electrode positions and write electrode and matlab files\n"
              << "c - clear all points\n"
              << "r - check electrode positions. Before pressing r, set electrode stripe to check (see below). Repeat pressing to walk yourself through the electrodes.\n" << "[0-7, 8, 9] - set electrode stripe to check (0-7). 8: anatomical markers 9: ArUco markers \n"
              << "s - save currently selected electrode strip/markers \n"
              << "l - load coordinates for currently selected electrode strip/markers \n"
              << "[left mouse] - To add electrodes manually, click once on the one, then on the other image of the displayed image pair and enter desired electrode number in command line.\n"
              << "\n"
              << "Output: Produces an *.esf ElectrodeSet file with the electrode specifications and an *.m MATLAB file that plots the found electrodes.\n";
}

void join(TwoView *s1, TwoView *s2, TwoView *d) //joining both pointclouds and saving result to first pointcloud
{
    //for every point of second pointcloud adjusting scale to first pointcloud
    //first: calculating scale factor
	std::cout << "calculating scale factor..." << std::endl;
    cv::Point3f s1c; //center of pointcloud
    cv::Point3f s2c;
    int same = 0;
    double scalefactor = 0;
    int scalenumber = 0;
    int size = (s1->stripe8_0->center3d_0.size() < s2->stripe8_0->center3d_0.size()) ? s1->stripe8_0->center3d_0.size() : s2->stripe8_0->center3d_0.size(); //condition ? value if true: if false
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe8_0->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_0->center3d_0[i] != cv::Point3f (-1,-1,-1) && s1->stripe8_0->center3d_0[i-1] != cv::Point3f (-1,-1,-1) && s2->stripe8_0->center3d_0[i-1] != cv::Point3f (-1,-1,-1))
        {
            s1c += s1->stripe8_0->center3d_0[i];
            s2c += s2->stripe8_0->center3d_0[i];
            if( i > 0)
            {
                double distancesquared1 = 0;
                distancesquared1 += (s1->stripe8_0->center3d_0[i].x - s1->stripe8_0->center3d_0[i-1].x) * (s1->stripe8_0->center3d_0[i].x - s1->stripe8_0->center3d_0[i-1].x);
                distancesquared1 += (s1->stripe8_0->center3d_0[i].y - s1->stripe8_0->center3d_0[i-1].y) * (s1->stripe8_0->center3d_0[i].y - s1->stripe8_0->center3d_0[i-1].y);
                distancesquared1 += (s1->stripe8_0->center3d_0[i].z - s1->stripe8_0->center3d_0[i-1].z) * (s1->stripe8_0->center3d_0[i].z - s1->stripe8_0->center3d_0[i-1].z);
                double distancesquared2 = 0;
                distancesquared2 += (s2->stripe8_0->center3d_0[i].x - s2->stripe8_0->center3d_0[i-1].x) * (s2->stripe8_0->center3d_0[i].x - s2->stripe8_0->center3d_0[i-1].x);
                distancesquared2 += (s2->stripe8_0->center3d_0[i].y - s2->stripe8_0->center3d_0[i-1].y) * (s2->stripe8_0->center3d_0[i].y - s2->stripe8_0->center3d_0[i-1].y);
                distancesquared2 += (s2->stripe8_0->center3d_0[i].z - s2->stripe8_0->center3d_0[i-1].z) * (s2->stripe8_0->center3d_0[i].z - s2->stripe8_0->center3d_0[i-1].z);
				if ( std::sqrt( distancesquared2) != 0 )
				{
                	scalefactor += std::sqrt( distancesquared1) / std::sqrt( distancesquared2);
                	scalenumber++;
					std::cout << "stripe8_0distancesquared (l,r pair):" << sqrt( distancesquared1) << "," << sqrt( distancesquared2) << std::endl; //make sure comment complies with lines: TwoView *pair1 = new.. TwoView *pair2 = new..
				}
            }
            same++;
        }
    }

    size = (s1->stripe8_1->center3d_0.size() < s2->stripe8_1->center3d_0.size()) ? s1->stripe8_1->center3d_0.size() : s2->stripe8_1->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe8_1->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_1->center3d_0[i] != cv::Point3f (-1,-1,-1) && s1->stripe8_1->center3d_0[i-1] != cv::Point3f (-1,-1,-1) && s2->stripe8_1->center3d_0[i-1] != cv::Point3f (-1,-1,-1))
        {
            s1c += s1->stripe8_1->center3d_0[i];
            s2c += s2->stripe8_1->center3d_0[i];
            if( i > 0)
            {
                double distancesquared1 = 0;
                distancesquared1 += (s1->stripe8_1->center3d_0[i].x - s1->stripe8_1->center3d_0[i-1].x) * (s1->stripe8_1->center3d_0[i].x - s1->stripe8_1->center3d_0[i-1].x);
                distancesquared1 += (s1->stripe8_1->center3d_0[i].y - s1->stripe8_1->center3d_0[i-1].y) * (s1->stripe8_1->center3d_0[i].y - s1->stripe8_1->center3d_0[i-1].y);
                distancesquared1 += (s1->stripe8_1->center3d_0[i].z - s1->stripe8_1->center3d_0[i-1].z) * (s1->stripe8_1->center3d_0[i].z - s1->stripe8_1->center3d_0[i-1].z);
                double distancesquared2 = 0;
                distancesquared2 += (s2->stripe8_1->center3d_0[i].x - s2->stripe8_1->center3d_0[i-1].x) * (s2->stripe8_1->center3d_0[i].x - s2->stripe8_1->center3d_0[i-1].x);
                distancesquared2 += (s2->stripe8_1->center3d_0[i].y - s2->stripe8_1->center3d_0[i-1].y) * (s2->stripe8_1->center3d_0[i].y - s2->stripe8_1->center3d_0[i-1].y);
                distancesquared2 += (s2->stripe8_1->center3d_0[i].z - s2->stripe8_1->center3d_0[i-1].z) * (s2->stripe8_1->center3d_0[i].z - s2->stripe8_1->center3d_0[i-1].z);
				if ( std::sqrt( distancesquared2) != 0 )
				{
                scalefactor += std::sqrt( distancesquared1) / std::sqrt( distancesquared2);
                scalenumber++;
				std::cout << "stripe8_1distancesquared (l,r pair):" << sqrt( distancesquared1) << "," << sqrt( distancesquared2) << std::endl; //make sure comment complies with lines: TwoView *pair1 = new.. TwoView *pair2 = new..
                }
            }
            same++;
        }
    }

    size = (s1->stripe8_2->center3d_0.size() < s2->stripe8_2->center3d_0.size()) ? s1->stripe8_2->center3d_0.size() : s2->stripe8_2->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe8_2->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_2->center3d_0[i] != cv::Point3f (-1,-1,-1) && s1->stripe8_2->center3d_0[i-1] != cv::Point3f (-1,-1,-1) && s2->stripe8_2->center3d_0[i-1] != cv::Point3f (-1,-1,-1))
        {
            s1c += s1->stripe8_2->center3d_0[i];
            s2c += s2->stripe8_2->center3d_0[i];
            if( i > 0)
            {
                double distancesquared1 = 0;
                distancesquared1 += (s1->stripe8_2->center3d_0[i].x - s1->stripe8_2->center3d_0[i-1].x) * (s1->stripe8_2->center3d_0[i].x - s1->stripe8_2->center3d_0[i-1].x);
                distancesquared1 += (s1->stripe8_2->center3d_0[i].y - s1->stripe8_2->center3d_0[i-1].y) * (s1->stripe8_2->center3d_0[i].y - s1->stripe8_2->center3d_0[i-1].y);
                distancesquared1 += (s1->stripe8_2->center3d_0[i].z - s1->stripe8_2->center3d_0[i-1].z) * (s1->stripe8_2->center3d_0[i].z - s1->stripe8_2->center3d_0[i-1].z);
                double distancesquared2 = 0;
                distancesquared2 += (s2->stripe8_2->center3d_0[i].x - s2->stripe8_2->center3d_0[i-1].x) * (s2->stripe8_2->center3d_0[i].x - s2->stripe8_2->center3d_0[i-1].x);
                distancesquared2 += (s2->stripe8_2->center3d_0[i].y - s2->stripe8_2->center3d_0[i-1].y) * (s2->stripe8_2->center3d_0[i].y - s2->stripe8_2->center3d_0[i-1].y);
                distancesquared2 += (s2->stripe8_2->center3d_0[i].z - s2->stripe8_2->center3d_0[i-1].z) * (s2->stripe8_2->center3d_0[i].z - s2->stripe8_2->center3d_0[i-1].z);
				if ( std::sqrt( distancesquared2) != 0 )
				{
                scalefactor += std::sqrt( distancesquared1) / std::sqrt( distancesquared2);
                scalenumber++;
				std::cout << "stripe8_2distancesquared (l,r pair):" << sqrt( distancesquared1) << "," << sqrt( distancesquared2) << std::endl; //make sure comment complies with lines: TwoView *pair1 = new.. TwoView *pair2 = new..
                }
            }
            same++;
        }
    }

    size = (s1->stripe8_3->center3d_0.size() < s2->stripe8_3->center3d_0.size()) ? s1->stripe8_3->center3d_0.size() : s2->stripe8_3->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe8_3->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_3->center3d_0[i] != cv::Point3f (-1,-1,-1) && s1->stripe8_3->center3d_0[i-1] != cv::Point3f (-1,-1,-1) && s2->stripe8_3->center3d_0[i-1] != cv::Point3f (-1,-1,-1))
        {
            s1c += s1->stripe8_3->center3d_0[i];
            s2c += s2->stripe8_3->center3d_0[i];
            if( i > 0)
            {
                double distancesquared1 = 0;
                distancesquared1 += (s1->stripe8_3->center3d_0[i].x - s1->stripe8_3->center3d_0[i-1].x) * (s1->stripe8_3->center3d_0[i].x - s1->stripe8_3->center3d_0[i-1].x);
                distancesquared1 += (s1->stripe8_3->center3d_0[i].y - s1->stripe8_3->center3d_0[i-1].y) * (s1->stripe8_3->center3d_0[i].y - s1->stripe8_3->center3d_0[i-1].y);
                distancesquared1 += (s1->stripe8_3->center3d_0[i].z - s1->stripe8_3->center3d_0[i-1].z) * (s1->stripe8_3->center3d_0[i].z - s1->stripe8_3->center3d_0[i-1].z);
                double distancesquared2 = 0;
                distancesquared2 += (s2->stripe8_3->center3d_0[i].x - s2->stripe8_3->center3d_0[i-1].x) * (s2->stripe8_3->center3d_0[i].x - s2->stripe8_3->center3d_0[i-1].x);
                distancesquared2 += (s2->stripe8_3->center3d_0[i].y - s2->stripe8_3->center3d_0[i-1].y) * (s2->stripe8_3->center3d_0[i].y - s2->stripe8_3->center3d_0[i-1].y);
                distancesquared2 += (s2->stripe8_3->center3d_0[i].z - s2->stripe8_3->center3d_0[i-1].z) * (s2->stripe8_3->center3d_0[i].z - s2->stripe8_3->center3d_0[i-1].z);
				if ( std::sqrt( distancesquared2) != 0 )
				{
                scalefactor += std::sqrt( distancesquared1) / std::sqrt( distancesquared2);
                scalenumber++;
				std::cout << "stripe8_3distancesquared (l,r pair):" << sqrt( distancesquared1) << "," << sqrt( distancesquared2) << std::endl; //make sure comment complies with lines: TwoView *pair1 = new.. TwoView *pair2 = new..
                }
            }
            same++;
        }
    }

    size = (s1->stripe12_0->center3d_0.size() < s2->stripe12_0->center3d_0.size()) ? s1->stripe12_0->center3d_0.size() : s2->stripe12_0->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe12_0->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_0->center3d_0[i] != cv::Point3f (-1,-1,-1) &&  s1->stripe12_0->center3d_0[i-1] != cv::Point3f (-1,-1,-1) && s2->stripe12_0->center3d_0[i-1] != cv::Point3f (-1,-1,-1))
        {
            s1c += s1->stripe12_0->center3d_0[i];
            s2c += s2->stripe12_0->center3d_0[i];
            if( i > 0)
            {
                double distancesquared1 = 0;
                distancesquared1 += (s1->stripe12_0->center3d_0[i].x - s1->stripe12_0->center3d_0[i-1].x) * (s1->stripe12_0->center3d_0[i].x - s1->stripe12_0->center3d_0[i-1].x);
                distancesquared1 += (s1->stripe12_0->center3d_0[i].y - s1->stripe12_0->center3d_0[i-1].y) * (s1->stripe12_0->center3d_0[i].y - s1->stripe12_0->center3d_0[i-1].y);
                distancesquared1 += (s1->stripe12_0->center3d_0[i].z - s1->stripe12_0->center3d_0[i-1].z) * (s1->stripe12_0->center3d_0[i].z - s1->stripe12_0->center3d_0[i-1].z);
                double distancesquared2 = 0;
                distancesquared2 += (s2->stripe12_0->center3d_0[i].x - s2->stripe12_0->center3d_0[i-1].x) * (s2->stripe12_0->center3d_0[i].x - s2->stripe12_0->center3d_0[i-1].x);
                distancesquared2 += (s2->stripe12_0->center3d_0[i].y - s2->stripe12_0->center3d_0[i-1].y) * (s2->stripe12_0->center3d_0[i].y - s2->stripe12_0->center3d_0[i-1].y);
                distancesquared2 += (s2->stripe12_0->center3d_0[i].z - s2->stripe12_0->center3d_0[i-1].z) * (s2->stripe12_0->center3d_0[i].z - s2->stripe12_0->center3d_0[i-1].z);
				if ( std::sqrt( distancesquared2) != 0 )
				{
                scalefactor += std::sqrt( distancesquared1) / std::sqrt( distancesquared2);
                scalenumber++;
				std::cout << "stripe12_0distancesquared (l,r pair):" << sqrt( distancesquared1) << "," << sqrt( distancesquared2) << std::endl; //make sure comment complies with lines: TwoView *pair1 = new.. TwoView *pair2 = new..
                }
            }
            same++;
        }
    }

    size = (s1->stripe12_1->center3d_0.size() < s2->stripe12_1->center3d_0.size()) ? s1->stripe12_1->center3d_0.size() : s2->stripe12_1->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe12_1->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_1->center3d_0[i] != cv::Point3f (-1,-1,-1) && s1->stripe12_1->center3d_0[i-1] != cv::Point3f (-1,-1,-1) && s2->stripe12_1->center3d_0[i-1] != cv::Point3f (-1,-1,-1))
        {
            s1c += s1->stripe12_1->center3d_0[i];
            s2c += s2->stripe12_1->center3d_0[i];
            if( i > 0)
            {
                double distancesquared1 = 0;
                distancesquared1 += (s1->stripe12_1->center3d_0[i].x - s1->stripe12_1->center3d_0[i-1].x) * (s1->stripe12_1->center3d_0[i].x - s1->stripe12_1->center3d_0[i-1].x);
                distancesquared1 += (s1->stripe12_1->center3d_0[i].y - s1->stripe12_1->center3d_0[i-1].y) * (s1->stripe12_1->center3d_0[i].y - s1->stripe12_1->center3d_0[i-1].y);
                distancesquared1 += (s1->stripe12_1->center3d_0[i].z - s1->stripe12_1->center3d_0[i-1].z) * (s1->stripe12_1->center3d_0[i].z - s1->stripe12_1->center3d_0[i-1].z);
                double distancesquared2 = 0;
                distancesquared2 += (s2->stripe12_1->center3d_0[i].x - s2->stripe12_1->center3d_0[i-1].x) * (s2->stripe12_1->center3d_0[i].x - s2->stripe12_1->center3d_0[i-1].x);
                distancesquared2 += (s2->stripe12_1->center3d_0[i].y - s2->stripe12_1->center3d_0[i-1].y) * (s2->stripe12_1->center3d_0[i].y - s2->stripe12_1->center3d_0[i-1].y);
                distancesquared2 += (s2->stripe12_1->center3d_0[i].z - s2->stripe12_1->center3d_0[i-1].z) * (s2->stripe12_1->center3d_0[i].z - s2->stripe12_1->center3d_0[i-1].z);
				if ( std::sqrt( distancesquared2) != 0 )
				{
                scalefactor += std::sqrt( distancesquared1) / std::sqrt( distancesquared2);
                scalenumber++;
				std::cout << "stripe12_1distancesquared (l,r pair):" << sqrt( distancesquared1) << "," << sqrt( distancesquared2) << std::endl; //make sure comment complies with lines: TwoView *pair1 = new.. TwoView *pair2 = new..
                }
            }
            same++;
        }
    }

    size = (s1->stripe12_2->center3d_0.size() < s2->stripe12_2->center3d_0.size()) ? s1->stripe12_2->center3d_0.size() : s2->stripe12_2->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe12_2->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_2->center3d_0[i] != cv::Point3f (-1,-1,-1) && s1->stripe12_2->center3d_0[i-1] != cv::Point3f (-1,-1,-1) && s2->stripe12_2->center3d_0[i-1] != cv::Point3f (-1,-1,-1))
        {
            s1c += s1->stripe12_2->center3d_0[i];
            s2c += s2->stripe12_2->center3d_0[i];
            if( i > 0)
            {
                double distancesquared1 = 0;
                distancesquared1 += (s1->stripe12_2->center3d_0[i].x - s1->stripe12_2->center3d_0[i-1].x) * (s1->stripe12_2->center3d_0[i].x - s1->stripe12_2->center3d_0[i-1].x);
                distancesquared1 += (s1->stripe12_2->center3d_0[i].y - s1->stripe12_2->center3d_0[i-1].y) * (s1->stripe12_2->center3d_0[i].y - s1->stripe12_2->center3d_0[i-1].y);
                distancesquared1 += (s1->stripe12_2->center3d_0[i].z - s1->stripe12_2->center3d_0[i-1].z) * (s1->stripe12_2->center3d_0[i].z - s1->stripe12_2->center3d_0[i-1].z);
                double distancesquared2 = 0;
                distancesquared2 += (s2->stripe12_2->center3d_0[i].x - s2->stripe12_2->center3d_0[i-1].x) * (s2->stripe12_2->center3d_0[i].x - s2->stripe12_2->center3d_0[i-1].x);
                distancesquared2 += (s2->stripe12_2->center3d_0[i].y - s2->stripe12_2->center3d_0[i-1].y) * (s2->stripe12_2->center3d_0[i].y - s2->stripe12_2->center3d_0[i-1].y);
                distancesquared2 += (s2->stripe12_2->center3d_0[i].z - s2->stripe12_2->center3d_0[i-1].z) * (s2->stripe12_2->center3d_0[i].z - s2->stripe12_2->center3d_0[i-1].z);
				if ( std::sqrt( distancesquared2) != 0 )
				{
                scalefactor += std::sqrt( distancesquared1) / std::sqrt( distancesquared2);
                scalenumber++;
				std::cout << "stripe12_2distancesquared (l,r pair):" << sqrt( distancesquared1) << "," << sqrt( distancesquared2) << std::endl; //make sure comment complies with lines: TwoView *pair1 = new.. TwoView *pair2 = new..
                }
            }
            same++;
        }
    }

    size = (s1->stripe12_3->center3d_0.size() < s2->stripe12_3->center3d_0.size()) ? s1->stripe12_3->center3d_0.size() : s2->stripe12_3->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe12_3->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_3->center3d_0[i] != cv::Point3f (-1,-1,-1) && s1->stripe12_3->center3d_0[i-1] != cv::Point3f (-1,-1,-1) && s2->stripe12_3->center3d_0[i-1] != cv::Point3f (-1,-1,-1))
        {
            s1c += s1->stripe12_3->center3d_0[i];
            s2c += s2->stripe12_3->center3d_0[i];
            if( i > 0)
            {
                double distancesquared1 = 0;
                distancesquared1 += (s1->stripe12_3->center3d_0[i].x - s1->stripe12_3->center3d_0[i-1].x) * (s1->stripe12_3->center3d_0[i].x - s1->stripe12_3->center3d_0[i-1].x);
                distancesquared1 += (s1->stripe12_3->center3d_0[i].y - s1->stripe12_3->center3d_0[i-1].y) * (s1->stripe12_3->center3d_0[i].y - s1->stripe12_3->center3d_0[i-1].y);
                distancesquared1 += (s1->stripe12_3->center3d_0[i].z - s1->stripe12_3->center3d_0[i-1].z) * (s1->stripe12_3->center3d_0[i].z - s1->stripe12_3->center3d_0[i-1].z);
                double distancesquared2 = 0;
                distancesquared2 += (s2->stripe12_3->center3d_0[i].x - s2->stripe12_3->center3d_0[i-1].x) * (s2->stripe12_3->center3d_0[i].x - s2->stripe12_3->center3d_0[i-1].x);
                distancesquared2 += (s2->stripe12_3->center3d_0[i].y - s2->stripe12_3->center3d_0[i-1].y) * (s2->stripe12_3->center3d_0[i].y - s2->stripe12_3->center3d_0[i-1].y);
                distancesquared2 += (s2->stripe12_3->center3d_0[i].z - s2->stripe12_3->center3d_0[i-1].z) * (s2->stripe12_3->center3d_0[i].z - s2->stripe12_3->center3d_0[i-1].z);
				if ( std::sqrt( distancesquared2) != 0 )
				{
                scalefactor += std::sqrt( distancesquared1) / std::sqrt( distancesquared2);
                scalenumber++;
				std::cout << "stripe12_3distancesquared (l,r pair):" << sqrt( distancesquared1) << "," << sqrt( distancesquared2) << std::endl; //make sure comment complies with lines: TwoView *pair1 = new.. TwoView *pair2 = new..
                }
            }
            same++;
        }
    }

    size = (s1->anatomicalMarkers->center3d_0.size() < s2->anatomicalMarkers->center3d_0.size()) ? s1->anatomicalMarkers->center3d_0.size() : s2->anatomicalMarkers->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->anatomicalMarkers->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->anatomicalMarkers->center3d_0[i] != cv::Point3f (-1,-1,-1) && s1->anatomicalMarkers->center3d_0[i-1] != cv::Point3f (-1,-1,-1) && s2->anatomicalMarkers->center3d_0[i-1] != cv::Point3f (-1,-1,-1))
        {
            s1c += s1->anatomicalMarkers->center3d_0[i];
            s2c += s2->anatomicalMarkers->center3d_0[i];
            if( i > 0)
            {
                double distancesquared1 = 0;
                distancesquared1 += (s1->anatomicalMarkers->center3d_0[i].x - s1->anatomicalMarkers->center3d_0[i-1].x) * (s1->anatomicalMarkers->center3d_0[i].x - s1->anatomicalMarkers->center3d_0[i-1].x);
                distancesquared1 += (s1->anatomicalMarkers->center3d_0[i].y - s1->anatomicalMarkers->center3d_0[i-1].y) * (s1->anatomicalMarkers->center3d_0[i].y - s1->anatomicalMarkers->center3d_0[i-1].y);
                distancesquared1 += (s1->anatomicalMarkers->center3d_0[i].z - s1->anatomicalMarkers->center3d_0[i-1].z) * (s1->anatomicalMarkers->center3d_0[i].z - s1->anatomicalMarkers->center3d_0[i-1].z);
                double distancesquared2 = 0;
                distancesquared2 += (s2->anatomicalMarkers->center3d_0[i].x - s2->anatomicalMarkers->center3d_0[i-1].x) * (s2->anatomicalMarkers->center3d_0[i].x - s2->anatomicalMarkers->center3d_0[i-1].x);
                distancesquared2 += (s2->anatomicalMarkers->center3d_0[i].y - s2->anatomicalMarkers->center3d_0[i-1].y) * (s2->anatomicalMarkers->center3d_0[i].y - s2->anatomicalMarkers->center3d_0[i-1].y);
                distancesquared2 += (s2->anatomicalMarkers->center3d_0[i].z - s2->anatomicalMarkers->center3d_0[i-1].z) * (s2->anatomicalMarkers->center3d_0[i].z - s2->anatomicalMarkers->center3d_0[i-1].z);
				if ( std::sqrt( distancesquared2) != 0 )
				{
                scalefactor += std::sqrt( distancesquared1) / std::sqrt( distancesquared2);
                scalenumber++;
				std::cout << "anatomicalMarkersdistancesquared (l,r pair):" << sqrt( distancesquared1) << "," << sqrt( distancesquared2) << std::endl; //make sure comment complies with lines: TwoView *pair1 = new.. TwoView *pair2 = new..
                }
            }
            same++;
        }
    }

    s1c.x /= same;
    s1c.y /= same;
    s1c.z /= same;
    s2c.x /= same;
    s2c.y /= same;
    s2c.z /= same;
    scalefactor /= scalenumber;

    std::cout << "Note that electrode distances should be either 45mm (for 8) or 30mm (for 12 markers) for the default marker system provided with the software." << std::endl;
    std::cout << "-----------" << std::endl;
    std::cout << "Same: " << same << std::endl;
    std::cout << "Scalenumber: " << scalenumber << std::endl;
    std::cout << "Scalefactor: " << scalefactor << std::endl;

    //second: scaling, translating and rotating second pointcloud

    cv::Mat H = (cv::Mat_<float>(3,3) << 0, 0, 0,
                                         0,  0, 0,
                                         0,  0, 0);
	std::cout << "Computing proper rigid transformation:" << std::endl;

    size = (s1->stripe8_0->center3d_0.size() < s2->stripe8_0->center3d_0.size()) ? s1->stripe8_0->center3d_0.size() : s2->stripe8_0->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe8_0->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_0->center3d_0[i] != cv::Point3f (-1,-1,-1))
        {
			std::cout << "Considering reconstructions of: Stripe8 0 (A1-A8) electrode " << i << std::endl;
            H += cv::Mat( s2->stripe8_0->center3d_0[i] - s2c)*scalefactor * cv::Mat( s1->stripe8_0->center3d_0[i] - s1c).t();
        }
    }

    size = (s1->stripe8_1->center3d_0.size() < s2->stripe8_1->center3d_0.size()) ? s1->stripe8_1->center3d_0.size() : s2->stripe8_1->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe8_1->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_1->center3d_0[i] != cv::Point3f (-1,-1,-1))
        {
			std::cout << "Considering reconstructions of: Stripe8 1 (A9-A16) electrode " << i << std::endl;
            H += cv::Mat( s2->stripe8_1->center3d_0[i] - s2c)*scalefactor * cv::Mat( s1->stripe8_1->center3d_0[i] - s1c).t();
        }
    }

    size = (s1->stripe8_2->center3d_0.size() < s2->stripe8_2->center3d_0.size()) ? s1->stripe8_2->center3d_0.size() : s2->stripe8_2->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe8_2->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_2->center3d_0[i] != cv::Point3f (-1,-1,-1))
        {
			std::cout << "Considering reconstructions of: Stripe8 2 (A17-A24) electrode " << i << std::endl;
            H += cv::Mat( s2->stripe8_2->center3d_0[i] - s2c)*scalefactor * cv::Mat( s1->stripe8_2->center3d_0[i] - s1c).t();
        }
    }

    size = (s1->stripe8_3->center3d_0.size() < s2->stripe8_3->center3d_0.size()) ? s1->stripe8_3->center3d_0.size() : s2->stripe8_3->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe8_3->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_3->center3d_0[i] != cv::Point3f (-1,-1,-1))
        {
			std::cout << "Considering reconstructions of: Stripe8 3 (A25-A32) electrode " << i << std::endl;
            H += cv::Mat( s2->stripe8_3->center3d_0[i] - s2c)*scalefactor * cv::Mat( s1->stripe8_3->center3d_0[i] - s1c).t();
        }
    }

    size = (s1->stripe12_0->center3d_0.size() < s2->stripe12_0->center3d_0.size()) ? s1->stripe12_0->center3d_0.size() : s2->stripe12_0->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe12_0->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_0->center3d_0[i] != cv::Point3f (-1,-1,-1))
        {
			std::cout << "Considering reconstructions of: Stripe12 0 (B1-B12) electrode " << i << std::endl;
            H += cv::Mat( s2->stripe12_0->center3d_0[i] - s2c)*scalefactor * cv::Mat( s1->stripe12_0->center3d_0[i] - s1c).t();
        }
    }

    size = (s1->stripe12_1->center3d_0.size() < s2->stripe12_1->center3d_0.size()) ? s1->stripe12_1->center3d_0.size() : s2->stripe12_1->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe12_1->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_1->center3d_0[i] != cv::Point3f (-1,-1,-1))
        {
			std::cout << "Considering reconstructions of: Stripe12 1 (B13-B24) electrode " << i << std::endl;
            H += cv::Mat( s2->stripe12_1->center3d_0[i] - s2c)*scalefactor * cv::Mat( s1->stripe12_1->center3d_0[i] - s1c).t();
        }
    }

    size = (s1->stripe12_2->center3d_0.size() < s2->stripe12_2->center3d_0.size()) ? s1->stripe12_2->center3d_0.size() : s2->stripe12_2->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe12_2->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_2->center3d_0[i] != cv::Point3f (-1,-1,-1))
        {
			std::cout << "Considering reconstructions of: Stripe12 2 (C1-C12) electrode " << i << std::endl;
            H += cv::Mat( s2->stripe12_2->center3d_0[i] - s2c)*scalefactor * cv::Mat( s1->stripe12_2->center3d_0[i] - s1c).t();
        }
    }

    size = (s1->stripe12_3->center3d_0.size() < s2->stripe12_3->center3d_0.size()) ? s1->stripe12_3->center3d_0.size() : s2->stripe12_3->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->stripe12_3->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_3->center3d_0[i] != cv::Point3f (-1,-1,-1))
        {
			std::cout << "Considering reconstructions of: Stripe12 3 (C13-C24) electrode " << i << std::endl;
            H += cv::Mat( s2->stripe12_3->center3d_0[i] - s2c)*scalefactor * cv::Mat( s1->stripe12_3->center3d_0[i] - s1c).t();
        }
    }

    size = (s1->anatomicalMarkers->center3d_0.size() < s2->anatomicalMarkers->center3d_0.size()) ? s1->anatomicalMarkers->center3d_0.size() : s2->anatomicalMarkers->center3d_0.size();
    for( int i = 0; i < size; i++)
    {
        if( s1->anatomicalMarkers->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->anatomicalMarkers->center3d_0[i] != cv::Point3f (-1,-1,-1))
        {
			std::cout << "Considering reconstructions of: Anatomical Marker no. " << i << std::endl;
            H += cv::Mat( s2->anatomicalMarkers->center3d_0[i] - s2c)*scalefactor * cv::Mat( s1->anatomicalMarkers->center3d_0[i] - s1c).t();
        }
    }

    cv::SVD svd(H, cv::SVD::FULL_UV);
    cv::Mat X = svd.vt.t() * svd.u.t();
	std::cout << "Done." << std::endl;
	cv::Point3f tmppoint2;
    //finally: adding adapted points of second pointcloud to first pointcloud
	std::cout << "Adding adapted points of second pointcloud to first pointcloud." << std::endl;
    for( int i = 0; i < s2->stripe8_0->center3d_0.size(); i++)
    {
        cv::Mat tmpmat = X * cv::Mat(s2->stripe8_0->center3d_0[i] - s2c)*scalefactor;
        cv::Point3f tmppoint;
        tmppoint.x = tmpmat.at<float>(0,0);
        tmppoint.y = tmpmat.at<float>(1,0);
        tmppoint.z = tmpmat.at<float>(2,0);
        if( i < s1->stripe8_0->center3d_0.size())
        {
            if( s1->stripe8_0->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_0->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
            tmppoint2=tmppoint + s1c;
        	std::cout << "Dist. [mm] coords from different image pairs (stripe8_0):" << sqrt( (tmppoint2.x - s1->stripe8_0->center3d_0[i].x) * (tmppoint2.x - s1->stripe8_0->center3d_0[i].x) + (tmppoint2.y - s1->stripe8_0->center3d_0[i].y) * (tmppoint2.y - s1->stripe8_0->center3d_0[i].y) + (tmppoint2.z - s1->stripe8_0->center3d_0[i].z) * (tmppoint2.z - s1->stripe8_0->center3d_0[i].z) ) << std::endl; 
        	}
            if( s1->stripe8_0->center3d_0[i] == cv::Point3f (-1,-1,-1) && s2->stripe8_0->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
                d->stripe8_0->center3d_0[i] = tmppoint + s1c;
                d->stripe8_0->center2d_0[i] = cv::Point2f(-1,-1);
            }
        }
        else
        {
            d->stripe8_0->center3d_0.push_back(tmppoint + s1c);
            d->stripe8_0->center3d_1.push_back(cv::Point3f(-1,-1,-1));
            d->stripe8_0->center2d_0.push_back(cv::Point2f(-1,-1));
            d->stripe8_0->center2d_1.push_back(cv::Point2f(-1,-1));
        }
    }

    for( int i = 0; i < s2->stripe8_1->center3d_0.size(); i++)
    {
        cv::Mat tmpmat = X * cv::Mat(s2->stripe8_1->center3d_0[i] - s2c)*scalefactor;
        cv::Point3f tmppoint;
        tmppoint.x = tmpmat.at<float>(0,0);
        tmppoint.y = tmpmat.at<float>(1,0);
        tmppoint.z = tmpmat.at<float>(2,0);
        if( i < s1->stripe8_1->center3d_0.size())
        {
            if( s1->stripe8_1->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_1->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
            tmppoint2=tmppoint + s1c;
        	std::cout << "Dist. [mm] coords from different image pairs (stripe8_1):" << sqrt( (tmppoint2.x - s1->stripe8_1->center3d_0[i].x) * (tmppoint2.x - s1->stripe8_1->center3d_0[i].x) + (tmppoint2.y - s1->stripe8_1->center3d_0[i].y) * (tmppoint2.y - s1->stripe8_1->center3d_0[i].y) + (tmppoint2.z - s1->stripe8_1->center3d_0[i].z) * (tmppoint2.z - s1->stripe8_1->center3d_0[i].z) ) << std::endl;
        	}
            if( s1->stripe8_1->center3d_0[i] == cv::Point3f (-1,-1,-1) && s2->stripe8_1->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
                d->stripe8_1->center3d_0[i] = tmppoint + s1c;
                d->stripe8_1->center2d_0[i] = cv::Point2f(-1,-1);
            }
        }
        else
        {
            d->stripe8_1->center3d_0.push_back(tmppoint + s1c);
            d->stripe8_1->center3d_1.push_back(cv::Point3f(-1,-1,-1));
            d->stripe8_1->center2d_0.push_back(cv::Point2f(-1,-1));
            d->stripe8_1->center2d_1.push_back(cv::Point2f(-1,-1));
        }
    }

    for( int i = 0; i < s2->stripe8_2->center3d_0.size(); i++)
    {
        cv::Mat tmpmat = X * cv::Mat(s2->stripe8_2->center3d_0[i] - s2c)*scalefactor;
        cv::Point3f tmppoint;
        tmppoint.x = tmpmat.at<float>(0,0);
        tmppoint.y = tmpmat.at<float>(1,0);
        tmppoint.z = tmpmat.at<float>(2,0);
        if( i < s1->stripe8_2->center3d_0.size())
        {
            if( s1->stripe8_2->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_2->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
            tmppoint2=tmppoint + s1c;
        	std::cout << "Dist. [mm] coords from different image pairs (stripe8_2):" << sqrt( (tmppoint2.x - s1->stripe8_2->center3d_0[i].x) * (tmppoint2.x - s1->stripe8_2->center3d_0[i].x) + (tmppoint2.y - s1->stripe8_2->center3d_0[i].y) * (tmppoint2.y - s1->stripe8_2->center3d_0[i].y) + (tmppoint2.z - s1->stripe8_2->center3d_0[i].z) * (tmppoint2.z - s1->stripe8_2->center3d_0[i].z) ) << std::endl; 
        	}
            if( s1->stripe8_2->center3d_0[i] == cv::Point3f (-1,-1,-1) && s2->stripe8_2->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
                d->stripe8_2->center3d_0[i] = tmppoint + s1c;
                d->stripe8_2->center2d_0[i] = cv::Point2f(-1,-1);
            }
        }
        else
        {
            d->stripe8_2->center3d_0.push_back(tmppoint + s1c);
            d->stripe8_2->center3d_1.push_back(cv::Point3f(-1,-1,-1));
            d->stripe8_2->center2d_0.push_back(cv::Point2f(-1,-1));
            d->stripe8_2->center2d_1.push_back(cv::Point2f(-1,-1));
        }
    }

    for( int i = 0; i < s2->stripe8_3->center3d_0.size(); i++)
    {
        cv::Mat tmpmat = X * cv::Mat(s2->stripe8_3->center3d_0[i] - s2c)*scalefactor;
        cv::Point3f tmppoint;
        tmppoint.x = tmpmat.at<float>(0,0);
        tmppoint.y = tmpmat.at<float>(1,0);
        tmppoint.z = tmpmat.at<float>(2,0);
        if( i < s1->stripe8_3->center3d_0.size())
        {
            if( s1->stripe8_3->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe8_3->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
            tmppoint2=tmppoint + s1c;
        	std::cout << "Dist. [mm] coords from different image pairs (stripe8_3):" << sqrt( (tmppoint2.x - s1->stripe8_3->center3d_0[i].x) * (tmppoint2.x - s1->stripe8_3->center3d_0[i].x) + (tmppoint2.y - s1->stripe8_3->center3d_0[i].y) * (tmppoint2.y - s1->stripe8_3->center3d_0[i].y) + (tmppoint2.z - s1->stripe8_3->center3d_0[i].z) * (tmppoint2.z - s1->stripe8_3->center3d_0[i].z) ) << std::endl; 
        	}
            if( s1->stripe8_3->center3d_0[i] == cv::Point3f (-1,-1,-1) && s2->stripe8_3->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
                d->stripe8_3->center3d_0[i] = tmppoint + s1c;
                d->stripe8_3->center2d_0[i] = cv::Point2f(-1,-1);
            }
        }
        else
        {
            d->stripe8_3->center3d_0.push_back(tmppoint + s1c);
            d->stripe8_3->center3d_1.push_back(cv::Point3f(-1,-1,-1));
            d->stripe8_3->center2d_0.push_back(cv::Point2f(-1,-1));
            d->stripe8_3->center2d_1.push_back(cv::Point2f(-1,-1));
        }
    }

    for( int i = 0; i < s2->stripe12_0->center3d_0.size(); i++)
    {
        cv::Mat tmpmat = X * cv::Mat(s2->stripe12_0->center3d_0[i] - s2c)*scalefactor;
        cv::Point3f tmppoint;
        tmppoint.x = tmpmat.at<float>(0,0);
        tmppoint.y = tmpmat.at<float>(1,0);
        tmppoint.z = tmpmat.at<float>(2,0);
        if( i < s1->stripe12_0->center3d_0.size())
        {
            if( s1->stripe12_0->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_0->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
            tmppoint2=tmppoint + s1c;
        	std::cout << "Dist. [mm] coords from different image pairs (stripe12_0):" << sqrt( (tmppoint2.x - s1->stripe12_0->center3d_0[i].x) * (tmppoint2.x - s1->stripe12_0->center3d_0[i].x) + (tmppoint2.y - s1->stripe12_0->center3d_0[i].y) * (tmppoint2.y - s1->stripe12_0->center3d_0[i].y) + (tmppoint2.z - s1->stripe12_0->center3d_0[i].z) * (tmppoint2.z - s1->stripe12_0->center3d_0[i].z) ) << std::endl; 
        	}
            if( s1->stripe12_0->center3d_0[i] == cv::Point3f (-1,-1,-1) && s2->stripe12_0->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
                d->stripe12_0->center3d_0[i] = tmppoint + s1c;
                d->stripe12_0->center2d_0[i] = cv::Point2f(-1,-1);
            }
        }
        else
        {
            d->stripe12_0->center3d_0.push_back(tmppoint + s1c);
            d->stripe12_0->center3d_1.push_back(cv::Point3f(-1,-1,-1));
            d->stripe12_0->center2d_0.push_back(cv::Point2f(-1,-1));
            d->stripe12_0->center2d_1.push_back(cv::Point2f(-1,-1));
        }
    }

    for( int i = 0; i < s2->stripe12_1->center3d_0.size(); i++)
    {
        cv::Mat tmpmat = X * cv::Mat(s2->stripe12_1->center3d_0[i] - s2c)*scalefactor;
        cv::Point3f tmppoint;
        tmppoint.x = tmpmat.at<float>(0,0);
        tmppoint.y = tmpmat.at<float>(1,0);
        tmppoint.z = tmpmat.at<float>(2,0);
        if( i < s1->stripe12_1->center3d_0.size())
        {
            if( s1->stripe12_1->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_1->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
            tmppoint2=tmppoint + s1c;
        	std::cout << "Dist. [mm] coords from different image pairs (stripe12_1):" << sqrt( (tmppoint2.x - s1->stripe12_1->center3d_0[i].x) * (tmppoint2.x - s1->stripe12_1->center3d_0[i].x) + (tmppoint2.y - s1->stripe12_1->center3d_0[i].y) * (tmppoint2.y - s1->stripe12_1->center3d_0[i].y) + (tmppoint2.z - s1->stripe12_1->center3d_0[i].z) * (tmppoint2.z - s1->stripe12_1->center3d_0[i].z) ) << std::endl; 
        	}
            if( s1->stripe12_1->center3d_0[i] == cv::Point3f (-1,-1,-1) && s2->stripe12_1->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
                d->stripe12_1->center3d_0[i] = tmppoint + s1c;
                d->stripe12_1->center2d_0[i] = cv::Point2f(-1,-1);
            }
        }
        else
        {
            d->stripe12_1->center3d_0.push_back(tmppoint + s1c);
            d->stripe12_1->center3d_1.push_back(cv::Point3f(-1,-1,-1));
            d->stripe12_1->center2d_0.push_back(cv::Point2f(-1,-1));
            d->stripe12_1->center2d_1.push_back(cv::Point2f(-1,-1));
        }
    }

    for( int i = 0; i < s2->stripe12_2->center3d_0.size(); i++)
    {
        cv::Mat tmpmat = X * cv::Mat(s2->stripe12_2->center3d_0[i] - s2c)*scalefactor;
        cv::Point3f tmppoint;
        tmppoint.x = tmpmat.at<float>(0,0);
        tmppoint.y = tmpmat.at<float>(1,0);
        tmppoint.z = tmpmat.at<float>(2,0);
        if( i < s1->stripe12_2->center3d_0.size())
        {
            if( s1->stripe12_2->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_2->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
            tmppoint2=tmppoint + s1c;
        	std::cout << "Dist. [mm] coords from different image pairs (stripe12_2):" << sqrt( (tmppoint2.x - s1->stripe12_2->center3d_0[i].x) * (tmppoint2.x - s1->stripe12_2->center3d_0[i].x) + (tmppoint2.y - s1->stripe12_2->center3d_0[i].y) * (tmppoint2.y - s1->stripe12_2->center3d_0[i].y) + (tmppoint2.z - s1->stripe12_2->center3d_0[i].z) * (tmppoint2.z - s1->stripe12_2->center3d_0[i].z) ) << std::endl; 
        	}
            if( s1->stripe12_2->center3d_0[i] == cv::Point3f (-1,-1,-1) && s2->stripe12_2->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
                d->stripe12_2->center3d_0[i] = tmppoint + s1c;
                d->stripe12_2->center2d_0[i] = cv::Point2f(-1,-1);
            }
        }
        else
        {
            d->stripe12_2->center3d_0.push_back(tmppoint + s1c);
            d->stripe12_2->center3d_1.push_back(cv::Point3f(-1,-1,-1));
            d->stripe12_2->center2d_0.push_back(cv::Point2f(-1,-1));
            d->stripe12_2->center2d_1.push_back(cv::Point2f(-1,-1));
        }
    }

    for( int i = 0; i < s2->stripe12_3->center3d_0.size(); i++)
    {
        cv::Mat tmpmat = X * cv::Mat(s2->stripe12_3->center3d_0[i] - s2c)*scalefactor;
        cv::Point3f tmppoint;
        tmppoint.x = tmpmat.at<float>(0,0);
        tmppoint.y = tmpmat.at<float>(1,0);
        tmppoint.z = tmpmat.at<float>(2,0);
        if( i < s1->stripe12_3->center3d_0.size())
        {
            if( s1->stripe12_3->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->stripe12_3->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
            tmppoint2=tmppoint + s1c;
        	std::cout << "Dist. [mm] coords from different image pairs (stripe12_3):" << sqrt( (tmppoint2.x - s1->stripe12_3->center3d_0[i].x) * (tmppoint2.x - s1->stripe12_3->center3d_0[i].x) + (tmppoint2.y - s1->stripe12_3->center3d_0[i].y) * (tmppoint2.y - s1->stripe12_3->center3d_0[i].y) + (tmppoint2.z - s1->stripe12_3->center3d_0[i].z) * (tmppoint2.z - s1->stripe12_3->center3d_0[i].z) ) << std::endl; 
        	}
            if( s1->stripe12_3->center3d_0[i] == cv::Point3f (-1,-1,-1) && s2->stripe12_3->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
                d->stripe12_3->center3d_0[i] = tmppoint + s1c;
                d->stripe12_3->center2d_0[i] = cv::Point2f(-1,-1);
            }
        }
        else
        {
            d->stripe12_3->center3d_0.push_back(tmppoint + s1c);
            d->stripe12_3->center3d_1.push_back(cv::Point3f(-1,-1,-1));
            d->stripe12_3->center2d_0.push_back(cv::Point2f(-1,-1));
            d->stripe12_3->center2d_1.push_back(cv::Point2f(-1,-1));
        }
    }
    
    for( int i = 0; i < s2->anatomicalMarkers->center3d_0.size(); i++)
    {
        cv::Mat tmpmat = X * cv::Mat(s2->anatomicalMarkers->center3d_0[i] - s2c)*scalefactor;
        cv::Point3f tmppoint;
        tmppoint.x = tmpmat.at<float>(0,0);
        tmppoint.y = tmpmat.at<float>(1,0);
        tmppoint.z = tmpmat.at<float>(2,0);
        if( i < s1->anatomicalMarkers->center3d_0.size())
        {
            if( s1->anatomicalMarkers->center3d_0[i] != cv::Point3f (-1,-1,-1) && s2->anatomicalMarkers->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
            tmppoint2=tmppoint + s1c;
        	std::cout << "Dist. [mm] coords from different image pairs (anatomicalMarkers):" << sqrt( (tmppoint2.x - s1->anatomicalMarkers->center3d_0[i].x) * (tmppoint2.x - s1->anatomicalMarkers->center3d_0[i].x) + (tmppoint2.y - s1->anatomicalMarkers->center3d_0[i].y) * (tmppoint2.y - s1->anatomicalMarkers->center3d_0[i].y) + (tmppoint2.z - s1->anatomicalMarkers->center3d_0[i].z) * (tmppoint2.z - s1->anatomicalMarkers->center3d_0[i].z) ) << std::endl; 
        	}
            if( s1->anatomicalMarkers->center3d_0[i] == cv::Point3f (-1,-1,-1) && s2->anatomicalMarkers->center3d_0[i] != cv::Point3f (-1,-1,-1))
            {
                d->anatomicalMarkers->center3d_0[i] = tmppoint + s1c;
                d->anatomicalMarkers->center2d_0[i] = cv::Point2f(-1,-1);
            }
        }
        else
        {
            d->anatomicalMarkers->center3d_0.push_back(tmppoint + s1c);
            d->anatomicalMarkers->center3d_1.push_back(cv::Point3f(-1,-1,-1));
            d->anatomicalMarkers->center2d_0.push_back(cv::Point2f(-1,-1));
            d->anatomicalMarkers->center2d_1.push_back(cv::Point2f(-1,-1));
        }
    }
}

void scale(TwoView* s) //finding relation between pixel size and metric size by analysing the distance of specific aruco markers
                       //distance is calculated for ids: 0 - 80, 0 - 160, 80 - 240, 160 - 240
                       //average distance is used
                       //working pointcloud is scaled by found factor
{	//Distances between markers: 
	//d(0-80) = d(160-240) = 168.25mm
	//d(0-160) = d(80-240) = 84.5mm
	//average when using all four: 126.375mm
	//average between 0-80 and 160-240 should be 200mm, but is 168.25mm DUE TO PRINTING ERROR IN USED CARDBOARD (scaling error) during pdf conversion (see markerpdf)
    double markerdistance = 84.5; //mm AVERAGE DISTANCE BETWEEN MARKERS
	int usem12 = 0;//INSTRUCT TO SAMPLE DISTANCE BETWEEN m1 and m2
	int usem13 = 1;//INSTRUCT TO SAMPLE DISTANCE BETWEEN m1 and m3
	int usem24 = 0;//INSTRUCT TO SAMPLE DISTANCE BETWEEN m2 and m4
	int usem34 = 0;//INSTRUCT TO SAMPLE DISTANCE BETWEEN m3 and m4
	
    double distance = 0;
    int distancenumber = 0;
    int m1 = -1;
    int m2 = -1;
    int m3 = -1;
    int m4 = -1;

    for( int i=0; i < s->fundamentalMatches->center3d_0.size(); i++)
    {
        if(s->fundamentalMatches->id[i] == 0)
        {
            m1 = i;
        }
        else if(s->fundamentalMatches->id[i] == 80)
        {
            m2 = i;
        }
        else if(s->fundamentalMatches->id[i] == 160)
        {
            m3 = i;
        }
        else if(s->fundamentalMatches->id[i] == 240)
        {
            m4 = i;
        }
    }

    if( m1 != -1 && m2 != -1 && usem12 == 1)
    {
        double distancesquared = 0;
        distancesquared += (s->fundamentalMatches->center3d_0[m1].x - s->fundamentalMatches->center3d_0[m2].x) * (s->fundamentalMatches->center3d_0[m1].x - s->fundamentalMatches->center3d_0[m2].x);
        distancesquared += (s->fundamentalMatches->center3d_0[m1].y - s->fundamentalMatches->center3d_0[m2].y) * (s->fundamentalMatches->center3d_0[m1].y - s->fundamentalMatches->center3d_0[m2].y);
        distancesquared += (s->fundamentalMatches->center3d_0[m1].z - s->fundamentalMatches->center3d_0[m2].z) * (s->fundamentalMatches->center3d_0[m1].z - s->fundamentalMatches->center3d_0[m2].z);
        distancenumber++;
        distance += sqrt(distancesquared);
    }
    if( m1 != -1 && m3 != -1 && usem13 == 1)
    {
        double distancesquared = 0;
        distancesquared += (s->fundamentalMatches->center3d_0[m1].x - s->fundamentalMatches->center3d_0[m3].x) * (s->fundamentalMatches->center3d_0[m1].x - s->fundamentalMatches->center3d_0[m3].x);
        distancesquared += (s->fundamentalMatches->center3d_0[m1].y - s->fundamentalMatches->center3d_0[m3].y) * (s->fundamentalMatches->center3d_0[m1].y - s->fundamentalMatches->center3d_0[m3].y);
        distancesquared += (s->fundamentalMatches->center3d_0[m1].z - s->fundamentalMatches->center3d_0[m3].z) * (s->fundamentalMatches->center3d_0[m1].z - s->fundamentalMatches->center3d_0[m3].z);
        distancenumber++;
        distance += sqrt(distancesquared);
    }
    if( m2 != -1 && m4 != -1 && usem24 == 1)
    {
        double distancesquared = 0;
        distancesquared += (s->fundamentalMatches->center3d_0[m2].x - s->fundamentalMatches->center3d_0[m4].x) * (s->fundamentalMatches->center3d_0[m2].x - s->fundamentalMatches->center3d_0[m4].x);
        distancesquared += (s->fundamentalMatches->center3d_0[m2].y - s->fundamentalMatches->center3d_0[m4].y) * (s->fundamentalMatches->center3d_0[m2].y - s->fundamentalMatches->center3d_0[m4].y);
        distancesquared += (s->fundamentalMatches->center3d_0[m2].z - s->fundamentalMatches->center3d_0[m4].z) * (s->fundamentalMatches->center3d_0[m2].z - s->fundamentalMatches->center3d_0[m4].z);
        distancenumber++;
        distance += sqrt(distancesquared);
    }
    if( m3 != -1 && m4 != -1 && usem34 == 1)
    {
        double distancesquared = 0;
        distancesquared += (s->fundamentalMatches->center3d_0[m3].x - s->fundamentalMatches->center3d_0[m4].x) * (s->fundamentalMatches->center3d_0[m3].x - s->fundamentalMatches->center3d_0[m4].x);
        distancesquared += (s->fundamentalMatches->center3d_0[m3].y - s->fundamentalMatches->center3d_0[m4].y) * (s->fundamentalMatches->center3d_0[m3].y - s->fundamentalMatches->center3d_0[m4].y);
        distancesquared += (s->fundamentalMatches->center3d_0[m3].z - s->fundamentalMatches->center3d_0[m4].z) * (s->fundamentalMatches->center3d_0[m3].z - s->fundamentalMatches->center3d_0[m4].z);
        distancenumber++;
        distance += sqrt(distancesquared);
    }

    if( distancenumber != 0)
    {
        distance /= distancenumber;
        double scalefactor = markerdistance / distance;

        for( int i=0; i < s->stripe8_0->center3d_0.size(); i++)
        {
            if(s->stripe8_0->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->stripe8_0->center3d_0[i].x *= scalefactor;
                s->stripe8_0->center3d_0[i].y *= scalefactor;
                s->stripe8_0->center3d_0[i].z *= scalefactor;
            }
        }
        for( int i=0; i < s->stripe8_1->center3d_0.size(); i++)
        {
            if(s->stripe8_1->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->stripe8_1->center3d_0[i].x *= scalefactor;
                s->stripe8_1->center3d_0[i].y *= scalefactor;
                s->stripe8_1->center3d_0[i].z *= scalefactor;
            }
        }
        for( int i=0; i < s->stripe8_2->center3d_0.size(); i++)
        {
            if(s->stripe8_2->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->stripe8_2->center3d_0[i].x *= scalefactor;
                s->stripe8_2->center3d_0[i].y *= scalefactor;
                s->stripe8_2->center3d_0[i].z *= scalefactor;
            }
        }
        for( int i=0; i < s->stripe8_3->center3d_0.size(); i++)
        {
            if(s->stripe8_3->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->stripe8_3->center3d_0[i].x *= scalefactor;
                s->stripe8_3->center3d_0[i].y *= scalefactor;
                s->stripe8_3->center3d_0[i].z *= scalefactor;
            }
        }

        for( int i=0; i < s->stripe12_0->center3d_0.size(); i++)
        {
            if(s->stripe12_0->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->stripe12_0->center3d_0[i].x *= scalefactor;
                s->stripe12_0->center3d_0[i].y *= scalefactor;
                s->stripe12_0->center3d_0[i].z *= scalefactor;
            }
        }
        for( int i=0; i < s->stripe12_1->center3d_0.size(); i++)
        {
            if(s->stripe12_1->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->stripe12_1->center3d_0[i].x *= scalefactor;
                s->stripe12_1->center3d_0[i].y *= scalefactor;
                s->stripe12_1->center3d_0[i].z *= scalefactor;
            }
        }
        for( int i=0; i < s->stripe12_2->center3d_0.size(); i++)
        {
            if(s->stripe12_2->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->stripe12_2->center3d_0[i].x *= scalefactor;
                s->stripe12_2->center3d_0[i].y *= scalefactor;
                s->stripe12_2->center3d_0[i].z *= scalefactor;
            }
        }
        for( int i=0; i < s->stripe12_3->center3d_0.size(); i++)
        {
            if(s->stripe12_3->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->stripe12_3->center3d_0[i].x *= scalefactor;
                s->stripe12_3->center3d_0[i].y *= scalefactor;
                s->stripe12_3->center3d_0[i].z *= scalefactor;
            }
        }
        for( int i=0; i < s->anatomicalMarkers->center3d_0.size(); i++)
        {
            if(s->anatomicalMarkers->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->anatomicalMarkers->center3d_0[i].x *= scalefactor;
                s->anatomicalMarkers->center3d_0[i].y *= scalefactor;
                s->anatomicalMarkers->center3d_0[i].z *= scalefactor;
            }
        }
        for( int i=0; i < s->fundamentalMatches->center3d_0.size(); i++)
        {
            if(s->fundamentalMatches->center3d_0[i] != cv::Point3f(-1,-1,-1))
            {
                s->fundamentalMatches->center3d_0[i].x *= scalefactor;
                s->fundamentalMatches->center3d_0[i].y *= scalefactor;
                s->fundamentalMatches->center3d_0[i].z *= scalefactor;
            }
        }
    }
}
