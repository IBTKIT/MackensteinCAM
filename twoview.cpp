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


#include "twoview.h"

TwoView::TwoView(cv::Mat img0, cv::Mat img1, int orient0, int orient1, std::string path_, std::string save_)
{
    // constructor: creating all data which is needed for a new TwoView instance
    encounterederrors = 0;
    orig_img0 = img0;
    orig_img1 = img1;
    cv::cvtColor(orig_img0, orig_img0bw, CV_BGR2GRAY);
    cv::cvtColor(orig_img1, orig_img1bw, CV_BGR2GRAY);
    orientation0 = orient0;
    orientation1 = orient1;
    drawn_img0 = orig_img0.clone();
    drawn_img1 = orig_img1.clone();
    newPoint[0] = cv::Point2f(-1, -1);
    newPoint[1] = cv::Point2f(-1, -1);
    drawEpi = 1;
    SubimgPos = 0;
    nextSubimgPos = 0;
    subImage.create(cv::Size(1000, 500), img0.type());
    path = path_;
    save = save_;
    markerfinder = new findmarker( orig_img0, orig_img1);

    stripe8_0 = new match;
    stripe8_0->name = "(0) Stripe8 0 (A1-A8)";
    stripe8_1 = new match;
    stripe8_1->name = "(1) Stripe8 1 (A9-A16)";
    stripe8_2 = new match;
    stripe8_2->name = "(2) Stripe8 2 (A17-A24)";
    stripe8_3 = new match;
    stripe8_3->name = "(3) Stripe8 3 (A25-A32)";
    stripe12_0 = new match;
    stripe12_0->name = "(4) Stripe12 0 (B1-B12)";
    stripe12_1 = new match;
    stripe12_1->name = "(5) Stripe12 1 (B13-B24)";
    stripe12_2 = new match;
    stripe12_2->name = "(6) Stripe12 2 (C1-C12)";
    stripe12_3 = new match;
    stripe12_3->name = "(7) Stripe12 3 (C13-C24)";
    anatomicalMarkers = new match;
    anatomicalMarkers->name = "(8) Anatomical markers";
    fundamentalMatches = new match;
    fundamentalMatches->name = "(9) ArUco markers";

	// empty elements in stripes are required from the beginning on
	// (same is done after running TwoView::matchelectrodes(), bug was that without having any match, it was impossible to properly add electrodes manually)
    for( int i=0; i < 8; i++)
    {
        stripe8_0->center2d_0.push_back(cv::Point2f( -1, -1));
        stripe8_0->center2d_1.push_back(cv::Point2f( -1, -1));
        stripe8_1->center2d_0.push_back(cv::Point2f( -1, -1));
        stripe8_1->center2d_1.push_back(cv::Point2f( -1, -1));
        stripe8_2->center2d_0.push_back(cv::Point2f( -1, -1));
        stripe8_2->center2d_1.push_back(cv::Point2f( -1, -1));
        stripe8_3->center2d_0.push_back(cv::Point2f( -1, -1));
        stripe8_3->center2d_1.push_back(cv::Point2f( -1, -1));

        stripe8_0->center3d_0.push_back( cv::Point3f( -1, -1, -1));
        stripe8_0->center3d_1.push_back( cv::Point3f( -1, -1, -1));
        stripe8_0->id.push_back(-1);
        stripe8_0->description.push_back("");
        stripe8_1->center3d_0.push_back( cv::Point3f( -1, -1, -1));
        stripe8_1->center3d_1.push_back( cv::Point3f( -1, -1, -1));
        stripe8_1->id.push_back(-1);
        stripe8_1->description.push_back("");
        stripe8_2->center3d_0.push_back( cv::Point3f( -1, -1, -1));
        stripe8_2->center3d_1.push_back( cv::Point3f( -1, -1, -1));
        stripe8_2->id.push_back(-1);
        stripe8_2->description.push_back("");
        stripe8_3->center3d_0.push_back( cv::Point3f( -1, -1, -1));
        stripe8_3->center3d_1.push_back( cv::Point3f( -1, -1, -1));
        stripe8_3->id.push_back(-1);
        stripe8_3->description.push_back("");
    }

    for( int i=0; i < 12; i++)
    {
        stripe12_0->center2d_0.push_back(cv::Point2f( -1, -1));
        stripe12_0->center2d_1.push_back(cv::Point2f( -1, -1));
        stripe12_1->center2d_0.push_back(cv::Point2f( -1, -1));
        stripe12_1->center2d_1.push_back(cv::Point2f( -1, -1));
        stripe12_2->center2d_0.push_back(cv::Point2f( -1, -1));
        stripe12_2->center2d_1.push_back(cv::Point2f( -1, -1));
        stripe12_3->center2d_0.push_back(cv::Point2f( -1, -1));
        stripe12_3->center2d_1.push_back(cv::Point2f( -1, -1));

        stripe12_0->center3d_0.push_back( cv::Point3f( -1, -1, -1));
        stripe12_0->center3d_1.push_back( cv::Point3f( -1, -1, -1));
        stripe12_0->id.push_back(-1);
        stripe12_0->description.push_back("");
        stripe12_1->center3d_0.push_back( cv::Point3f( -1, -1, -1));
        stripe12_1->center3d_1.push_back( cv::Point3f( -1, -1, -1));
        stripe12_1->id.push_back(-1);
        stripe12_1->description.push_back("");
        stripe12_2->center3d_0.push_back( cv::Point3f( -1, -1, -1));
        stripe12_2->center3d_1.push_back( cv::Point3f( -1, -1, -1));
        stripe12_2->id.push_back(-1);
        stripe12_2->description.push_back("");
        stripe12_3->center3d_0.push_back( cv::Point3f( -1, -1, -1));
        stripe12_3->center3d_1.push_back( cv::Point3f( -1, -1, -1));
        stripe12_3->id.push_back(-1);
        stripe12_3->description.push_back("");
    }
    

    pointStorage = 0;

    electrodes0 = new findelectrodes(img0, orientation0);
    electrodes1 = new findelectrodes(img1, orientation1);

    cv::FileStorage fs_LLRR(path + "calibLLRR_CAMS.xml", cv::FileStorage::READ); //reading calibration file (obtained by calibration tool of opencv-lib)
    fs_LLRR["camera_matrix"] >> intrinsic_LLRR;
    //intrinsic_LLRR = (cv::Mat_<double>(3,3) << 4288.865076, 0, 2607.6825515, 0, 4280.650303, 1787.3351853, 0, 0, 1); //passing calibration paramenter manually (debugging)
    cv::invert(intrinsic_LLRR, intrinsic_LLRR_inv, cv::DECOMP_LU); //calculating inverse of intrinsic matrix
    fs_LLRR["distortion_coefficients"] >> distCoeffs_LLRR;

    cv::FileStorage fs_LTRT(path + "calibLTRT_CAMH.xml", cv::FileStorage::READ); //reading calibration file (obtained by calibration tool of opencv-lib)
    fs_LTRT["camera_matrix"] >> intrinsic_LTRT;
    //intrinsic_LTRT = (cv::Mat_<double>(3,3) << 4288.865076, 0, 2607.6825515, 0, 4280.650303, 1787.3351853, 0, 0, 1); //passing calibration paramenter manually (debugging)
    cv::invert(intrinsic_LTRT, intrinsic_LTRT_inv, cv::DECOMP_LU); //calculating inverse of intrinsic matrix
    fs_LTRT["distortion_coefficients"] >> distCoeffs_LTRT;

    setSize(1); //default size for fonts and points
    drawImages();
}

TwoView::~TwoView() //destructor: free all used memory at deletion time of instance: 	watch for memory leaks
{
    orig_img0.release();
    orig_img1.release();
    orig_img0bw.release();
    orig_img1bw.release();
    drawn_img0.release();
    drawn_img1.release();
    withEpilines.release();
    subImage.release();
    fundamental.release();
    intrinsic_LLRR.release();
    intrinsic_LTRT.release();
    intrinsic_LLRR_inv.release();
    intrinsic_LTRT_inv.release();
    distCoeffs_LLRR.release();
    distCoeffs_LTRT.release();
    essential.release();
    cameraP0.release();
    delete markerfinder;
    delete fundamentalMatches;
}

