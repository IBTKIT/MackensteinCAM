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

#include "findelectrodes.h"

findelectrodes::findelectrodes(const cv::Mat &image, int orientation)
{
   //img = image;
   cv::cvtColor(image, img, CV_BGR2GRAY); //working image: gray colorspace
   ori = orientation;
}

findelectrodes::~findelectrodes()
{
    thresh.release();
}

void findelectrodes::find()
{
    thresh = img.clone();

    //tresholding image by using adaptive thresholding, 203 is the matrix size and has to be adapted if thresholding fails
    cv::adaptiveThreshold(img,thresh,255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,203,0);

    //searching for contours in thresholded image, building hierachy of contours (outercontour -> innercontours)
    cv::findContours( thresh.clone() , contours, hierarchy,CV_RETR_TREE, CV_CHAIN_APPROX_NONE );

    //checking every found contour...
    for( int i = 0; i< contours.size(); i++ )
    {
        //counting innercountours from the left and from the right side
        int innercontours = -1;
        for(int next = hierarchy[i][2]; next >= 0; next = hierarchy[next][0])
        {
            innercontours++;
        }
        for(int prev = hierarchy[i][2]; prev >= 0; prev = hierarchy[prev][1])
        {
            innercontours++;
        }

        //checking if contour matches expected perimeter range and expected innercontour size, has to be adapted if image properties are changed extremely
        if( cv::arcLength(contours[i], true) > img.cols/4 && cv::arcLength(contours[i], true) < 30*img.cols &&
            innercontours >= 7 && innercontours <= 24)
        {
            //checking for image contours which are large enough to be considered as an electrode marker
            int largeinnercontours = 0;
            for(int next = hierarchy[i][2]; next >= 0; next = hierarchy[next][0])
            {
                if( (cv::contourArea( contours[next], false) >=  cv::contourArea( contours[i], false)/200) &&
                        (cv::contourArea( contours[next], false) <=  cv::contourArea( contours[i], false)/20) )
                {
                    largeinnercontours++;
                }
            }
            for(int prev = hierarchy[ hierarchy[i][2] ][1]; prev >= 0; prev = hierarchy[prev][1])
            {
                if( (cv::contourArea( contours[prev], false) >=  cv::contourArea( contours[i], false)/200) &&
                        (cv::contourArea( contours[prev], false) <=  cv::contourArea( contours[i], false)/20) )
                {
                    largeinnercontours++;
                }
            }

            if(largeinnercontours >= 7 && largeinnercontours <= 24)
            {
                //saving contour id in list for further analysis
                std::cout << "Contour-Id: " << i << " Innercontours:" << innercontours << " Largeinnercontours:" << largeinnercontours << std::endl;
                stripes.push_back(i);
            }
        }
    }

    for(int i = 0; i < stripes.size(); i++) //analysing every found stripe
    {
        //checking for sizes of inner countous, if size is in the range of electrode marker, calculate centroid of marker
        cv::Moments moment;
        std::vector<cv::Point2f> centroids;
        std::vector<double> centroidPerimeter;
        std::vector<int> centroidContour;
        for(int next = hierarchy[stripes[i]][2]; next >= 0; next = hierarchy[next][0])
        {
            if( (cv::contourArea( contours[next], false) >=  cv::contourArea( contours[stripes[i]], false)/300) &&
                    (cv::contourArea( contours[next], false) <=  cv::contourArea( contours[stripes[i]], false)/20) )
            {
                moment = cv::moments(contours[next], false);
                centroids.push_back( cv::Point2f(moment.m10/moment.m00 ,moment.m01/moment.m00));
                centroidPerimeter.push_back( cv::arcLength( contours[next], true));
                centroidContour.push_back( next);
            }
        }
        for(int prev = hierarchy[ hierarchy[stripes[i]][2] ][1]; prev >= 0; prev = hierarchy[prev][1])
        {
            if( (cv::contourArea( contours[prev], false) >=  cv::contourArea( contours[stripes[i]], false)/300) &&
                    (cv::contourArea( contours[prev], false) <=  cv::contourArea( contours[stripes[i]], false)/20) )
            {
                moment = cv::moments(contours[prev], false);
                centroids.push_back( cv::Point2f(moment.m10/moment.m00 ,moment.m01/moment.m00));
                centroidPerimeter.push_back( cv::arcLength( contours[prev], true));
                centroidContour.push_back( prev);
            }
        }

        //if two centroids are too close, thresholding for marker might be wrong and both centroids have to be joined
        for( int j=0; j < centroids.size(); j++)
        {
            for( int k = j+1; k < centroids.size(); k++)
            {
                double dist = sqrt( (centroids[j].x - centroids[k].x)*(centroids[j].x - centroids[k].x) + (centroids[j].y - centroids[k].y)*(centroids[j].y - centroids[k].y) );
                double averagePerimeter = ( cv::arcLength( contours[centroidContour[j]], true) +  cv::arcLength( contours[centroidContour[k]], true))/2;

                    if( dist <= averagePerimeter/2 //&& additional requirement removed: after long thinking, does not make sense why rectangular markers should not be merged and rhombic markers not
                        //cv::minAreaRect(contours[centroidContour[j]]).size.area() <= 1.7 *  cv::contourArea(contours[centroidContour[j]], false) &&
                        //cv::minAreaRect(contours[centroidContour[k]]).size.area() <= 1.7 *  cv::contourArea(contours[centroidContour[k]], false)  
			)
                    {
                        centroids[j].x = (centroids[j].x + centroids[k].x)/2;
                        centroids[j].y = (centroids[j].y + centroids[k].y)/2;
                        centroidPerimeter[j] += centroidPerimeter[k];
                        centroids[k].x = -1;
                        centroids[k].y = -1;
                    }
            }
        }

        //delete second marker of two joined markers (joined marker is written to first marker position)
        for( int j=0; j < centroids.size(); j++)
        {
            if( centroids[j].x == -1 )
            {
                centroids.erase(centroids.begin()+j);
                centroidPerimeter.erase(centroidPerimeter.begin()+j);
                j--;
            }
        }

        sortCentroids(centroids); //sort found centroids by x or y coordinates

        std::cout << centroids << std::endl;

        std::cout << "\nPotentieller Streifen: ";

        if(centroids.size() <= 12 && centroids.size() > 0) //stripe size is not allowed to be bigger than 12 or smaller than 1
        {
            //getting subpixel information for found centroids
            cv::cornerSubPix(img, centroids, cvSize(4,4), cvSize(1,1) ,cvTermCriteria ( CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,100,0.001 ));

            std::vector<int> id;

            for( int j=0; j < centroids.size(); j++)
            {
                //for every centroid: calculating connection vector of actual and next centroid
                //scaling the vector to the half of the diagonal length of one of the four rectangles which are belonging to an elctrode marker
                //rotating the vector in four directions which are roughly pointing to the diagonal of each reactangle
                //checking for colors of calculated points for getting bit of electrode marker
                //adding bit to id vector: '0','1' are set for bit values, '-2' is set for errors (bit not decoded)
                cv::Point2f direction;
                cv::Point2f rot1;
                cv::Point2f rot2;
                cv::Point2f rot3;
                cv::Point2f rot4;
                if( j!= (centroids.size()-1) )
                {
                    direction.x = centroids[j+1].x - centroids[j].x;
                    direction.y = centroids[j+1].y - centroids[j].y;
                }
                else
                {
                    direction.x = centroids[j].x - centroids[j-1].x;
                    direction.y = centroids[j].y - centroids[j-1].y;
                }
                double length = sqrt( direction.x * direction.x + direction.y * direction.y );
                direction.x = 0.09 * centroidPerimeter[j] * direction.x/length;
                direction.y = 0.09 * centroidPerimeter[j] * direction.y/length;
                rot1.x = ( 0.94 * direction.x - 0.34 * direction.y ); //20°
                rot1.y = ( 0.34 * direction.x + 0.94 * direction.y );
                rot2.x = ( -0.94 * direction.x - 0.34 * direction.y );
                rot2.y = ( 0.34 * direction.x - 0.94 * direction.y );
                rot3.x = ( -0.94 * direction.x + 0.34 * direction.y );
                rot3.y = ( -0.34 * direction.x - 0.94 * direction.y );
                rot4.x = ( 0.94 * direction.x + 0.34 * direction.y );
                rot4.y = ( -0.34 * direction.x + 0.94 * direction.y );

                if( thresh.at<uchar>(centroids[j].y + rot1.y, centroids[j].x + rot1.x) == 255 &&
                        thresh.at<uchar>(centroids[j].y + rot2.y, centroids[j].x + rot2.x) == 0 &&
                        thresh.at<uchar>(centroids[j].y + rot3.y, centroids[j].x + rot3.x) == 255 &&
                        thresh.at<uchar>(centroids[j].y + rot4.y, centroids[j].x + rot4.x) == 0 )
                {
                    id.push_back(1);
                    std::cout << 1;
                }
                else if( thresh.at<uchar>(centroids[j].y + rot1.y, centroids[j].x + rot1.x) == 0 &&
                         thresh.at<uchar>(centroids[j].y + rot2.y, centroids[j].x + rot2.x) == 255 &&
                         thresh.at<uchar>(centroids[j].y + rot3.y, centroids[j].x + rot3.x) == 0 &&
                         thresh.at<uchar>(centroids[j].y + rot4.y, centroids[j].x + rot4.x) == 255 )
                {
                    id.push_back(0);
                    std::cout << 0;
                }
                else if( thresh.at<uchar>(centroids[j].y + 0.8 * rot1.y, centroids[j].x + 0.8 * rot1.x) == 255 &&
                         thresh.at<uchar>(centroids[j].y + 0.8 *rot2.y, centroids[j].x + 0.8 * rot2.x) == 0 &&
                         thresh.at<uchar>(centroids[j].y + 0.8 *rot3.y, centroids[j].x + 0.8 * rot3.x) == 255 &&
                         thresh.at<uchar>(centroids[j].y + 0.8 *rot4.y, centroids[j].x + 0.8 * rot4.x) == 0 )
                {
                    id.push_back(1);
                    std::cout << 1;
                }
                else if( thresh.at<uchar>(centroids[j].y + 0.8 *rot1.y, centroids[j].x + 0.8 * rot1.x) == 0 &&
                         thresh.at<uchar>(centroids[j].y + 0.8 *rot2.y, centroids[j].x + 0.8 * rot2.x) == 255 &&
                         thresh.at<uchar>(centroids[j].y + 0.8 *rot3.y, centroids[j].x + 0.8 * rot3.x) == 0 &&
                         thresh.at<uchar>(centroids[j].y + 0.8 *rot4.y, centroids[j].x + 0.8 * rot4.x) == 255 )
                {
                    id.push_back(0);
                    std::cout << 0;
                }
                else
                {
                    id.push_back(-2);
                    std::cout << -2;
                }

                /*cv::circle(thresh, centroids[j] + rot1 , 2, cv::Scalar(0,0,0), 1);
                cv::circle(thresh, centroids[j] + rot2 , 2, cv::Scalar(0,0,0), 1);
                cv::circle(thresh, centroids[j] + rot3 , 2, cv::Scalar(0,0,0), 1);
                cv::circle(thresh, centroids[j] + rot4 , 2, cv::Scalar(0,0,0), 1);*/
            }
            std::cout << std::endl << std::endl;

            checkid(id, centroids);
        }

        centroids.clear();
    }
}

