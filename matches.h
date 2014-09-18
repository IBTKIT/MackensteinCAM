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

#ifndef POINTS_H
#define POINTS_H

#include <opencv2/core/core.hpp>

struct match
{
	std::string name;
    std::vector<cv::Point2f> center2d_0;
    std::vector<cv::Point2f> center2d_1;
    std::vector<cv::Point3f> center3d_0;
    std::vector<cv::Point3f> center3d_1;
    std::vector<int> id;
    std::vector<std::string> description;

    void clear()
    {
        center2d_0.clear();
        center2d_1.clear();
        center3d_0.clear();
        center3d_1.clear();
        id.clear();
        description.clear();
    }

    int validsize()
    {
        int ctr = 0;
        for(int i=0; i < center2d_0.size();i++)
        {
            if( center2d_0[i].x >= 0)
            {
                ctr++;
            }
        }
        return ctr;
    }
};

#endif // POINTS_H