void TwoView::addPoint(int imgNo,unsigned int x, unsigned int y) //manually add new point which was entered by mouse events
{
    if( imgNo != 0)
        imgNo = 1;
    newPoint[imgNo] = cv::Point2f(x, y);

    if( newPoint[0] != cv::Point2f(-1, -1) && newPoint[1] != cv::Point2f(-1, -1))
    {
        //selecting stripe...
        std::cout << std::endl << std::endl;
        std::cout << "(0) Stripe8 0 (A1-A8)" << std::endl;
        std::cout << "(1) Stripe8 1 (A9-A16)" << std::endl;
        std::cout << "(2) Stripe8 2 (A17-A24)" << std::endl;
        std::cout << "(3) Stripe8 3 (A25-A32)" << std::endl;
        std::cout << "(4) Stripe12 0 (B1-B12)" << std::endl;
        std::cout << "(5) Stripe12 1 (B13-B24)" << std::endl;
        std::cout << "(6) Stripe12 2 (C1-C12)" << std::endl;
        std::cout << "(7) Stripe12 3 (C13-C24)" << std::endl;
        std::cout << "(8) Anatomical markers" << std::endl;
        std::cout << "(9) ArUco markers" << std::endl;

        char stripe = 'a';
        int id = -1;
        string markerdescription = "";
        while( !(stripe >= 48 && stripe <= 57) && stripe != 'c') //48..55 : ASCII for 0..9
        {
            std::cout << "Choose stripe for adding point ((c) = cancel) : ";
            stripe = getchar();
        }
        if(stripe != 'c')
        {
            if( stripe <= 51 ) //51 : ASCII for 3
            {
                //selecting electrode...
                int istripe = stripe - '0'; // '0' => 0, '1' => 1
                std::cout << std::endl << std::endl;
                std::cout << "(0) Electrode A" << istripe*8+1 << std::endl; // must be in line with void TwoView::writeElectrodefile()
                std::cout << "(1) Electrode A" << istripe*8+2 << std::endl;
                std::cout << "(2) Electrode A" << istripe*8+3 << std::endl;
                std::cout << "(3) Electrode A" << istripe*8+4 << std::endl;
                std::cout << "(4) Electrode A" << istripe*8+5 << std::endl;
                std::cout << "(5) Electrode A" << istripe*8+6 << std::endl;
                std::cout << "(6) Electrode A" << istripe*8+7 << std::endl;
                std::cout << "(7) Electrode A" << istripe*8+8 << std::endl;

                while( !(id >= 0 && id <= 7))
                {
                    std::cout << "Choose electrode: ";
                    std::cin >> id;
//                    cin.clear();
//                    cin.sync();
                }
            }
            else if(stripe == 56 ) // ASCII for 8
            {
                std::cout << std::endl << std::endl;
                std::cout << "Please enter a description for the anatomical marker:" << std::endl;
//				while( !(markerdescription.empty()) )   //does not want the condition, always skips the cin then.
//                {
                    std::cout << "Enter marker description: ";
                    std::cin >> markerdescription;
//                    cin.clear();
//                    cin.sync();
//                }
                
            }
            else if(stripe == 57 ) // ASCII for 9
            {
                std::cout << std::endl << std::endl;
                std::cout << "Please enter the ID of the ArUco marker (e.g., 0 or 80 or 160 or 240)." << std::endl; //id must be unique for void scale(TwoView* s) in main.cpp to work
				while( !(id >= 0 && id <= 1023))
                {
                    std::cout << "Choose marker ID: ";
                    std::cin >> id;
//                    cin.clear();
//                    cin.sync();
                }
                
            }
            else if( (stripe >= 52 )  && (stripe <= 53 ) ) // ASCII for 4,5
            {
                //selecting electrode...
                int istripe = stripe - '0'; // '0' => 0, '1' => 1
                std::cout << std::endl << std::endl;
                std::cout << "(0)  Electrode B" << (istripe-4)*12+1 << std::endl; // must be in line with void TwoView::writeElectrodefile()
                std::cout << "(1)  Electrode B" << (istripe-4)*12+2 << std::endl;
                std::cout << "(2)  Electrode B" << (istripe-4)*12+3 << std::endl;
                std::cout << "(3)  Electrode B" << (istripe-4)*12+4 << std::endl;
                std::cout << "(4)  Electrode B" << (istripe-4)*12+5 << std::endl;
                std::cout << "(5)  Electrode B" << (istripe-4)*12+6 << std::endl;
                std::cout << "(6)  Electrode B" << (istripe-4)*12+7 << std::endl;
                std::cout << "(7)  Electrode B" << (istripe-4)*12+8 << std::endl;
                std::cout << "(8)  Electrode B" << (istripe-4)*12+9 << std::endl;
                std::cout << "(9)  Electrode B" << (istripe-4)*12+10 << std::endl;
                std::cout << "(10) Electrode B" << (istripe-4)*12+11 << std::endl;
                std::cout << "(11) Electrode B" << (istripe-4)*12+12 << std::endl;

                while( !(id >= 0 && id <= 11))
                {
                    std::cout << "Choose electrode: ";
                    std::cin >> id;
//                    cin.clear();
//                    cin.sync();
                }
            }
            else if( (stripe >= 54 )  && (stripe <= 55 ) ) // ASCII for 6,7
            {
                //selecting electrode...
                int istripe = stripe - '0'; // '0' => 0, '1' => 1
                std::cout << std::endl << std::endl;
                std::cout << "(0)  Electrode C" << (istripe-6)*12+1 << std::endl; // must be in line with void TwoView::writeElectrodefile()
                std::cout << "(1)  Electrode C" << (istripe-6)*12+2 << std::endl;
                std::cout << "(2)  Electrode C" << (istripe-6)*12+3 << std::endl;
                std::cout << "(3)  Electrode C" << (istripe-6)*12+4 << std::endl;
                std::cout << "(4)  Electrode C" << (istripe-6)*12+5 << std::endl;
                std::cout << "(5)  Electrode C" << (istripe-6)*12+6 << std::endl;
                std::cout << "(6)  Electrode C" << (istripe-6)*12+7 << std::endl;
                std::cout << "(7)  Electrode C" << (istripe-6)*12+8 << std::endl;
                std::cout << "(8)  Electrode C" << (istripe-6)*12+9 << std::endl;
                std::cout << "(9)  Electrode C" << (istripe-6)*12+10 << std::endl;
                std::cout << "(10) Electrode C" << (istripe-6)*12+11 << std::endl;
                std::cout << "(11) Electrode C" << (istripe-6)*12+12 << std::endl;

                while( !(id >= 0 && id <= 11))
                {
                    std::cout << "Choose electrode: ";
                    std::cin >> id;
//                    cin.clear();
//                    cin.sync();
                }
            }
            else
            {
	            std::cout << "Wrong choice for stripe. Manually added point will be ignored." << std::endl; //make sure this complies with #IGNORINGOTHERCHOICES
            }

            //get subpixel information for new point
            if (1==0) //#BYPASSSUBPIX (search for flag to see related conditions in the code)
            {
            std::vector<cv::Point2f> subpix;
            subpix.push_back(newPoint[0]);
            cv::cornerSubPix(orig_img0bw, subpix, cvSize(4,4), cvSize(1,1) ,cvTermCriteria ( CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,100,0.001 ));
            newPoint[0] = subpix[0];
            subpix.clear();
            subpix.push_back(newPoint[1]);
            cv::cornerSubPix(orig_img1bw, subpix, cvSize(4,4), cvSize(1,1) ,cvTermCriteria ( CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,100,0.001 ));
            newPoint[1] = subpix[0];
            subpix.clear();
            }
            
            //adding point to point vector
            if(stripe == '0' && stripe8_0->center2d_0.size() == 8)
            {
                stripe8_0->center2d_0[id] = newPoint[0];
                stripe8_0->center2d_1[id] = newPoint[1];
            }
            else if(stripe == '1' && stripe8_1->center2d_0.size() == 8)
            {
                stripe8_1->center2d_0[id] = newPoint[0];
                stripe8_1->center2d_1[id] = newPoint[1];
            }
            else if(stripe == '2' && stripe8_2->center2d_0.size() == 8)
            {
                stripe8_2->center2d_0[id] = newPoint[0];
                stripe8_2->center2d_1[id] = newPoint[1];
            }
            else if(stripe == '3' && stripe8_3->center2d_0.size() == 8)
            {
                stripe8_3->center2d_0[id] = newPoint[0];
                stripe8_3->center2d_1[id] = newPoint[1];
            }
            else if(stripe == '4' && stripe12_0->center2d_0.size() == 12)
            {
                stripe12_0->center2d_0[id] = newPoint[0];
                stripe12_0->center2d_1[id] = newPoint[1];
            }
            else if(stripe == '5' && stripe12_1->center2d_0.size() == 12)
            {
                stripe12_1->center2d_0[id] = newPoint[0];
                stripe12_1->center2d_1[id] = newPoint[1];
            }
            else if(stripe == '6' && stripe12_2->center2d_0.size() == 12)
            {
                stripe12_2->center2d_0[id] = newPoint[0];
                stripe12_2->center2d_1[id] = newPoint[1];
            }
            else if(stripe == '7' && stripe12_3->center2d_0.size() == 12)
            {
                stripe12_3->center2d_0[id] = newPoint[0];
                stripe12_3->center2d_1[id] = newPoint[1];
            }												//Ignoring other choices, e.g. '8'. Make sure this complies with #IGNORINGOTHERCHOICES
            else if(stripe == '8')
            {
            	//add new entry to anatomical markers
                anatomicalMarkers->center2d_0.push_back( newPoint[0] );
                anatomicalMarkers->center2d_1.push_back( newPoint[1] );
                anatomicalMarkers->center3d_0.push_back( cv::Point3f( -1, -1, -1));
                anatomicalMarkers->center3d_1.push_back( cv::Point3f( -1, -1, -1));
                anatomicalMarkers->description.push_back(markerdescription);
                anatomicalMarkers->id.push_back( anatomicalMarkers->center2d_0.size()-1 ); //set id to position in array
			    std::cout << "####################################################################" << std::endl;
			    std::cout << "New anatomical marker has been added successfully." << std::endl;
			    std::cout << "Anatomical markers (only displaying those of the active image pair):" << std::endl;
                for( int i1=0; i1 < anatomicalMarkers->center2d_0.size(); i1++)
                {
				std::cout << "id: " << anatomicalMarkers->id[i1] <<
					" img0: " << anatomicalMarkers->center2d_0[i1] << " img1: " << anatomicalMarkers->center2d_1[i1] << " description: " << anatomicalMarkers->description[i1] << std::endl;
                }
			    std::cout << "####################################################################" << std::endl;
				std::cout << "--- Attention: Anatomical markers of different image pairs are lateron matched according to their position in the array above. ---\n";
				std::cout << "--- Wrong correspondences between marker IDs here and marker IDs in the other image pair will mess up the joining of the two point clouds (one is produced for each image pair, see output files \"pair1.esf\" and \"pair2.esf\"). Differing identifies in the two clouds will screw up output file \"bothpairs.esf\"! ---\n";
				std::cout << "--- => You may now delete existing markers for cleanup. ---\n";
			    std::cout << "####################################################################" << std::endl;
				
            	//provide option to delete another marker then:
				char my2delete = 'a';
				while( !(my2delete >= 48 && my2delete <= 57) && my2delete != 's') //48..55 : ASCII for 0..9
				{
				std::cout << "Choose entry (0,...) to delete as it appears in the list of anatomical markers ((s) = skip) : ";
				std::cin >> my2delete;
				}

				if(my2delete != 's')
				{
				int ia = my2delete - '0'; // '0' => 0, '1' => 1

				anatomicalMarkers->center2d_0.erase(anatomicalMarkers->center2d_0.begin() + ia);
                anatomicalMarkers->center2d_1.erase(anatomicalMarkers->center2d_1.begin() + ia);
                anatomicalMarkers->center3d_0.erase(anatomicalMarkers->center3d_0.begin() + ia);
                anatomicalMarkers->center3d_1.erase(anatomicalMarkers->center3d_1.begin() + ia);
                anatomicalMarkers->description.erase(anatomicalMarkers->description.begin() + ia);
                anatomicalMarkers->id.erase(anatomicalMarkers->id.begin() + ia);
			    std::cout << "####################################################################" << std::endl;
			    std::cout << "New anatomical markers (only displaying those of the active image pair):" << std::endl;
   	             for( int i1=0; i1 < anatomicalMarkers->center2d_0.size(); i1++)
	                {
					std::cout << "id: " << anatomicalMarkers->id[i1] <<
						" img0: " << anatomicalMarkers->center2d_0[i1] << " img1: " << anatomicalMarkers->center2d_1[i1] << " description: " << anatomicalMarkers->description[i1] << std::endl;
	                }
				}
			    std::cout << "####################################################################" << std::endl;

            }
            else if(stripe == '9')
            {
            	//add new entry to ArUco markers
                fundamentalMatches->center2d_0.push_back( newPoint[0] );
                fundamentalMatches->center2d_1.push_back( newPoint[1] );
                fundamentalMatches->center3d_0.push_back( cv::Point3f( -1, -1, -1));
                fundamentalMatches->center3d_1.push_back( cv::Point3f( -1, -1, -1));
                fundamentalMatches->description.push_back("");
                fundamentalMatches->id.push_back( id );
			    std::cout << "New fundamental matches (only displaying those of the active image pair):" << std::endl;
                for( int i1=0; i1 < fundamentalMatches->center2d_0.size(); i1++)
                {
				std::cout << "id: " << fundamentalMatches->id[i1] <<
					" img0: " << fundamentalMatches->center2d_0[i1] << " img1: " << fundamentalMatches->center2d_1[i1] << std::endl;
                }

            	//provide option to delete another marker then:
				char my2delete = 'a';
				while( !(my2delete >= 48 && my2delete <= 57) && my2delete != 's') //48..55 : ASCII for 0..9
				{
				std::cout << "Choose entry (0,...) to delete as it appears in the list of fundamental matches ((s) = skip) : ";
				std::cin >> my2delete;
				}

				if(my2delete != 's')
				{
				int ia = my2delete - '0'; // '0' => 0, '1' => 1

				fundamentalMatches->center2d_0.erase(fundamentalMatches->center2d_0.begin() + ia);
                fundamentalMatches->center2d_1.erase(fundamentalMatches->center2d_1.begin() + ia);
                fundamentalMatches->center3d_0.erase(fundamentalMatches->center3d_0.begin() + ia);
                fundamentalMatches->center3d_1.erase(fundamentalMatches->center3d_1.begin() + ia);
                fundamentalMatches->description.erase(fundamentalMatches->description.begin() + ia);
                fundamentalMatches->id.erase(fundamentalMatches->id.begin() + ia);
			    std::cout << "New fundamental matches (only displaying those of the active image pair):" << std::endl;
   	             for( int i1=0; i1 < fundamentalMatches->center2d_0.size(); i1++)
	                {
					std::cout << "id: " << fundamentalMatches->id[i1] <<
						" img0: " << fundamentalMatches->center2d_0[i1] << " img1: " << fundamentalMatches->center2d_1[i1] << std::endl;
	                }
				}

            }
            
            manual.push_back(cv::Point2i(stripe, id));
        }

        //resetting variables
        newPoint[0] = cv::Point2f(-1, -1);
        newPoint[1] = cv::Point2f(-1, -1);
    }
    drawImages();
}

void TwoView::removePoint(int imgNo,unsigned int x, unsigned int y)
{
    //checking all stripes for removal, checking fundamental points as well
    removePointInStripe(imgNo, x, y, stripe8_0);
    removePointInStripe(imgNo, x, y, stripe8_1);
    removePointInStripe(imgNo, x, y, stripe8_2);
    removePointInStripe(imgNo, x, y, stripe8_3);
    removePointInStripe(imgNo, x, y, stripe12_0);
    removePointInStripe(imgNo, x, y, stripe12_1);
    removePointInStripe(imgNo, x, y, stripe12_2);
    removePointInStripe(imgNo, x, y, stripe12_3);
    removePointInStripe(imgNo, x, y, anatomicalMarkers);
    removePointInStripe(imgNo, x, y, fundamentalMatches);
    drawImages();
}

int TwoView::removePointInStripe(int imgNo,unsigned int x, unsigned int y, match *m)
{
    //checking if entered point is in the range of 15 pixels of an existing point, if(true): delete existing point
    if( !m->center2d_0.empty())
    {
        int xdiff = 0;
        int ydiff = 0;
        for(unsigned int i = 0; i < m->center2d_0.size(); i++)
        {
            if( imgNo == 0)
            {
                xdiff = abs( m->center2d_0[i].x - x);
                ydiff = abs( m->center2d_0[i].y - y);
            }
            else
            {
                xdiff = abs( m->center2d_1[i].x - x);
                ydiff = abs( m->center2d_1[i].y - y);
            }
            if( xdiff < 15 && ydiff < 15)
            {
                m->center2d_0.erase(m->center2d_0.begin() + i);
                m->center2d_1.erase(m->center2d_1.begin() + i);
                m->center3d_0.erase(m->center3d_0.begin() + i);
                m->center3d_1.erase(m->center3d_1.begin() + i);
                m->description.erase(m->description.begin() + i);
                m->id.erase(m->id.begin() + i);
                return 1;
            }
        }
    }
    return 0;
}