void findelectrodes::sortCentroids(std::vector<cv::Point2f> &centroids)
{
    if( centroids.size() > 1)
    {
        for( int ctr = 0; ctr < centroids.size(); ctr++)
        {
            for( int j=0; j < centroids.size()-1; j++)
            {
                if( ori == R) //first point at the right side of the image: sort x coordinates
                {
                    if(centroids[j].x < centroids[j+1].x )
                        std::swap(centroids[j], centroids[j+1]);
                }
                else if( ori == L) //left side
                {
                    if(centroids[j].x > centroids[j+1].x )
                        std::swap(centroids[j], centroids[j+1]);
                }
                else if( ori == T) // top side
                {
                    if(centroids[j].y > centroids[j+1].y )
                        std::swap(centroids[j], centroids[j+1]);
                }
                else if( ori == B) //bottom side
                {
                    if(centroids[j].y < centroids[j+1].y )
                        std::swap(centroids[j], centroids[j+1]);
                }
            }
        }
    }
}

void findelectrodes::checkid(std::vector<int> id, std::vector<cv::Point2f> &centroids) //checking for valid id, using hamming code for error correction, see bachelor thesis for details
{
    int p8_0[8] = {0,0,0,0,0,0,0,0};  //id patterns
    int p8_1[8] = {0,0,0,0,1,1,1,1};
    int p8_2[8] = {1,1,1,1,0,0,0,0};
    int p8_3[8] = {1,1,1,1,1,1,1,1};
    int p12_0[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    int p12_1[12] = {0,0,0,0,0,0,1,1,1,1,1,1};
    int p12_2[12] = {1,1,1,1,1,1,0,0,0,0,0,0};
    int p12_3[12] = {1,1,1,1,1,1,1,1,1,1,1,1};

    if( id.size() == 8)
    {
        int hammdist0 = 0; //storing hamming distances for each stripe of size eight...
        int hammdist1 = 0;
        int hammdist2 = 0;
        int hammdist3 = 0;

        for(int i = 0; i < 8; i++) //calculaating hamming distances
        {
            if( id[i] != p8_0[i] ) hammdist0++;
        }
        for(int i = 0; i < 8; i++)
        {
            if( id[i] != p8_1[i] ) hammdist1++;
        }
        for(int i = 0; i < 8; i++)
        {
            if( id[i] != p8_2[i] ) hammdist2++;
        }
        for(int i = 0; i < 8; i++)
        {
            if( id[i] != p8_3[i] ) hammdist3++;
        }

        if( hammdist0 == 0 && stripe8_0.size() == 0 ) stripe8_0 = centroids;  //adding points if hamming distance are valid and stripe does not exist yet
        else if( hammdist1 == 0 && stripe8_1.size() == 0 ) stripe8_1 = centroids;
        else if( hammdist2 == 0 && stripe8_2.size() == 0 ) stripe8_2 = centroids;
        else if( hammdist3 == 0 && stripe8_3.size() == 0 ) stripe8_3 = centroids;
        else if( hammdist0 == 1 && stripe8_0.size() == 0 ) stripe8_0 = centroids;
        else if( hammdist1 == 1 && stripe8_1.size() == 0 ) stripe8_1 = centroids;
        else if( hammdist2 == 1 && stripe8_2.size() == 0 ) stripe8_2 = centroids;
        else if( hammdist3 == 1 && stripe8_3.size() == 0 ) stripe8_3 = centroids;
    }
    else if( id.size() == 12)
    {
        int hammdist0 = 0;
        int hammdist1 = 0;
        int hammdist2 = 0;
        int hammdist3 = 0;

        for(int i = 0; i < 12; i++)
        {
            if( id[i] != p12_0[i] ) hammdist0++;
        }
        for(int i = 0; i < 12; i++)
        {
            if( id[i] != p12_1[i] ) hammdist1++;
        }
        for(int i = 0; i < 12; i++)
        {
            if( id[i] != p12_2[i] ) hammdist2++;
        }
        for(int i = 0; i < 12; i++)
        {
            if( id[i] != p12_3[i] ) hammdist3++;
        }

        if( hammdist0 == 0 && stripe12_0.size() == 0 ) stripe12_0 = centroids;
        else if( hammdist1 == 0 && stripe12_1.size() == 0 ) stripe12_1 = centroids;
        else if( hammdist2 == 0 && stripe12_2.size() == 0 ) stripe12_2 = centroids;
        else if( hammdist3 == 0 && stripe12_3.size() == 0 ) stripe12_3 = centroids;
        else if( hammdist0 == 1 && stripe12_0.size() == 0 ) stripe12_0 = centroids;
        else if( hammdist1 == 1 && stripe12_1.size() == 0 ) stripe12_1 = centroids;
        else if( hammdist2 == 1 && stripe12_2.size() == 0 ) stripe12_2 = centroids;
        else if( hammdist3 == 1 && stripe12_3.size() == 0 ) stripe12_3 = centroids;
    }
    else if( id.size() == 7)
    {
        int hammdist0f = 0; //checking hammingdistance for missing first bit
        int hammdist1f = 0;
        int hammdist2f = 0;
        int hammdist3f = 0;
        int hammdist0b = 0; //checking hammingdistance for missing last bit
        int hammdist1b = 0;
        int hammdist2b = 0;
        int hammdist3b = 0;

        for(int i = 0; i < 7; i++)
        {
            if( id[i] != p8_0[i] ) hammdist0f++;
        }
        for(int i = 0; i < 7; i++)
        {
            if( id[i] != p8_1[i] ) hammdist1f++;
        }
        for(int i = 0; i < 7; i++)
        {
            if( id[i] != p8_2[i] ) hammdist2f++;
        }
        for(int i = 0; i < 7; i++)
        {
            if( id[i] != p8_3[i] ) hammdist3f++;
        }
        for(int i = 0; i < 7; i++)
        {
            if( id[i] != p8_0[i+1] ) hammdist0b++;
        }
        for(int i = 0; i < 7; i++)
        {
            if( id[i] != p8_1[i+1] ) hammdist1b++;
        }
        for(int i = 0; i < 7; i++)
        {
            if( id[i] != p8_2[i+1] ) hammdist2b++;
        }
        for(int i = 0; i < 7; i++)
        {
            if( id[i] != p8_3[i+1] ) hammdist3b++;
        }

        if( hammdist0f == 0 && stripe8_0.size() == 0 )
        {
            stripe8_0 = centroids;
            stripe8_0.push_back(cv::Point2f(-1,-1));
        }
        else if( hammdist1f == 0 && stripe8_1.size() == 0 )
        {
            stripe8_1 = centroids;
            stripe8_1.push_back(cv::Point2f(-1,-1));
        }
        else if( hammdist2f == 0 && stripe8_2.size() == 0 )
        {
            stripe8_2 = centroids;
            stripe8_2.push_back(cv::Point2f(-1,-1));
        }
        else if( hammdist3f == 0 && stripe8_3.size() == 0 )
        {
            stripe8_3 = centroids;
            stripe8_3.push_back(cv::Point2f(-1,-1));
        }

        else if( hammdist0b == 0 && stripe8_0.size() == 0 )
        {
            stripe8_0.push_back(cv::Point2f(-1,-1));
            stripe8_0.insert( stripe8_0.end(), centroids.begin(), centroids.end());
        }
        else if( hammdist1b == 0 && stripe8_1.size() == 0 )
        {
            stripe8_1.push_back(cv::Point2f(-1,-1));
            stripe8_1.insert( stripe8_1.end(), centroids.begin(), centroids.end());
        }
        else if( hammdist2b == 0 && stripe8_2.size() == 0 )
        {
            stripe8_2.push_back(cv::Point2f(-1,-1));
            stripe8_2.insert( stripe8_2.end(), centroids.begin(), centroids.end());
        }
        else if( hammdist3b == 0 && stripe8_3.size() == 0 )
        {
            stripe8_3.push_back(cv::Point2f(-1,-1));
            stripe8_3.insert( stripe8_3.end(), centroids.begin(), centroids.end());
        }
        
    }
    else if( id.size() == 11)
    {
        int hammdist0f = 0;
        int hammdist1f = 0;
        int hammdist2f = 0;
        int hammdist3f = 0;
        int hammdist0b = 0;
        int hammdist1b = 0;
        int hammdist2b = 0;
        int hammdist3b = 0;

        for(int i = 0; i < 11; i++)
        {
            if( id[i] != p12_0[i] ) hammdist0f++;
        }
        for(int i = 0; i < 11; i++)
        {
            if( id[i] != p12_1[i] ) hammdist1f++;
        }
        for(int i = 0; i < 11; i++)
        {
            if( id[i] != p12_2[i] ) hammdist2f++;
        }
        for(int i = 0; i < 11; i++)
        {
            if( id[i] != p12_3[i] ) hammdist3f++;
        }
        for(int i = 0; i < 11; i++)
        {
            if( id[i] != p12_0[i+1] ) hammdist0b++;
        }
        for(int i = 0; i < 11; i++)
        {
            if( id[i] != p12_1[i+1] ) hammdist1b++;
        }
        for(int i = 0; i < 11; i++)
        {
            if( id[i] != p12_2[i+1] ) hammdist2b++;
        }
        for(int i = 0; i < 11; i++)
        {
            if( id[i] != p12_3[i+1] ) hammdist3b++;
        }

        if( hammdist0f == 0 && stripe12_0.size() == 0 )
        {
            stripe12_0 = centroids;
            stripe12_0.push_back(cv::Point2f(-1,-1));
        }
        else if( hammdist1f == 0 && stripe12_1.size() == 0 )
        {
            stripe12_1 = centroids;
            stripe12_1.push_back(cv::Point2f(-1,-1));
        }
        else if( hammdist2f == 0 && stripe12_2.size() == 0 )
        {
            stripe12_2 = centroids;
            stripe12_2.push_back(cv::Point2f(-1,-1));
        }
        else if( hammdist3f == 0 && stripe12_3.size() == 0 )
        {
            stripe12_3 = centroids;
            stripe12_3.push_back(cv::Point2f(-1,-1));
        }

        else if( hammdist0b == 0 && stripe12_0.size() == 0 )
        {
            stripe12_0.push_back(cv::Point2f(-1,-1));
            stripe12_0.insert( stripe12_0.end(), centroids.begin(), centroids.end());
        }
        else if( hammdist1b == 0 && stripe12_1.size() == 0 )
        {
            stripe12_1.push_back(cv::Point2f(-1,-1));
            stripe12_1.insert( stripe12_1.end(), centroids.begin(), centroids.end());
        }
        else if( hammdist2b == 0 && stripe12_2.size() == 0 )
        {
            stripe12_2.push_back(cv::Point2f(-1,-1));
            stripe12_2.insert( stripe12_2.end(), centroids.begin(), centroids.end());
        }
        else if( hammdist3b == 0 && stripe12_3.size() == 0 )
        {
            stripe12_3.push_back(cv::Point2f(-1,-1));
            stripe12_3.insert( stripe12_3.end(), centroids.begin(), centroids.end());
        }
    }
}

void findelectrodes::drawDebug() //draw contours, found elctrode positions and thresholded image
{
    cv::Mat drawing = cv::Mat::zeros( img.size(), CV_8UC3 );

    cv::RNG rng(12345);
    for(int i = 0; i < stripes.size(); i++)
    {
        cv::drawContours( drawing, contours, stripes[i], cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255)), 1, 8, hierarchy, 0, cv::Point() );
    }

    if( stripe8_0.size() == 8 )
    {
        for( int i = 0; i < stripe8_0.size(); i++)
        {
            if( stripe8_0[i].x >= 0 )
            {
                cv::circle(drawing, stripe8_0[i], 0, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_0[i], 1, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_0[i], 2, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_0[i], 3, cv::Scalar(255,255,0), 1);
            }
        }
        std::cout << "Found: electrodestripe 8_0" << std::endl;
    }

    if( stripe8_1.size() == 8 )
    {
        for( int i = 0; i < stripe8_1.size(); i++)
        {
            if( stripe8_1[i].x >= 0 )
            {
                cv::circle(drawing, stripe8_1[i], 0, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_1[i], 1, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_1[i], 2, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_1[i], 3, cv::Scalar(255,255,0), 1);
            }
        }
        std::cout << "Found: electrodestripe 8_1" << std::endl;
    }

    if( stripe8_2.size() == 8 )
    {
        for( int i = 0; i < stripe8_2.size(); i++)
        {
            if( stripe8_2[i].x >= 0 )
            {
                cv::circle(drawing, stripe8_2[i], 0, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_2[i], 1, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_2[i], 2, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_2[i], 3, cv::Scalar(255,255,0), 1);
            }
        }
        std::cout << "Found: electrodestripe 8_2" << std::endl;
    }

    if( stripe8_3.size() == 8 )
    {
        for( int i = 0; i < stripe8_3.size(); i++)
        {
            if( stripe8_3[i].x >= 0 )
            {
                cv::circle(drawing, stripe8_3[i], 0, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_3[i], 1, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_3[i], 2, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe8_3[i], 3, cv::Scalar(255,255,0), 1);
            }
        }
        std::cout << "Found: electrodestripe 8_3" << std::endl;
    }

    if( stripe12_0.size() == 12 )
    {
        for( int i = 0; i < stripe12_0.size(); i++)
        {
            if( stripe12_0[i].x >= 0 )
            {
                cv::circle(drawing, stripe12_0[i], 0, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_0[i], 1, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_0[i], 2, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_0[i], 3, cv::Scalar(255,255,0), 1);
            }
        }
        std::cout << "Found: electrodestripe 12_0" << std::endl;
    }

    if( stripe12_1.size() == 12 )
    {
        for( int i = 0; i < stripe12_1.size(); i++)
        {
            if( stripe12_1[i].x >= 0 )
            {
                cv::circle(drawing, stripe12_1[i], 0, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_1[i], 1, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_1[i], 2, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_1[i], 3, cv::Scalar(255,255,0), 1);
            }
        }
        std::cout << "Found: electrodestripe 12_1" << std::endl;
    }

    if( stripe12_2.size() == 12 )
    {
        for( int i = 0; i < stripe12_2.size(); i++)
        {
            if( stripe12_2[i].x >= 0 )
            {
                cv::circle(drawing, stripe12_2[i], 0, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_2[i], 1, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_2[i], 2, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_2[i], 3, cv::Scalar(255,255,0), 1);
            }
        }
        std::cout << "Found: electrodestripe 12_2" << std::endl;
    }

    if( stripe12_3.size() == 12 )
    {
        for( int i = 0; i < stripe12_3.size(); i++)
        {
            if( stripe12_3[i].x >= 0 )
            {
                cv::circle(drawing, stripe12_3[i], 0, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_3[i], 1, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_3[i], 2, cv::Scalar(255,255,0), 1);
                cv::circle(drawing, stripe12_3[i], 3, cv::Scalar(255,255,0), 1);
            }
        }
        std::cout << "Found: electrodestripe 12_3" << std::endl;
    }

    cv::namedWindow("Debug: Contours", 0);
    cv::imshow("Debug: Contours", drawing);
    if( !thresh.empty() )
    {
        cv::namedWindow("Debug: Tresholded", 0);
        cv::imshow("Debug: Tresholded", thresh);
    }
}