void TwoView::removeAllPoints()
{
    fundamentalMatches->clear();
    anatomicalMarkers->clear();
    stripe8_0->clear();
    stripe8_1->clear();
    stripe8_2->clear();
    stripe8_3->clear();
    stripe12_0->clear();
    stripe12_1->clear();
    stripe12_2->clear();
    stripe12_3->clear();
}

//adding point directly to point vector
void TwoView::addMatch(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, int id, std::string description)
{
    getPointstorage()->center2d_0.push_back( cv::Point2f(x0, y0));
    getPointstorage()->center2d_1.push_back( cv::Point2f(x1, y1));
    getPointstorage()->center3d_0.push_back( cv::Point3f(-1, -1, -1));
    getPointstorage()->center3d_1.push_back( cv::Point3f(-1, -1, -1));
    getPointstorage()->description.push_back( description);
    getPointstorage()->id.push_back(id);
    drawImages();
}

void TwoView::addMatch(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{
    addMatch(x0, y0, x1, y1, -1, "");
}

//setting new position of existing point, done in refinement window
void TwoView::modifySubimgsMatch(unsigned int x, unsigned int y)
{
    if( !getPointstorage()->center2d_0.empty() && SubimgPos < getPointstorage()->center2d_0.size())
    {
        std::vector<cv::Point2f> subpix;
        if( x < 500)
        {
            cv::Point point = cv::Point2f(getPointstorage()->center2d_0[SubimgPos].x - 250 + x, getPointstorage()->center2d_0[SubimgPos].y - 250 + y);
            if (1==0) //#BYPASSSUBPIX (search for flag to see related conditions in the code)
            {
            subpix.push_back(point);
            cv::cornerSubPix(orig_img0bw, subpix, cvSize(4,4), cvSize(1,1) ,cvTermCriteria ( CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,100,0.001 ));
            getPointstorage()->center2d_0[SubimgPos].x = subpix[0].x;
            getPointstorage()->center2d_0[SubimgPos].y = subpix[0].y;
            subpix.clear();
            }
            else
            {
            getPointstorage()->center2d_0[SubimgPos] = point;
            }
        }
        else
        {
            cv::Point point = cv::Point2f(getPointstorage()->center2d_1[SubimgPos].x - 750 + x, getPointstorage()->center2d_1[SubimgPos].y - 250 + y);
            if (1==0) //#BYPASSSUBPIX (search for flag to see related conditions in the code)
            {
            subpix.push_back(point);
            cv::cornerSubPix(orig_img0bw, subpix, cvSize(4,4), cvSize(1,1) ,cvTermCriteria ( CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,100,0.001 ));
            getPointstorage()->center2d_1[SubimgPos].x = subpix[0].x;
            getPointstorage()->center2d_1[SubimgPos].y = subpix[0].y;
            subpix.clear();
            }
            else
            {
            getPointstorage()->center2d_1[SubimgPos] = point;
            }
        }
    }
}

//checking for fundamental markers (aruco markers) in images
void TwoView::addFundamentalmarkers()
{
    match tmp = markerfinder->getFundamentalemarkers();
    std::cout << "FINDING FUNDAMENTAL MARKERS FOR IMAGE PAIR:" << std::endl;
    for( int i=0; i < tmp.center2d_0.size(); i++)
    {
        std::cout << "Found id: " << tmp.id[i] <<
                     " img0: " << tmp.center2d_0[i] << " img1: " << tmp.center2d_1[i] << std::endl;
    }
    fundamentalMatches->center2d_0.insert(fundamentalMatches->center2d_0.begin(), tmp.center2d_0.begin(), tmp.center2d_0.end());
    fundamentalMatches->center2d_1.insert(fundamentalMatches->center2d_1.begin(), tmp.center2d_1.begin(), tmp.center2d_1.end());
    fundamentalMatches->center3d_0.insert(fundamentalMatches->center3d_0.begin(), tmp.center3d_0.begin(), tmp.center3d_0.end()); //not sure if this is used. Carries vector with the following contents: [-1, -1, -1]
    fundamentalMatches->center3d_1.insert(fundamentalMatches->center3d_1.begin(), tmp.center3d_1.begin(), tmp.center3d_1.end()); //not sure if this is used. Carries vector with the following contents: [-1, -1, -1]
    fundamentalMatches->description.insert(fundamentalMatches->description.begin(), tmp.description.begin(), tmp.description.end());
    fundamentalMatches->id.insert(fundamentalMatches->id.begin(), tmp.id.begin(), tmp.id.end());
    drawImages();
}

cv::Mat TwoView::getImageWithPoints(int imgNo)
{
/* runs well on Windows computers
    if( imgNo == 0)
        return drawn_img0;
    else
        return drawn_img1;
*/      

// Window scaling for MacOSX computers. Is linked to adjustment in main.cpp, see #WINDOWSCALING in comments.
   if( imgNo == 0)
        cv::resize(drawn_img0, tmp, cv::Size(), 0.25, 0.25);
    else
        cv::resize(drawn_img1, tmp, cv::Size(), 0.25, 0.25);
    return tmp;
}

cv::Mat TwoView::getSubimage() // getting 500x500 pixel image frame in the areae of next electrode position in stripe
{
    if( !getPointstorage()->center2d_0.empty() && SubimgPos < getPointstorage()->center2d_0.size())
    {
        drawSubimage(SubimgPos);
        return subImage;
    }
    else
    {
        cv::Mat black;
        black.create(500,1000,CV_8UC3);
        return black;
    }
}

cv::Mat TwoView::getNextSubimage()
{
    if( !getPointstorage()->center2d_0.empty() && nextSubimgPos < getPointstorage()->center2d_0.size())
	{
        if( getPointstorage()->center2d_0[nextSubimgPos] != cv::Point2f(-1,-1)) //check whether Point2f has default value, in that case (else) create black window instead of subimage
        {
            drawSubimage(nextSubimgPos);
            SubimgPos = nextSubimgPos;
            if( nextSubimgPos == (getPointstorage()->center2d_0.size() - 1))
                nextSubimgPos = 0;
            else
                nextSubimgPos++;
            std::stringstream number;
            number << SubimgPos;
            cv::putText(subImage, number.str(), cv::Point2f(30,30), CV_FONT_NORMAL, 1, CV_RGB(0,0,255));
            return subImage;
        }
        else
        {
            SubimgPos = nextSubimgPos;
            if( nextSubimgPos == (getPointstorage()->center2d_0.size() - 1))
                nextSubimgPos = 0;
            else
                nextSubimgPos++;

            cv::Mat black;
            black.create(500,1000,CV_8UC3);
            cv::rectangle(black, cv::Point2f(30,2), cv::Point2f(80,40), CV_RGB(0,0,0), -1, 8, 0); //before plotting number, "refresh" background somehow
            std::stringstream number;
            number << SubimgPos;
            cv::putText(black, number.str(), cv::Point2f(30,30), CV_FONT_NORMAL, 1, CV_RGB(0,0,255));
            return black;
        }
    }
    else
    {
        nextSubimgPos = 0;
        SubimgPos = 0;
        cv::Mat black;
        black.create(500,1000,CV_8UC3);
        return black;
    }
}

void TwoView::toggleDrawEpi() //enalbe/disable of drawing epipolar lines
{
    drawEpi = (drawEpi == 0 ? 1 : 0);
    drawImages();
}

void TwoView::calculateFundamentalMat( int method, int markers) // markers: 0 = just fundamentalMatches, 1 = just electrodes, else = all
{
    //removing radial and tangential distortion of given marker points
    std::vector<std::vector<cv::Point2f> > undistortedMatches;
    undistortedMatches.push_back(std::vector<cv::Point2f>());
    undistortedMatches.push_back(std::vector<cv::Point2f>());
    std::vector<cv::Point2f> points0;
    std::vector<cv::Point2f> points1;

    if(markers != 1)
    {
        getValidpoints(points0, points1, fundamentalMatches);
    }
    if(markers != 0)
    {
        getValidpoints(points0, points1, stripe8_0);
        getValidpoints(points0, points1, stripe8_1);
        getValidpoints(points0, points1, stripe8_2);
        getValidpoints(points0, points1, stripe8_3);
        getValidpoints(points0, points1, stripe12_0);
        getValidpoints(points0, points1, stripe12_1);
        getValidpoints(points0, points1, stripe12_2);
        getValidpoints(points0, points1, stripe12_3);
        getValidpoints(points0, points1, anatomicalMarkers);
    }

    if(points0.size()!= 0 )
    {
        cv::undistortPoints( points0, undistortedMatches[0], intrinsic_LLRR, distCoeffs_LLRR);
        cv::undistortPoints( points1, undistortedMatches[1], intrinsic_LTRT, distCoeffs_LTRT);

		//undistortPoints normalizes the coordinates, which is corrected for below
		//(Which according to the OpenCV documentation is not necessary. Reason why it's still done: Note that according to forum "stackoverflow", there is a bug in the documentation: Undistorted points are indeed returned in normalized coordinates by undistortPoints.)
        for( int i = 0; i < undistortedMatches[0].size(); i++)
        {
            undistortedMatches[0][i].x = undistortedMatches[0][i].x * intrinsic_LLRR.at<double>(0,0) + intrinsic_LLRR.at<double>(0,2);
            undistortedMatches[0][i].y = undistortedMatches[0][i].y * intrinsic_LLRR.at<double>(1,1) + intrinsic_LLRR.at<double>(1,2);
        }
        for( int i = 0; i < undistortedMatches[1].size(); i++)
        {
            undistortedMatches[1][i].x = undistortedMatches[1][i].x * intrinsic_LTRT.at<double>(0,0) + intrinsic_LTRT.at<double>(0,2);
            undistortedMatches[1][i].y = undistortedMatches[1][i].y * intrinsic_LTRT.at<double>(1,1) + intrinsic_LTRT.at<double>(1,2);
        }

        if(undistortedMatches[0].size() < 15 ) //workaround because ransac is just called if number of matches >= 15
        {
            undistortedMatches[0].insert(undistortedMatches[0].begin(), undistortedMatches[0].begin(), undistortedMatches[0].end());
            undistortedMatches[1].insert(undistortedMatches[1].begin(), undistortedMatches[1].begin(), undistortedMatches[1].end());
        }
        //calculating fundamental matrix, essential matrix and camera matrices: see hartley and zisserman: multiple view geomentrie or bachelor thesis for details
        fundamental = cv::findFundamentalMat(undistortedMatches[0], undistortedMatches[1], method, 1.0, 1.0);
        
		int fundamental_checkv = 0;
			fundamental_checkv += abs(fundamental.at<double>(0,0)) + abs(fundamental.at<double>(0,1)) + abs(fundamental.at<double>(0,2));
			fundamental_checkv += abs(fundamental.at<double>(1,0)) + abs(fundamental.at<double>(1,1)) + abs(fundamental.at<double>(1,2));
			fundamental_checkv += abs(fundamental.at<double>(2,0)) + abs(fundamental.at<double>(2,1)) + abs(fundamental.at<double>(2,2));
		if (fundamental_checkv == 0) {
			std::cout << "####################################################" << std::endl;
			std::cout << "ERROR: Fundamental matrix contains only zeros." << std::endl;
			std::cout << "Please adjust your setup and run again -f again." << std::endl;
			std::cout << "####################################################" << std::endl;
			encounterederrors=encounterederrors+1;
		}
		else {
	
        essential = intrinsic_LTRT.t() * fundamental * intrinsic_LLRR;
        cv::SVD svd_essential(essential, cv::SVD::FULL_UV );
        cv::Mat W = (cv::Mat_<double>(3,3) << 0, -1, 0,
                     1,  0, 0,
                     0,  0, 1);
        cameraP0 = (cv::Mat_<double>(3,4) << 1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0);
        cv::Mat UWVT = svd_essential.u * W * svd_essential.vt;
        cv::Mat UWTVT = svd_essential.u * W.t() * svd_essential.vt;
        cv::Mat u3 = svd_essential.u.col(2);

        cameraP1.push_back( (cv::Mat_<double>(3,4) << UWVT.at<double>(0,0), UWVT.at<double>(0,1), UWVT.at<double>(0,2), u3.at<double>(0),
                             UWVT.at<double>(1,0), UWVT.at<double>(1,1), UWVT.at<double>(1,2), u3.at<double>(1),
                             UWVT.at<double>(2,0), UWVT.at<double>(2,1), UWVT.at<double>(2,2), u3.at<double>(2)) );

        cameraP1.push_back( (cv::Mat_<double>(3,4) << UWVT.at<double>(0,0), UWVT.at<double>(0,1), UWVT.at<double>(0,2), -u3.at<double>(0),
                             UWVT.at<double>(1,0), UWVT.at<double>(1,1), UWVT.at<double>(1,2), -u3.at<double>(1),
                             UWVT.at<double>(2,0), UWVT.at<double>(2,1), UWVT.at<double>(2,2), -u3.at<double>(2)) );

        cameraP1.push_back( (cv::Mat_<double>(3,4) << UWTVT.at<double>(0,0), UWTVT.at<double>(0,1), UWTVT.at<double>(0,2), u3.at<double>(0),
                             UWTVT.at<double>(1,0), UWTVT.at<double>(1,1), UWTVT.at<double>(1,2), u3.at<double>(1),
                             UWTVT.at<double>(2,0), UWTVT.at<double>(2,1), UWTVT.at<double>(2,2), u3.at<double>(2)) );

        cameraP1.push_back( (cv::Mat_<double>(3,4) << UWTVT.at<double>(0,0), UWTVT.at<double>(0,1), UWTVT.at<double>(0,2), -u3.at<double>(0),
                             UWTVT.at<double>(1,0), UWTVT.at<double>(1,1), UWTVT.at<double>(1,2), -u3.at<double>(1),
                             UWTVT.at<double>(2,0), UWTVT.at<double>(2,1), UWTVT.at<double>(2,2), -u3.at<double>(2)) );

        std::cout << "UWVT, u3" << std::endl;
        std::cout << UWVT.at<double>(0,0) << "  " << UWVT.at<double>(0,1) << "  " << UWVT.at<double>(0,2) << "  " << u3.at<double>(0) << std::endl
                  << UWVT.at<double>(1,0) << "  " << UWVT.at<double>(1,1) << "  " << UWVT.at<double>(1,2) << "  " << u3.at<double>(1)<< std::endl
                  << UWVT.at<double>(2,0) << "  " << UWVT.at<double>(2,1) << "  " << UWVT.at<double>(2,2) << "  " << u3.at<double>(2) << std::endl;

        std::cout << "UWVT, -u3" << std::endl;
        std::cout << UWVT.at<double>(0,0) << "  " << UWVT.at<double>(0,1) << "  " << UWVT.at<double>(0,2) << "  " << -u3.at<double>(0) << std::endl
                  << UWVT.at<double>(1,0) << "  " << UWVT.at<double>(1,1) << "  " << UWVT.at<double>(1,2) << "  " << -u3.at<double>(1)<< std::endl
                  << UWVT.at<double>(2,0) << "  " << UWVT.at<double>(2,1) << "  " << UWVT.at<double>(2,2) << "  " << -u3.at<double>(2) << std::endl;

        std::cout << "UWTVT, u3" << std::endl;
        std::cout << UWTVT.at<double>(0,0) << "  " << UWTVT.at<double>(0,1) << "  " << UWTVT.at<double>(0,2) << "  " << u3.at<double>(0) << std::endl
                  << UWTVT.at<double>(1,0) << "  " << UWTVT.at<double>(1,1) << "  " << UWTVT.at<double>(1,2) << "  " << u3.at<double>(1)<< std::endl
                  << UWTVT.at<double>(2,0) << "  " << UWTVT.at<double>(2,1) << "  " << UWTVT.at<double>(2,2) << "  " << u3.at<double>(2) << std::endl;

        std::cout << "UWTVT, -u3" << std::endl;
        std::cout << UWTVT.at<double>(0,0) << "  " << UWTVT.at<double>(0,1) << "  " << UWTVT.at<double>(0,2) << "  " << -u3.at<double>(0) << std::endl
                  << UWTVT.at<double>(1,0) << "  " << UWTVT.at<double>(1,1) << "  " << UWTVT.at<double>(1,2) << "  " << -u3.at<double>(1)<< std::endl
                  << UWTVT.at<double>(2,0) << "  " << UWTVT.at<double>(2,1) << "  " << UWTVT.at<double>(2,2) << "  " << -u3.at<double>(2) << std::endl;
        std::cout << std::endl;
        /*
       for(int i=0; i < undistortedMatches[0].size(); i++)
       {
           cv::Point3d p;
           p.x = undistortedMatches[0][i].x;
           p.y = undistortedMatches[0][i].y;
           p.z = 1;
           std::cout << cv::Mat(p).t() * fundamental * cv::Mat(p) << std::endl;
       }

       std::vector<std::vector<cv::Point2f> > test;
       test.push_back(std::vector<cv::Point2f>());
       test.push_back(std::vector<cv::Point2f>());
       markerfinder->distanceFilter( undistortedMatches[0], undistortedMatches[1], test[0], test[1], fundamental, 5);
       std::cout << undistortedMatches[0].size() << "  " << test[0].size() << std::endl;
*/
        drawImages();
        }
    }
}

void TwoView::triangulateElectrodes() //triangulate electrodepostion for all electrodes
{
    if( fundamental.total() != 0 )
    {
        triangulate(stripe8_0);
        triangulate(stripe8_1);
        triangulate(stripe8_2);
        triangulate(stripe8_3);
        triangulate(stripe12_0);
        triangulate(stripe12_1);
        triangulate(stripe12_2);
        triangulate(stripe12_3);
        triangulate(anatomicalMarkers);
    }
}

void TwoView::triangulateFundamentalmarkers() //triangulate fundamental points
{
    if( fundamental.total() != 0 )
    {
        triangulate(fundamentalMatches);
    }
}

void TwoView::triangulate(match *m) //actual triangulation
{
    //creating list of points to triangulate
    std::vector<std::vector<cv::Mat> > triangulatedPoints;
    for( int i=0; i<4; i++)
    {
        triangulatedPoints.push_back(std::vector<cv::Mat> ());
        triangulatedPoints[i].push_back(cv::Mat ());
        triangulatedPoints[i].push_back(cv::Mat ());
    }

    //removing radial and tangetial distortion
    std::vector<std::vector<cv::Point2f> > undistortedMatches;
    undistortedMatches.push_back(std::vector<cv::Point2f>());
    undistortedMatches.push_back(std::vector<cv::Point2f>());
    std::vector<cv::Point2f> points0;
    std::vector<cv::Point2f> points1;

    getValidpoints(points0, points1, m);

    if(points0.size()!= 0 )
    {
        cv::undistortPoints( points0, undistortedMatches[0], intrinsic_LLRR, distCoeffs_LLRR);
        cv::undistortPoints( points1, undistortedMatches[1], intrinsic_LTRT, distCoeffs_LTRT);

        for( int i = 0; i < undistortedMatches[0].size(); i++)
        {
            undistortedMatches[0][i].x = undistortedMatches[0][i].x * intrinsic_LLRR.at<double>(0,0) + intrinsic_LLRR.at<double>(0,2);
            undistortedMatches[0][i].y = undistortedMatches[0][i].y * intrinsic_LLRR.at<double>(1,1) + intrinsic_LLRR.at<double>(1,2);
        }
        for( int i = 0; i < undistortedMatches[1].size(); i++)
        {
            undistortedMatches[1][i].x = undistortedMatches[1][i].x * intrinsic_LTRT.at<double>(0,0) + intrinsic_LTRT.at<double>(0,2);
            undistortedMatches[1][i].y = undistortedMatches[1][i].y * intrinsic_LTRT.at<double>(1,1) + intrinsic_LTRT.at<double>(1,2);
        }
        std::cout << "Undistorted Matches0:(x,y)\n";
        for( int i= 0; i < undistortedMatches[0].size(); i++)
            std::cout << undistortedMatches[0][i].x << "   " << undistortedMatches[0][i].y << std::endl;


        std::vector<std::vector<cv::Point2f> > correctedMatches;
        correctedMatches.push_back(std::vector<cv::Point2f>());
        correctedMatches.push_back(std::vector<cv::Point2f>());
        cv::correctMatches(fundamental , undistortedMatches[0], undistortedMatches[1], correctedMatches[0], correctedMatches[1]);

        std::cout << "Corrected Matches0:(x,y)\n";
        for( int i= 0; i < correctedMatches[0].size(); i++)
            std::cout << correctedMatches[0][i].x << "   " << correctedMatches[0][i].y << std::endl;

        std::vector<std::vector<cv::Point2f> > normalizedMatches;
        normalizedMatches.push_back(std::vector<cv::Point2f>());
        normalizedMatches.push_back(std::vector<cv::Point2f>());
        std::cout << "\nNormalized Matches0:(x,y)\n";
        for( int i = 0; i < correctedMatches[0].size(); i++)
        {
            cv::Mat homogenousMatch = (cv::Mat_<double>(3,1) << correctedMatches[0][i].x, correctedMatches[0][i].y, 1.0);
            cv::Mat normalizedPoint = intrinsic_LLRR_inv * homogenousMatch;
            cv::Point2f nP( normalizedPoint.at<double>(0,0)/normalizedPoint.at<double>(2,0), normalizedPoint.at<double>(1,0)/normalizedPoint.at<double>(2,0));
            normalizedMatches[0].push_back( nP);
            std::cout << nP.x << "   " << nP.y << std::endl;
        }
        for( int i = 0; i < correctedMatches[1].size(); i++)
        {
            cv::Mat homogenousMatch = (cv::Mat_<double>(3,1) << correctedMatches[1][i].x, correctedMatches[1][i].y, 1.0);
            cv::Mat normalizedPoint = intrinsic_LTRT_inv * homogenousMatch;
            cv::Point2f nP( normalizedPoint.at<double>(0,0)/normalizedPoint.at<double>(2,0), normalizedPoint.at<double>(1,0)/normalizedPoint.at<double>(2,0));
            normalizedMatches[1].push_back( nP);
        }

        cv::Mat tmp_matches0;
        tmp_matches0.create(2, normalizedMatches[0].size(), CV_64F );
        for( int i = 0; i< normalizedMatches[0].size(); i++)
        {
            tmp_matches0.at<double>(0, i) = normalizedMatches[0][i].x;
            tmp_matches0.at<double>(1, i) = normalizedMatches[0][i].y;
        }

        cv::Mat tmp_matches1;
        tmp_matches1.create(2, normalizedMatches[1].size(), CV_64F );
        for( int i = 0; i< normalizedMatches[1].size(); i++)
        {
            tmp_matches1.at<double>(0, i) = normalizedMatches[1][i].x;
            tmp_matches1.at<double>(1, i) = normalizedMatches[1][i].y;
        }

        int bestCameraP1 = 0;
        int lowestWrongPoints = 30000;
        int wrongPoints = 0;
		int wrongPointsArray[4] = { 0 };

        for( int i = 0; i < 4; i++)
        {
            cv::Mat rotation, translation, cameraMatrix;
            cv::decomposeProjectionMatrix( cameraP1[i], cameraMatrix, rotation, translation);
            cv::Mat transformation = (cv::Mat_<double>(4,4) << rotation.at<double>(0,0), rotation.at<double>(0,1), rotation.at<double>(0,2),translation.at<double>(0,0)/translation.at<double>(3,0),
                                      rotation.at<double>(1,0), rotation.at<double>(1,1), rotation.at<double>(1,2), translation.at<double>(1,0)/translation.at<double>(3,0),
                                      rotation.at<double>(2,0), rotation.at<double>(2,1), rotation.at<double>(2,2), translation.at<double>(2,0)/translation.at<double>(3,0),
                                      0, 0, 0, 1);
            wrongPoints = 0;
            cv::triangulatePoints(cameraP0, cameraP1[i], tmp_matches0, tmp_matches1, triangulatedPoints[i][0]);
            triangulatedPoints[i][1] =  transformation * triangulatedPoints[i][0];
            for( int j = 0; j < triangulatedPoints[i][0].size().width; j++)
            {
                if( triangulatedPoints[i][0].at<double>(2,j)/triangulatedPoints[i][0].at<double>(3,j) < 0 )
                    wrongPoints++;
            }
            for( int j = 0; j < triangulatedPoints[i][1].size().width; j++)
            {
                if( (triangulatedPoints[i][1].at<double>(2,j)/triangulatedPoints[i][1].at<double>(3,j)) < 0 )
                {
                    wrongPoints++;
                }
            }
            if( lowestWrongPoints > wrongPoints)
            {
                bestCameraP1 = i;
                lowestWrongPoints = wrongPoints;
            }
            std::cout << "Camera P0 and P1[" << i << "] Wrong Points: " << wrongPoints << std::endl;
            wrongPointsArray[i]=wrongPoints;
        }

		for (int i = 0; i < 4; i++)
		{
			if ( wrongPointsArray[i]==lowestWrongPoints && bestCameraP1!=i )
				{
					std::cout << "ERROR in "<< m->name << ": Multiple best solutions for camera matrix P1 (which is theoretically impossible according to hartley04, Sect. 9.6.3, so maybe due to numerics?). Choice made by algorithm may be wrong. Please check results for quality. If inconsistencies exist, adjust setup by addding/removing image points taken into consideration." << std::endl;
					encounterederrors=encounterederrors+1;
				}
		}

        std::cout << std::endl << "Triangulated Points P0: ";
        for( int i = 0; i < triangulatedPoints[bestCameraP1][0].size().width; i++)
            std::cout << std::endl << triangulatedPoints[bestCameraP1][0].at<double>(0,i)/triangulatedPoints[bestCameraP1][0].at<double>(3,i) << "  " << triangulatedPoints[bestCameraP1][0].at<double>(1,i)/triangulatedPoints[bestCameraP1][0].at<double>(3,i) << "  "<< triangulatedPoints[bestCameraP1][0].at<double>(2,i)/triangulatedPoints[bestCameraP1][0].at<double>(3,i);
        std::cout << std::endl << "Triangulated Points P1: ";
        for( int i = 0; i < triangulatedPoints[bestCameraP1][1].size().width; i++)
            std::cout << std::endl << triangulatedPoints[bestCameraP1][1].at<double>(0,i)/triangulatedPoints[bestCameraP1][1].at<double>(3,i) << "  " << triangulatedPoints[bestCameraP1][1].at<double>(1,i)/triangulatedPoints[bestCameraP1][1].at<double>(3,i) << "  "<< triangulatedPoints[bestCameraP1][1].at<double>(2,i)/triangulatedPoints[bestCameraP1][1].at<double>(3,i);
        std::cout << std::endl << std::endl;

        setValid3dpoints(triangulatedPoints[bestCameraP1][0], triangulatedPoints[bestCameraP1][1], m);
    }
}

void TwoView::writeMatlabfile()
{
    std::vector<cv::Point3f> points0;
    std::vector<cv::Point3f> points1;
    std::fstream f;
    f.open( (path + save + ".m").c_str(), std::ios::out);
    f << "hold on\n view(0, -90)\nxlabel('x'), ylabel('y'), zlabel('z')\naxis auto, box on\n";

    getValid3dpoints(points0, points1, stripe8_0);
    if( points0.size() > 0 )
    {
        f << "x8_0=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].x << ", ";
        }
        f << points0[points0.size()-1].x << "]\n";
        f << "y8_0=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].y << ", ";
        }
        f << points0[points0.size()-1].y << "]\n";
        f << "z8_0=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].z << ", ";
        }
        f << points0[points0.size()-1].z << "]\n";
        f << "plot3(x8_0,y8_0,z8_0, '.-', 'MarkerEdgeColor',[0.5 0.5 0],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0])\n";
    }

    points0.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe8_1);
    if( points0.size() > 0 )
    {
        f << "x8_1=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].x << ", ";
        }
        f << points0[points0.size()-1].x << "]\n";
        f << "y8_1=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].y << ", ";
        }
        f << points0[points0.size()-1].y << "]\n";
        f << "z8_1=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].z << ", ";
        }
        f << points0[points0.size()-1].z << "]\n";
        f << "plot3(x8_1,y8_1,z8_1, '.-', 'MarkerEdgeColor',[0 0 0.5],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    }

    points0.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe8_2);
    if( points0.size() > 0 )
    {
        f << "x8_2=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].x << ", ";
        }
        f << points0[points0.size()-1].x << "]\n";
        f << "y8_2=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].y << ", ";
        }
        f << points0[points0.size()-1].y << "]\n";
        f << "z8_2=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].z << ", ";
        }
        f << points0[points0.size()-1].z << "]\n";
        f << "plot3(x8_2,y8_2,z8_2, '.-', 'MarkerEdgeColor',[0 1 1],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    }

    points0.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe8_3);
    if( points0.size() > 0 )
    {
        f << "x8_3=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].x << ", ";
        }
        f << points0[points0.size()-1].x << "]\n";
        f << "y8_3=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].y << ", ";
        }
        f << points0[points0.size()-1].y << "]\n";
        f << "z8_3=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].z << ", ";
        }
        f << points0[points0.size()-1].z << "]\n";
        f << "plot3(x8_3,y8_3,z8_3, '.-', 'MarkerEdgeColor',[1 1 0],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    }

    points0.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe12_0);
    if( points0.size() > 0 )
    {
        f << "x12_0=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].x << ", ";
        }
        f << points0[points0.size()-1].x << "]\n";
        f << "y12_0=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].y << ", ";
        }
        f << points0[points0.size()-1].y << "]\n";
        f << "z12_0=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].z << ", ";
        }
        f << points0[points0.size()-1].z << "]\n";
        f << "plot3(x12_0,y12_0,z12_0, '.-', 'MarkerEdgeColor',[1 0 1],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    }

    points0.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe12_1);
    if( points0.size() > 0 )
    {
        f << "x12_1=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].x << ", ";
        }
        f << points0[points0.size()-1].x << "]\n";
        f << "y12_1=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].y << ", ";
        }
        f << points0[points0.size()-1].y << "]\n";
        f << "z12_1=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].z << ", ";
        }
        f << points0[points0.size()-1].z << "]\n";
        f << "plot3(x12_1,y12_1,z12_1, '.-', 'MarkerEdgeColor',[0 1 0],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    }

    points0.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe12_2);
    if( points0.size() > 0 )
    {
        f << "x12_2=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].x << ", ";
        }
        f << points0[points0.size()-1].x << "]\n";
        f << "y12_2=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].y << ", ";
        }
        f << points0[points0.size()-1].y << "]\n";
        f << "z12_2=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].z << ", ";
        }
        f << points0[points0.size()-1].z << "]\n";
        f << "plot3(x12_2,y12_2,z12_2, '.-', 'MarkerEdgeColor',[1 0 0],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    }

    points0.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe12_3);
    if( points0.size() > 0 )
    {
        f << "x12_3=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].x << ", ";
        }
        f << points0[points0.size()-1].x << "]\n";
        f << "y12_3=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].y << ", ";
        }
        f << points0[points0.size()-1].y << "]\n";
        f << "z12_3=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].z << ", ";
        }
        f << points0[points0.size()-1].z << "]\n";
        f << "plot3(x12_3,y12_3,z12_3, '.-', 'MarkerEdgeColor',[0 0 1],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    }
    
    points0.clear();
    points1.clear();
    getValid3dpoints(points0, points1, anatomicalMarkers);
    if( points0.size() > 0 )
    {
        f << "xanatomicalMarkers=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].x << ", ";
        }
        f << points0[points0.size()-1].x << "]\n";
        f << "yanatomicalMarkers=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].y << ", ";
        }
        f << points0[points0.size()-1].y << "]\n";
        f << "zanatomicalMarkers=[";
        for( int i = 0; i < points0.size()-1; i++)
        {
            f << points0[i].z << ", ";
        }
        f << points0[points0.size()-1].z << "]\n";
        f << "plot3(xanatomicalMarkers,yanatomicalMarkers,zanatomicalMarkers, '.-', 'MarkerEdgeColor',[0 0 1],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    }
    f.close();

/*
    f.open( (path + "points3dP1.m").c_str(), std::ios::out);
    f << "hold on\n view(0, -90)\nxlabel('x'), ylabel('y'), zlabel('z')\naxis image, box on\n";

    getValid3dpoints(points0, points1, stripe8_0);
    f << "x8_0=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].x << ", ";
    }
    f << points1[points1.size()-1].x << "]\n";
    f << "y8_0=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].y << ", ";
    }
    f << points1[points1.size()-1].y << "]\n";
    f << "z8_0=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].z << ", ";
    }
    f << points1[points1.size()-1].z << "]\n";

    points1.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe8_1);
    f << "x8_1=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].x << ", ";
    }
    f << points1[points1.size()-1].x << "]\n";
    f << "y8_1=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].y << ", ";
    }
    f << points1[points1.size()-1].y << "]\n";
    f << "z8_1=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].z << ", ";
    }
    f << points1[points1.size()-1].z << "]\n";

    points1.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe8_2);
    f << "x8_2=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].x << ", ";
    }
    f << points1[points1.size()-1].x << "]\n";
    f << "y8_2=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].y << ", ";
    }
    f << points1[points1.size()-1].y << "]\n";
    f << "z8_2=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].z << ", ";
    }
    f << points1[points1.size()-1].z << "]\n";

    points1.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe8_3);
    f << "x8_3=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].x << ", ";
    }
    f << points1[points1.size()-1].x << "]\n";
    f << "y8_3=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].y << ", ";
    }
    f << points1[points1.size()-1].y << "]\n";
    f << "z8_3=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].z << ", ";
    }
    f << points1[points1.size()-1].z << "]\n";

    points1.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe12_0);
    f << "x12_0=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].x << ", ";
    }
    f << points1[points1.size()-1].x << "]\n";
    f << "y12_0=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].y << ", ";
    }
    f << points1[points1.size()-1].y << "]\n";
    f << "z12_0=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].z << ", ";
    }
    f << points1[points1.size()-1].z << "]\n";

    points1.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe12_1);
    f << "x12_1=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].x << ", ";
    }
    f << points1[points1.size()-1].x << "]\n";
    f << "y12_1=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].y << ", ";
    }
    f << points1[points1.size()-1].y << "]\n";
    f << "z12_1=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].z << ", ";
    }
    f << points1[points1.size()-1].z << "]\n";

    points1.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe12_2);
    f << "x12_2=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].x << ", ";
    }
    f << points1[points1.size()-1].x << "]\n";
    f << "y12_2=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].y << ", ";
    }
    f << points1[points1.size()-1].y << "]\n";
    f << "z12_2=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].z << ", ";
    }
    f << points1[points1.size()-1].z << "]\n";

    points1.clear();
    points1.clear();
    getValid3dpoints(points0, points1, stripe12_3);
    f << "x12_3=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].x << ", ";
    }
    f << points1[points1.size()-1].x << "]\n";
    f << "y12_3=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].y << ", ";
    }
    f << points1[points1.size()-1].y << "]\n";
    f << "z12_3=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].z << ", ";
    }
    f << points1[points1.size()-1].z << "]\n";
    
    points1.clear();
    points1.clear();
    getValid3dpoints(points0, points1, anatomicalMarkers);
    f << "xanatomicalMarkers=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].x << ", ";
    }
    f << points1[points1.size()-1].x << "]\n";
    f << "yanatomicalMarkers=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].y << ", ";
    }
    f << points1[points1.size()-1].y << "]\n";
    f << "zanatomicalMarkers=[";
    for( int i = 0; i < points1.size()-1; i++)
    {
        f << points1[i].z << ", ";
    }
    f << points1[points1.size()-1].z << "]\n";

    f << "plot3(x8_0,y8_0,z8_0, '.-', 'MarkerEdgeColor',[0.5 0.5 0],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0])\n";
    f << "plot3(x8_1,y8_1,z8_1, '.-', 'MarkerEdgeColor',[0 0 0.5],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    f << "plot3(x8_2,y8_2,z8_2, '.-', 'MarkerEdgeColor',[0 1 1],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    f << "plot3(x8_3,y8_3,z8_3, '.-', 'MarkerEdgeColor',[1 1 0],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    f << "plot3(x12_0,y12_0,z12_0, '.-', 'MarkerEdgeColor',[1 0 1],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    f << "plot3(x12_1,y12_1,z12_1, '.-', 'MarkerEdgeColor',[0 1 0],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    f << "plot3(x12_2,y12_2,z12_2, '.-', 'MarkerEdgeColor',[1 0 0],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    f << "plot3(x12_3,y12_3,z12_3, '.-', 'MarkerEdgeColor',[0 0 1],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";
    f << "plot3(xanatomicalMarkers,yanatomicalMarkers,zanatomicalMarkers, '.-', 'MarkerEdgeColor',[0 0 1],'MarkerSize',25,'LineWidth',1,'Color',[0 0 0 ])\n";

    f.close();
*/
}

void TwoView::writeElectrodefile() //see ibt-wiki for details
{
    std::fstream f;
    int ctr = 0;
    f.open( (path + save + ".esf").c_str(), std::ios::out);

    f << "# ElectrodeSet\n";
    f << stripe8_0->validsize() + stripe8_1->validsize() + stripe8_2->validsize() + stripe8_3->validsize() +
         stripe12_0->validsize() + stripe12_1->validsize() + stripe12_2->validsize() + stripe12_3->validsize() + anatomicalMarkers->validsize();
    f << "\n1 0 0 0\n";
    f << "0 1 0 0\n";
    f << "0 0 1 0\n";
    f << "0 0 0 1\n";

    for(int i=0; i < stripe8_0->center2d_0.size();i++)
    {
        if( (stripe8_0->center3d_0[i].x != -1)&&(stripe8_0->center3d_0[i].y != -1)&&(stripe8_0->center3d_0[i].z != -1) ) //default value. also, z component cannot be negative (otherwise behind camera)
        {
            f << stripe8_0->center3d_0[i].x << " "
              << stripe8_0->center3d_0[i].y << " "
              << stripe8_0->center3d_0[i].z << " "
              << "8 "
              << ctr << " "
              << "A" << i+1 << "\n";
            ctr++;
        }
    }

    for(int i=0; i < stripe8_1->center2d_0.size();i++)
    {
        if( (stripe8_1->center3d_0[i].x != -1)&&(stripe8_1->center3d_0[i].y != -1)&&(stripe8_1->center3d_0[i].z != -1) ) //default value. also, z component cannot be negative (otherwise behind camera)
        {
            f << stripe8_1->center3d_0[i].x << " "
              << stripe8_1->center3d_0[i].y << " "
              << stripe8_1->center3d_0[i].z << " "
              << "8 "
              << ctr << " "
              << "A" << 8 + i+1 << "\n"; //stripe8_1 starts with A9
            ctr++;
        }
    }

    for(int i=0; i < stripe8_2->center2d_0.size();i++)
    {
        if( (stripe8_2->center3d_0[i].x != -1)&&(stripe8_2->center3d_0[i].y != -1)&&(stripe8_2->center3d_0[i].z != -1) ) //default value. also, z component cannot be negative (otherwise behind camera)
        {
            f << stripe8_2->center3d_0[i].x << " "
              << stripe8_2->center3d_0[i].y << " "
              << stripe8_2->center3d_0[i].z << " "
              << "8 "
              << ctr << " "
              << "A" << 16 + i+1 << "\n";//stripe8_2 starts with A17
            ctr++;
        }
    }

    for(int i=0; i < stripe8_3->center2d_0.size();i++)
    {
        if( (stripe8_3->center3d_0[i].x != -1)&&(stripe8_3->center3d_0[i].y != -1)&&(stripe8_3->center3d_0[i].z != -1) ) //default value. also, z component cannot be negative (otherwise behind camera)
        {
            f << stripe8_3->center3d_0[i].x << " "
              << stripe8_3->center3d_0[i].y << " "
              << stripe8_3->center3d_0[i].z << " "
              << "8 "
              << ctr << " "
              << "A" << 24 + i+1 << "\n";//stripe8_3 starts with A25
            ctr++;
        }
    }

    for(int i=0; i < stripe12_0->center2d_0.size();i++)
    {
        if( (stripe12_0->center3d_0[i].x != -1)&&(stripe12_0->center3d_0[i].y != -1)&&(stripe12_0->center3d_0[i].z != -1) ) //default value. also, z component cannot be negative (otherwise behind camera)
        {
            f << stripe12_0->center3d_0[i].x << " "
              << stripe12_0->center3d_0[i].y << " "
              << stripe12_0->center3d_0[i].z << " "
              << "8 "
              << ctr << " "
              << "B" << i+1 << "\n";
            ctr++;
        }
    }

    for(int i=0; i < stripe12_1->center2d_0.size();i++)
    {
        if( (stripe12_1->center3d_0[i].x != -1)&&(stripe12_1->center3d_0[i].y != -1)&&(stripe12_1->center3d_0[i].z != -1) ) //default value. also, z component cannot be negative (otherwise behind camera)
        {
            f << stripe12_1->center3d_0[i].x << " "
              << stripe12_1->center3d_0[i].y << " "
              << stripe12_1->center3d_0[i].z << " "
              << "8 "
              << ctr << " "
              << "B" << 12 + i+1 << "\n";//stripe12_1 starts with B13
            ctr++;
        }
    }

    for(int i=0; i < stripe12_2->center2d_0.size();i++)
    {
        if( (stripe12_2->center3d_0[i].x != -1)&&(stripe12_2->center3d_0[i].y != -1)&&(stripe12_2->center3d_0[i].z != -1) ) //default value. also, z component cannot be negative (otherwise behind camera)
        {
            f << stripe12_2->center3d_0[i].x << " "
              << stripe12_2->center3d_0[i].y << " "
              << stripe12_2->center3d_0[i].z << " "
              << "8 "
              << ctr << " "
              << "C" << i+1 << "\n";
            ctr++;
        }
    }

    for(int i=0; i < stripe12_3->center2d_0.size();i++)
    {
        if( (stripe12_3->center3d_0[i].x != -1)&&(stripe12_3->center3d_0[i].y != -1)&&(stripe12_3->center3d_0[i].z != -1) ) //default value. also, z component cannot be negative (otherwise behind camera)
        {
            f << stripe12_3->center3d_0[i].x << " "
              << stripe12_3->center3d_0[i].y << " "
              << stripe12_3->center3d_0[i].z << " "
              << "8 "
              << ctr << " "
              << "C" << 12 + i+1 << "\n";//stripe12_3 starts with C13
            ctr++;
        }
    }

    for(int i=0; i < anatomicalMarkers->center2d_0.size();i++)
    {
        if( (anatomicalMarkers->center3d_0[i].x != -1)&&(anatomicalMarkers->center3d_0[i].y != -1)&&(anatomicalMarkers->center3d_0[i].z != -1) ) //default value. also, z component cannot be negative (otherwise behind camera)
        {
            f << anatomicalMarkers->center3d_0[i].x << " "
              << anatomicalMarkers->center3d_0[i].y << " "
              << anatomicalMarkers->center3d_0[i].z << " "
              << "8 "
              << ctr << " "
              << anatomicalMarkers->description[i] << "\n";
            ctr++;
        }
    }
    
    f.close();
}

int TwoView::calculateDistances( std::vector< std::vector<int> > idlist) //for experimental purposes
{
    double min = 30000;
    double max = 0;
    for(int idlistctr = 0; idlistctr < idlist.size(); idlistctr++)
    {
        if(idlist[idlistctr].size() != 2 )
        {
            continue;
        }
        for(int i=0; i < getPointstorage()->id.size(); i++)
        {
            if( getPointstorage()->id[i] == idlist[idlistctr][0])
            {
                for(int j=0; j < getPointstorage()->id.size(); j++)
                {
                    if( getPointstorage()->id[j] == idlist[idlistctr][1])
                    {
                        double distancesquared = 0;
                        distancesquared += (getPointstorage()->center3d_0[i].x - getPointstorage()->center3d_0[j].x) * (getPointstorage()->center3d_0[i].x - getPointstorage()->center3d_0[j].x);
                        distancesquared += (getPointstorage()->center3d_0[i].y - getPointstorage()->center3d_0[j].y) * (getPointstorage()->center3d_0[i].y - getPointstorage()->center3d_0[j].y);
                        distancesquared += (getPointstorage()->center3d_0[i].z - getPointstorage()->center3d_0[j].z) * (getPointstorage()->center3d_0[i].z - getPointstorage()->center3d_0[j].z);
                        if( std::sqrt( distancesquared) < min )
                        {
                            min = std::sqrt( distancesquared);
                        }
                        if( std::sqrt( distancesquared) > max )
                        {
                            max = std::sqrt( distancesquared);
                        }
                        std::cout << "Distance [ " << getPointstorage()->id[i] << " <-> "
                                  << getPointstorage()->id[j] << "]  "
                                  << std::sqrt( distancesquared) <<std::endl;
                    }
                }
            }
        }
    }
    std::cout << "Min: " << min << "  Max: " << max << " Max. Error: " << (max-min)/min*100 << "%" << std::endl;
    return 0;
}

void TwoView::calculateAngles( std::vector< std::vector<int> > idlist) //for experimental purposes
{
    int pos[4] = {-1,-1,-1,-1};
    for(int idlistctr = 0; idlistctr < idlist.size(); idlistctr++)
    {
        if(idlist[idlistctr].size() != 4 )
        {
            continue;
        }
        for(int i=0; i < getPointstorage()->id.size(); i++)
        {
            for( int j = 0; j < 4; j++)
            {
                if( getPointstorage()->id[i] == idlist[idlistctr][j])
                {
                    pos[j] = i;
                }
            }
        }
        if( pos[0] != -1 &&  pos[1] != -1 && pos[2] != -1 && pos[3] != -1)
        {
            cv::Point3f v1 = cv::Point3f( getPointstorage()->center3d_0[pos[1]].x - getPointstorage()->center3d_0[pos[0]].x,
                                          getPointstorage()->center3d_0[pos[1]].y - getPointstorage()->center3d_0[pos[0]].y,
                                          getPointstorage()->center3d_0[pos[1]].z - getPointstorage()->center3d_0[pos[0]].z);
            cv::Point3f v2 = cv::Point3f( getPointstorage()->center3d_0[pos[3]].x - getPointstorage()->center3d_0[pos[2]].x,
                                          getPointstorage()->center3d_0[pos[3]].y - getPointstorage()->center3d_0[pos[2]].y,
                                          getPointstorage()->center3d_0[pos[3]].z - getPointstorage()->center3d_0[pos[2]].z);
            double angle = 180 / 3.14159265 *
                           std::acos( (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z) /
                                      std::sqrt( (v1.x*v1.x + v1.y*v1.y + v1.z*v1.z) * (v2.x*v2.x + v2.y*v2.y + v2.z*v2.z) ) );
            std::cout << "Angle [ " << getPointstorage()->id[pos[0]] << "," << getPointstorage()->id[pos[1]]
                      << " ] [ " << getPointstorage()->id[pos[2]] << "," << getPointstorage()->id[pos[3]] << " ]  " << angle << std::endl;
        }
        pos[0] = pos[1] = pos[2] = pos[3] = -1;
    }
}

void TwoView::setSize(int size) //set size for font and points
{
    if( size == 0)
    {
        circleRadius = 3;
        fontScale = 2;
    }
    else
    {
        circleRadius = 6;
        fontScale = 4;
    }
    drawImages();
}

match* TwoView::getPointstorage()
{
    if(pointStorage == 0)
        return stripe8_0;
    else if(pointStorage == 1)
        return stripe8_1;
    else if(pointStorage == 2)
        return stripe8_2;
    else if(pointStorage == 3)
        return stripe8_3;
    else if(pointStorage == 4)
        return stripe12_0;
    else if(pointStorage == 5)
        return stripe12_1;
    else if(pointStorage == 6)
        return stripe12_2;
    else if(pointStorage == 7)
        return stripe12_3;
    else if(pointStorage == 8)
        return anatomicalMarkers;
    else if(pointStorage == 9)
        return fundamentalMatches;
    else
	return NULL;
}

void TwoView::setPointstorage( int storage)
{
    pointStorage = storage;
    SubimgPos = 0;
    nextSubimgPos = 0;
}

void TwoView::setSavefile(std::string save_)
{
    save = save_;
}

//load and save matches: not used right now
void TwoView::saveMatches()
{
    std::cout << "Name of points to save: ";
    std::string s;
    std::getline(std::cin, s);
    cv::FileStorage fs(path + "points" + s + ".xml", cv::FileStorage::WRITE);

    cv::Mat tmp_matches0;
    tmp_matches0.create(2, getPointstorage()->center2d_0.size(), CV_64F );
    for( int i = 0; i < getPointstorage()->center2d_0.size(); i++)
    {
        tmp_matches0.at<double>(0, i) = getPointstorage()->center2d_0[i].x;
        tmp_matches0.at<double>(1, i) = getPointstorage()->center2d_0[i].y;
    }
    fs << "matches0" << tmp_matches0;

    cv::Mat tmp_matches1;
    tmp_matches1.create(2, getPointstorage()->center2d_1.size(), CV_64F );
    for( int i = 0; i < getPointstorage()->center2d_1.size(); i++)
    {
        tmp_matches1.at<double>(0, i) = getPointstorage()->center2d_1[i].x;
        tmp_matches1.at<double>(1, i) = getPointstorage()->center2d_1[i].y;
    }
    fs << "matches1" << tmp_matches1;

    cv::Mat tmp_id;
    tmp_id.create(1, getPointstorage()->id.size(), CV_64F );
    for( int i = 0; i < getPointstorage()->id.size(); i++)
    {
        tmp_id.at<double>(0, i) = getPointstorage()->id[i];
    }
    fs << "id" << tmp_id;

    fs << "descriptions" << "[";
    for( int i=0; i < getPointstorage()->description.size(); i++)
        fs << getPointstorage()->description[i];
    fs << "]";
}

void TwoView::loadMatches()
{
    getPointstorage()->clear();

    std::cout << "NAME of points to load (opens file pointsNAME.xml): ";
    std::string s;
    std::getline(std::cin, s);
    cv::FileStorage fs(path + "points" + s + ".xml", cv::FileStorage::READ);

    cv::Mat tmp_matches0;
    fs["matches0"] >> tmp_matches0;
    for( int i = 0; i < tmp_matches0.size().width; i++)
    {
        getPointstorage()->center2d_0.push_back( cv::Point2f( tmp_matches0.at<double>(0,i), tmp_matches0.at<double>(1,i)));
        getPointstorage()->center3d_0.push_back( cv::Point3f( -1, -1, -1));
    }

    cv::Mat tmp_matches1;
    fs["matches1"] >> tmp_matches1;
    for( int i = 0; i < tmp_matches1.size().width; i++)
    {
        getPointstorage()->center2d_1.push_back( cv::Point2f( tmp_matches1.at<double>(0,i), tmp_matches1.at<double>(1,i)));
        getPointstorage()->center3d_1.push_back( cv::Point3f( -1, -1, -1));
    }

    cv::Mat tmp_id;
    fs["id"] >> tmp_id;
    for( int i = 0; i < tmp_id.size().width; i++)
        getPointstorage()->id.push_back( tmp_id.at<double>(0,i));

    cv::FileNode fn = fs["descriptions"];
    if ( fn.type() == cv::FileNode::SEQ )
    {
        cv::FileNodeIterator i = fn.begin(), i_end = fn.end();
        for (; i != i_end; ++i)
            getPointstorage()->description.push_back( *i);
    }

    drawImages();
}

void TwoView::drawImages() //for every stripe and fundamental points: draw all valid points to images
{
    drawn_img0 = orig_img0.clone();
    drawn_img1 = orig_img1.clone();

    if( drawEpi != 0 )
        drawEpilines();

    if( !fundamentalMatches->center2d_0.empty())
    {
        for(unsigned int i = 0; i < fundamentalMatches->center2d_0.size(); i++)
        {
            cv::circle(drawn_img0, fundamentalMatches->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img0, fundamentalMatches->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img0, fundamentalMatches->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img1, fundamentalMatches->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img1, fundamentalMatches->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img1, fundamentalMatches->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
        }
    }

    if( !anatomicalMarkers->center2d_0.empty())
    {
        for(unsigned int i = 0; i < anatomicalMarkers->center2d_0.size(); i++)
        {
            cv::circle(drawn_img0, anatomicalMarkers->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img0, anatomicalMarkers->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img0, anatomicalMarkers->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img1, anatomicalMarkers->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img1, anatomicalMarkers->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
            cv::circle(drawn_img1, anatomicalMarkers->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
        }
    }

    if( !stripe8_0->center2d_0.empty())
    {
        for(unsigned int i = 0; i < stripe8_0->center2d_0.size(); i++)
        {
            if( stripe8_0->center2d_0[i].x >= 0 && stripe8_0->center2d_1[i].x >= 0 )
            {
                cv::circle(drawn_img0, stripe8_0->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe8_0->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe8_0->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_0->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_0->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_0->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            }
        }
    }

    if( !stripe8_1->center2d_0.empty())
    {
        for(unsigned int i = 0; i < stripe8_1->center2d_0.size(); i++)
        {
            if( stripe8_1->center2d_0[i].x >= 0 && stripe8_1->center2d_1[i].x >= 0 )
            {
                cv::circle(drawn_img0, stripe8_1->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe8_1->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe8_1->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_1->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_1->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_1->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            }
        }
    }

    if( !stripe8_2->center2d_0.empty())
    {
        for(unsigned int i = 0; i < stripe8_2->center2d_0.size(); i++)
        {
            if( stripe8_2->center2d_0[i].x >= 0 && stripe8_2->center2d_1[i].x >= 0 )
            {
                cv::circle(drawn_img0, stripe8_2->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe8_2->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe8_2->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_2->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_2->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_2->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            }
        }
    }

    if( !stripe8_3->center2d_0.empty())
    {
        for(unsigned int i = 0; i < stripe8_3->center2d_0.size(); i++)
        {
            if( stripe8_3->center2d_0[i].x >= 0 && stripe8_3->center2d_1[i].x >= 0 )
            {
                cv::circle(drawn_img0, stripe8_3->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe8_3->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe8_3->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_3->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_3->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe8_3->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            }
        }
    }

    if( !stripe12_0->center2d_0.empty())
    {
        for(unsigned int i = 0; i < stripe12_0->center2d_0.size(); i++)
        {
            if( stripe12_0->center2d_0[i].x >= 0 && stripe12_0->center2d_1[i].x >= 0 )
            {
                cv::circle(drawn_img0, stripe12_0->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe12_0->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe12_0->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_0->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_0->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_0->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            }
        }
    }

    if( !stripe12_1->center2d_0.empty())
    {
        for(unsigned int i = 0; i < stripe12_1->center2d_0.size(); i++)
        {
            if( stripe12_1->center2d_0[i].x >= 0 && stripe12_1->center2d_1[i].x >= 0 )
            {
                cv::circle(drawn_img0, stripe12_1->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe12_1->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe12_1->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_1->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_1->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_1->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            }
        }
    }

    if( !stripe12_2->center2d_0.empty())
    {
        for(unsigned int i = 0; i < stripe12_2->center2d_0.size(); i++)
        {
            if( stripe12_2->center2d_0[i].x >= 0 && stripe12_2->center2d_1[i].x >= 0 )
            {
                cv::circle(drawn_img0, stripe12_2->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe12_2->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe12_2->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_2->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_2->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_2->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            }
        }
    }

    if( !stripe12_3->center2d_0.empty())
    {
        for(unsigned int i = 0; i < stripe12_3->center2d_0.size(); i++)
        {
            if( stripe12_3->center2d_0[i].x >= 0 && stripe12_3->center2d_1[i].x >= 0 )
            {
                cv::circle(drawn_img0, stripe12_3->center2d_0[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe12_3->center2d_0[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img0, stripe12_3->center2d_0[i], circleRadius+2, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_3->center2d_1[i], circleRadius, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_3->center2d_1[i], circleRadius+1, cv::Scalar(255,255,0), 1);
                cv::circle(drawn_img1, stripe12_3->center2d_1[i], circleRadius+2, cv::Scalar(255,255,0), 1);
            }
        }
    }

    if(newPoint[0] != cv::Point2f(-1, -1))
        cv::circle(drawn_img0, newPoint[0], circleRadius, cv::Scalar(0,0,255), 1);
        cv::circle(drawn_img0, newPoint[0], circleRadius+2, cv::Scalar(0,0,255), 1);
    if(newPoint[1] != cv::Point2f(-1, -1))
        cv::circle(drawn_img1, newPoint[1], circleRadius, cv::Scalar(0,0,255), 1);
        cv::circle(drawn_img1, newPoint[1], circleRadius+2, cv::Scalar(0,0,255), 1);
}

void TwoView::drawEpilines() // drawing epipolar lines: undistort points first, draw undistorted points and epipolar linse
{
    if( fundamental.total() != 0 )
    {
        std::vector<cv::Vec3f> lines0;
        std::vector<cv::Vec3f> lines1;

        std::vector<cv::Point2f> points0;
        std::vector<cv::Point2f> points1;
        getValidpoints(points0, points1, stripe8_0);
        getValidpoints(points0, points1, stripe8_1);
        getValidpoints(points0, points1, stripe8_2);
        getValidpoints(points0, points1, stripe8_3);
        getValidpoints(points0, points1, stripe12_0);
        getValidpoints(points0, points1, stripe12_1);
        getValidpoints(points0, points1, stripe12_2);
        getValidpoints(points0, points1, stripe12_3);
        getValidpoints(points0, points1, anatomicalMarkers);

        if( !points0.empty())
        {
            std::vector<cv::Point2f> undistortedpoints0;
            std::vector<cv::Point2f> undistortedpoints1;
            cv::undistortPoints( points0, undistortedpoints0, intrinsic_LLRR, distCoeffs_LLRR);
            cv::undistortPoints( points1, undistortedpoints1, intrinsic_LTRT, distCoeffs_LTRT);

            for( int i = 0; i < undistortedpoints0.size(); i++)
            {
                undistortedpoints0[i].x = undistortedpoints0[i].x * intrinsic_LLRR.at<double>(0,0) + intrinsic_LLRR.at<double>(0,2);
                undistortedpoints0[i].y = undistortedpoints0[i].y * intrinsic_LLRR.at<double>(1,1) + intrinsic_LLRR.at<double>(1,2);
            }
            for( int i = 0; i < undistortedpoints1.size(); i++)
            {
                undistortedpoints1[i].x = undistortedpoints1[i].x * intrinsic_LTRT.at<double>(0,0) + intrinsic_LTRT.at<double>(0,2);
                undistortedpoints1[i].y = undistortedpoints1[i].y * intrinsic_LTRT.at<double>(1,1) + intrinsic_LTRT.at<double>(1,2);
            }

            for(unsigned int i = 0; i < undistortedpoints0.size(); i++)
            {
                std::cout << points0[i] << "  " << undistortedpoints0[i] << std::endl;
// presumably debugging
//                if( undistortedpoints0[i].x >= 0 && undistortedpoints1[i].x >= 0 )
//                {
//                    cv::circle(drawn_img0, undistortedpoints0[i], circleRadius, cv::Scalar(0,0,255), 1);
//                    cv::circle(drawn_img0, undistortedpoints0[i], circleRadius+1, cv::Scalar(0,0,255), 1);
//                    cv::circle(drawn_img0, undistortedpoints0[i], circleRadius+2, cv::Scalar(0,0,255), 1);
//                    cv::circle(drawn_img1, undistortedpoints1[i], circleRadius, cv::Scalar(0,0,255), 1);
//                    cv::circle(drawn_img1, undistortedpoints1[i], circleRadius+1, cv::Scalar(0,0,255), 1);
//                    cv::circle(drawn_img1, undistortedpoints1[i], circleRadius+2, cv::Scalar(0,0,255), 1);
//                }
            }

            cv::computeCorrespondEpilines(undistortedpoints1, 2,fundamental,lines0);
            cv::computeCorrespondEpilines(undistortedpoints0, 1,fundamental,lines1);
            for (int i= 0; i < lines0.size(); i++)
            {
                cv::line(drawn_img0,cv::Point(0,-lines0[i][2]/lines0[i][1]),
                         cv::Point(drawn_img0.cols,-(lines0[i][2]+lines0[i][0]*drawn_img0.cols)/lines0[i][1]),
                         cv::Scalar(255,255,255));
            }

            for (int i= 0; i < lines1.size(); i++)
            {
                cv::line(drawn_img1,cv::Point(0,-lines1[i][2]/lines1[i][1]),
                         cv::Point(drawn_img1.cols,-(lines1[i][2]+lines1[i][0]*drawn_img1.cols)/lines1[i][1]),
                         cv::Scalar(255,255,255));
            }
        }
    }
}

void TwoView::drawSubimage(int pos) //drawing 500x500 pixel subimage for refinement window
{
    if( !getPointstorage()->center2d_0.empty() && pos < getPointstorage()->center2d_0.size())
    {
        subImage.setTo(0);
        int xoffset = 0;
        int yoffset = 0;
        int xcut = 0;
        int ycut = 0;

        if( getPointstorage()->center2d_0[pos].x < 250)
            xoffset = 250 - getPointstorage()->center2d_0[pos].x;
        if( getPointstorage()->center2d_0[pos].y < 250)
            yoffset = 250 - getPointstorage()->center2d_0[pos].y;
        if( (orig_img0.cols - 1 - getPointstorage()->center2d_0[pos].x) < 250 )
            xcut = 250 - (orig_img0.cols - 1 - getPointstorage()->center2d_0[pos].x);
        if( (orig_img0.rows - 1 - getPointstorage()->center2d_0[pos].y) < 250 )
            ycut = 250 - (orig_img0.rows - 1 - getPointstorage()->center2d_0[pos].y);
        cv::Mat roi = subImage( cv::Rect(xoffset, yoffset, 500-xoffset-xcut, 500-yoffset-ycut));
        orig_img0( cv::Rect(getPointstorage()->center2d_0[pos].x - 250 + xoffset, getPointstorage()->center2d_0[pos].y - 250 + yoffset, 500-xoffset-xcut, 500-yoffset-ycut)).copyTo(roi);
        cv::circle(roi, cv::Point2f(250-xoffset, 250-yoffset), 0, cv::Scalar(255,255,0), 1);
        cv::circle(roi, cv::Point2f(250-xoffset, 250-yoffset), 5, cv::Scalar(255,255,0), 1);

        xoffset = yoffset = xcut = ycut = 0;
        if( getPointstorage()->center2d_1[pos].x < 250)
            xoffset = 250 - getPointstorage()->center2d_1[pos].x;
        if( getPointstorage()->center2d_1[pos].y < 250)
            yoffset = 250 -getPointstorage()->center2d_1[pos].y;
        if( (orig_img1.cols - 1 - getPointstorage()->center2d_1[pos].x) < 250 )
            xcut = 250 - (orig_img1.cols - 1 - getPointstorage()->center2d_1[pos].x);
        if( (orig_img1.rows - 1 - getPointstorage()->center2d_1[pos].y) < 250 )
            ycut = 250 - (orig_img1.rows - 1 - getPointstorage()->center2d_1[pos].y);
        roi = subImage( cv::Rect(500 + xoffset, yoffset, 500-xoffset-xcut, 500-yoffset-ycut));
        orig_img1( cv::Rect(getPointstorage()->center2d_1[pos].x - 250 + xoffset, getPointstorage()->center2d_1[pos].y - 250 + yoffset, 500-xoffset-xcut, 500-yoffset-ycut)).copyTo(roi);
        cv::circle(roi, cv::Point2f(250-xoffset, 250-yoffset), 0, cv::Scalar(255,255,0), 1);
        cv::circle(roi, cv::Point2f(250-xoffset, 250-yoffset), 5, cv::Scalar(255,255,0), 1);
        drawImages();
    }
}

void TwoView::matchelectrodes() //for every stripe: checking if found stripe size in left and right image pair is equal to valid stripe size
                                //if(true): adding electrodes which are existing in both image pairs
{
    electrodes0->find();
    electrodes1->find();

    manual.clear();

    if( electrodes0->stripe8_0.size() == electrodes1->stripe8_0.size() && electrodes0->stripe8_0.size() == 8)
    {
    stripe8_0->clear(); //only clear when actually performing the matching (moved into if structure)
        for( int i=0; i < electrodes0->stripe8_0.size(); i++)
        {
            if( electrodes0->stripe8_0[i].x >= 0 && electrodes1->stripe8_0[i].x >= 0 )
            {
                stripe8_0->center2d_0.push_back( electrodes0->stripe8_0[i]);
                stripe8_0->center2d_1.push_back( electrodes1->stripe8_0[i]);
            }
            else
            {
                stripe8_0->center2d_0.push_back(cv::Point2f( -1, -1));
                stripe8_0->center2d_1.push_back(cv::Point2f( -1, -1));
            }
            stripe8_0->center3d_0.push_back( cv::Point3f( -1, -1, -1));
            stripe8_0->center3d_1.push_back( cv::Point3f( -1, -1, -1));
            stripe8_0->id.push_back(-1);
            stripe8_0->description.push_back("");
        }
    }

    if( electrodes0->stripe8_1.size() == electrodes1->stripe8_1.size() && electrodes0->stripe8_1.size() == 8)
    {
    stripe8_1->clear();
        for( int i=0; i < electrodes0->stripe8_1.size(); i++)
        {
            if( electrodes0->stripe8_1[i].x >= 0 && electrodes1->stripe8_1[i].x >= 0 )
            {
                stripe8_1->center2d_0.push_back( electrodes0->stripe8_1[i]);
                stripe8_1->center2d_1.push_back( electrodes1->stripe8_1[i]);
            }
            else
            {
                stripe8_1->center2d_0.push_back(cv::Point2f( -1, -1));
                stripe8_1->center2d_1.push_back(cv::Point2f( -1, -1));
            }
            stripe8_1->center3d_0.push_back( cv::Point3f( -1, -1, -1));
            stripe8_1->center3d_1.push_back( cv::Point3f( -1, -1, -1));
            stripe8_1->id.push_back(-1);
            stripe8_1->description.push_back("");
        }
    }

    if( electrodes0->stripe8_2.size() == electrodes1->stripe8_2.size() && electrodes0->stripe8_2.size() == 8)
    {
    stripe8_2->clear();
        for( int i=0; i < electrodes0->stripe8_2.size(); i++)
        {
            if( electrodes0->stripe8_2[i].x >= 0 && electrodes1->stripe8_2[i].x >= 0 )
            {
                stripe8_2->center2d_0.push_back( electrodes0->stripe8_2[i]);
                stripe8_2->center2d_1.push_back( electrodes1->stripe8_2[i]);
            }
            else
            {
                stripe8_2->center2d_0.push_back(cv::Point2f( -1, -1));
                stripe8_2->center2d_1.push_back(cv::Point2f( -1, -1));
            }
            stripe8_2->center3d_0.push_back( cv::Point3f( -1, -1, -1));
            stripe8_2->center3d_1.push_back( cv::Point3f( -1, -1, -1));
            stripe8_2->id.push_back(-1);
            stripe8_2->description.push_back("");
        }
    }

    if( electrodes0->stripe8_3.size() == electrodes1->stripe8_3.size() && electrodes0->stripe8_3.size() == 8)
    {
    stripe8_3->clear();
        for( int i=0; i < electrodes0->stripe8_3.size(); i++)
        {
            if( electrodes0->stripe8_3[i].x >= 0 && electrodes1->stripe8_3[i].x >= 0 )
            {
                stripe8_3->center2d_0.push_back( electrodes0->stripe8_3[i]);
                stripe8_3->center2d_1.push_back( electrodes1->stripe8_3[i]);
            }
            else
            {
                stripe8_3->center2d_0.push_back(cv::Point2f( -1, -1));
                stripe8_3->center2d_1.push_back(cv::Point2f( -1, -1));
            }
            stripe8_3->center3d_0.push_back( cv::Point3f( -1, -1, -1));
            stripe8_3->center3d_1.push_back( cv::Point3f( -1, -1, -1));
            stripe8_3->id.push_back(-1);
            stripe8_3->description.push_back("");
        }
    }

    if( electrodes0->stripe12_0.size() == electrodes1->stripe12_0.size() && electrodes0->stripe12_0.size() == 12)
    {
    stripe12_0->clear();
        for( int i=0; i < electrodes0->stripe12_0.size(); i++)
        {
            if( electrodes0->stripe12_0[i].x >= 0 && electrodes1->stripe12_0[i].x >= 0 )
            {
                stripe12_0->center2d_0.push_back( electrodes0->stripe12_0[i]);
                stripe12_0->center2d_1.push_back( electrodes1->stripe12_0[i]);
            }
            else
            {
                stripe12_0->center2d_0.push_back(cv::Point2f( -1, -1));
                stripe12_0->center2d_1.push_back(cv::Point2f( -1, -1));
            }
            stripe12_0->center3d_0.push_back( cv::Point3f( -1, -1, -1));
            stripe12_0->center3d_1.push_back( cv::Point3f( -1, -1, -1));
            stripe12_0->id.push_back(-1);
            stripe12_0->description.push_back("");
        }
    }

    if( electrodes0->stripe12_1.size() == electrodes1->stripe12_1.size() && electrodes0->stripe12_1.size() == 12)
    {
    stripe12_1->clear();
        for( int i=0; i < electrodes0->stripe12_1.size(); i++)
        {
            if( electrodes0->stripe12_1[i].x >= 0 && electrodes1->stripe12_1[i].x >= 0 )
            {
                stripe12_1->center2d_0.push_back( electrodes0->stripe12_1[i]);
                stripe12_1->center2d_1.push_back( electrodes1->stripe12_1[i]);
            }
            else
            {
                stripe12_1->center2d_0.push_back(cv::Point2f( -1, -1));
                stripe12_1->center2d_1.push_back(cv::Point2f( -1, -1));
            }
            stripe12_1->center3d_0.push_back( cv::Point3f( -1, -1, -1));
            stripe12_1->center3d_1.push_back( cv::Point3f( -1, -1, -1));
            stripe12_1->id.push_back(-1);
            stripe12_1->description.push_back("");
        }
    }

    if( electrodes0->stripe12_2.size() == electrodes1->stripe12_2.size() && electrodes0->stripe12_2.size() == 12)
    {
    stripe12_2->clear();
        for( int i=0; i < electrodes0->stripe12_2.size(); i++)
        {
            if( electrodes0->stripe12_2[i].x >= 0 && electrodes1->stripe12_2[i].x >= 0 )
            {
                stripe12_2->center2d_0.push_back( electrodes0->stripe12_2[i]);
                stripe12_2->center2d_1.push_back( electrodes1->stripe12_2[i]);
            }
            else
            {
                stripe12_2->center2d_0.push_back(cv::Point2f( -1, -1));
                stripe12_2->center2d_1.push_back(cv::Point2f( -1, -1));
            }
            stripe12_2->center3d_0.push_back( cv::Point3f( -1, -1, -1));
            stripe12_2->center3d_1.push_back( cv::Point3f( -1, -1, -1));
            stripe12_2->id.push_back(-1);
            stripe12_2->description.push_back("");
        }
    }

    if( electrodes0->stripe12_3.size() == electrodes1->stripe12_3.size() && electrodes0->stripe12_3.size() == 12)
    {
    stripe12_3->clear();
        for( int i=0; i < electrodes0->stripe12_3.size(); i++)
        {
            if( electrodes0->stripe12_3[i].x >= 0 && electrodes1->stripe12_3[i].x >= 0 )
            {
                stripe12_3->center2d_0.push_back( electrodes0->stripe12_3[i]);
                stripe12_3->center2d_1.push_back( electrodes1->stripe12_3[i]);
            }
            else
            {
                stripe12_3->center2d_0.push_back(cv::Point2f( -1, -1));
                stripe12_3->center2d_1.push_back(cv::Point2f( -1, -1));
            }
            stripe12_3->center3d_0.push_back( cv::Point3f( -1, -1, -1));
            stripe12_3->center3d_1.push_back( cv::Point3f( -1, -1, -1));
            stripe12_3->id.push_back(-1);
            stripe12_3->description.push_back("");
        }
    }

    drawImages();
}

void TwoView::getValidpoints( std::vector<cv::Point2f> &points0, std::vector<cv::Point2f> &points1, match *m)
{
    //checking if points are set
    for( int i=0; i < m->center2d_0.size(); i++)
    {
        if( m->center2d_0[i].x >= 0 )
        {
            points0.push_back(m->center2d_0[i]);
            points1.push_back(m->center2d_1[i]);
        }
    }
}

void TwoView::getValid3dpoints( std::vector<cv::Point3f> &points0, std::vector<cv::Point3f> &points1, match *m)
{
    //checking if points are set
    for( int i=0; i < m->center2d_0.size(); i++)
    {
        if( ( m->center3d_0[i].x != -1 )&&( m->center3d_0[i].y != -1 )&&( m->center3d_0[i].z != -1 ) )//default value. also, z component cannot be negative (otherwise behind camera)
        {
            points0.push_back(m->center3d_0[i]);
            points1.push_back(m->center3d_1[i]);
        }
    }
}

void TwoView::setValid3dpoints( cv::Mat mat0, cv::Mat mat1, match *m )
{
    int cols = ( (mat0.cols > mat1.cols) ? mat0.cols : mat1.cols );
    int ctr = 0;

    for( int i=0; i < m->center2d_0.size(); i++)
    {
        if( m->center2d_0[i].x >= 0 && ctr < cols)
        {
            m->center3d_0[i] = cv::Point3f(mat0.at<double>(0,ctr)/mat0.at<double>(3,ctr),
                                                 mat0.at<double>(1,ctr)/mat0.at<double>(3,ctr),
                                                 mat0.at<double>(2,ctr)/mat0.at<double>(3,ctr));
            m->center3d_1[i] = cv::Point3f(mat1.at<double>(0,ctr)/mat1.at<double>(3,ctr),
                                                    mat1.at<double>(1,ctr)/mat1.at<double>(3,ctr),
                                                    mat1.at<double>(2,ctr)/mat1.at<double>(3,ctr));
            ctr++;
        }
    }
}
