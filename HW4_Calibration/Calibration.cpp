//
//  main.cpp
//  HW4
//
//  Created by hzfmacbook on 1/4/16.
//  Copyright © 2016 hzfmacbook. All rights reserved.
//


#include <opencv/cv.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int n_boards = 0; //Will be set by input list
int board_dt = 90; //Wait 90 frames per chessboard view
int board_w;
int board_h;

int main(int argc, const char* argv[])
{
    
    if(argc != 5){
        printf("ERROR: Wrong number of input parameters\n");
        
        return -1;
    }
    board_w  = atoi(argv[1]);
    board_h  = atoi(argv[2]);
    n_boards = atoi(argv[3]);
    board_dt = atoi(argv[4]);
    
    int board_n  = board_w * board_h;
    CvSize board_sz = cvSize( board_w, board_h );
    
    
    cvNamedWindow( "Calibration" );

    //create mats
    CvMat* image_points      = cvCreateMat(n_boards*board_n,2,CV_32FC1);
    CvMat* object_points     = cvCreateMat(n_boards*board_n,3,CV_32FC1);
    CvMat* point_counts      = cvCreateMat(n_boards,1,CV_32SC1);
    CvMat* intrinsic_matrix  = cvCreateMat(3,3,CV_32FC1);
    CvMat* distortion_coeffs = cvCreateMat(4,1,CV_32FC1);
    
    CvPoint2D32f* corners = new CvPoint2D32f[ board_n ];
    int corner_count;
    int successes = 0;
    int step;
    
    char filename[20];
    int i = 1;
    sprintf(filename, "right%02d.jpg", i);
    
    IplImage *image = cvLoadImage(filename);
    IplImage *gray_image = cvCreateImage(cvGetSize(image),8,1);//subpixel
    
    // CAPTURE CORNER VIEWS LOOP UNTIL WE’VE GOT n_boards
    // SUCCESSFUL CAPTURES (ALL CORNERS ON THE BOARD ARE FOUND)
    //
    
    while(successes < n_boards) {
        //Skip every board_dt frames to allow user to move chessboard
        //Find chessboard corners:
        
        int found = cvFindChessboardCorners(
                                            image, board_sz, corners, &corner_count,
                                            CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS
                                            );
        
        //Get Subpixel accuracy on those corners
        cvCvtColor(image, gray_image, CV_BGR2GRAY);
        cvFindCornerSubPix(gray_image, corners, corner_count,
                           cvSize(11,11),cvSize(-1,-1), cvTermCriteria(
                                                                       CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
        
        //Draw it
        cvDrawChessboardCorners(image, board_sz, corners,
                                corner_count, found);
        //      cvShowImage( "Calibration", image );
        
        // If we got a good board, add it to our data
        if( corner_count == board_n ) {
            cvShowImage( "Calibration", image ); //show in color if we did collect the image
            step = successes*board_n;
            for( int i=step, j=0; j<board_n; ++i,++j ) {
                CV_MAT_ELEM(*image_points, float,i,0) = corners[j].x;
                CV_MAT_ELEM(*image_points, float,i,1) = corners[j].y;
                CV_MAT_ELEM(*object_points,float,i,0) = j/board_w;
                CV_MAT_ELEM(*object_points,float,i,1) = j%board_w;
                CV_MAT_ELEM(*object_points,float,i,2) = 0.0f;
            }
            CV_MAT_ELEM(*point_counts, int,successes,0) = board_n;
            successes++;
            printf("Collected our %d of %d needed chessboard images\n", i, n_boards);
        }
        else
            cvShowImage( "Calibration", gray_image ); //Show Gray if we didn't collect the image
        //end skip board_dt between chessboard capture
        
        
        i++;
        
        if (i > n_boards) {
            break;
        }
        
        sprintf(filename, "right%02d.jpg", i);
        image = cvLoadImage(filename);//cvQueryFrame( capture ); //Get next image
        //cvShowImage("Raw Video", image);
        
    } //END COLLECTION WHILE LOOP.
    
    cvDestroyWindow("Calibration");
    printf("\n\n*** CALLIBRATING THE CAMERA...");
    
    //ALLOCATE MATRICES ACCORDING TO HOW MANY CHESSBOARDS FOUND
    CvMat* object_points2  = cvCreateMat(successes*board_n,3,CV_32FC1);
    CvMat* image_points2   = cvCreateMat(successes*board_n,2,CV_32FC1);
    CvMat* point_counts2   = cvCreateMat(successes,1,CV_32SC1);
    
    //TRANSFER THE POINTS INTO THE CORRECT SIZE MATRICES
    for(int i = 0; i<successes*board_n; ++i){
        CV_MAT_ELEM( *image_points2, float, i, 0) = CV_MAT_ELEM( *image_points, float, i, 0);
        CV_MAT_ELEM( *image_points2, float,i,1) = CV_MAT_ELEM( *image_points, float, i, 1);
        CV_MAT_ELEM(*object_points2, float, i, 0) = CV_MAT_ELEM( *object_points, float, i, 0) ;
        CV_MAT_ELEM( *object_points2, float, i, 1) = CV_MAT_ELEM( *object_points, float, i, 1) ;
        CV_MAT_ELEM( *object_points2, float, i, 2) = CV_MAT_ELEM( *object_points, float, i, 2) ;
    }
    
    for(int i=0; i<successes; ++i) { //These are all the same number
        CV_MAT_ELEM( *point_counts2, int, i, 0) =
        CV_MAT_ELEM( *point_counts, int, i, 0);
    }
    cvReleaseMat(&object_points);
    cvReleaseMat(&image_points);
    cvReleaseMat(&point_counts);
    
    // At this point we have all of the chessboard corners we need.
    // Initialize the intrinsic matrix such that the two focal
    // lengths have a ratio of 1.0
    //
    CV_MAT_ELEM( *intrinsic_matrix, float, 0, 0 ) = 1.0f;
    CV_MAT_ELEM( *intrinsic_matrix, float, 1, 1 ) = 1.0f;
    
    //CALIBRATE THE CAMERA!
    cvCalibrateCamera2(
                       object_points2, image_points2,
                       point_counts2,  cvGetSize( image ),
                       intrinsic_matrix, distortion_coeffs,
                       NULL, NULL,0  //CV_CALIB_FIX_ASPECT_RATIO
                       );
    
    
    
    // EXAMPLE OF LOADING THESE MATRICES BACK IN:
    CvMat *intrinsic = cvCloneMat(intrinsic_matrix) ;//(CvMat*)cvLoad("Intrinsics.xml");
    CvMat *distortion = cvCloneMat(distortion_coeffs) ;//(CvMat*)cvLoad("Distortion.xml");
    
    // Build the undistort map which we will use for all
    // subsequent frames.
    //
    IplImage* mapx = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
    IplImage* mapy = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
    
    
    cvInitUndistortMap(intrinsic, distortion, mapx, mapy);
    
    // Just run the camera to the screen, now showing the raw and
    // the undistorted image.
    //
    cvNamedWindow( "Undistort" );
    
    
    char file_name[20] = "test.jpg";
    
    if((image = cvLoadImage(file_name))== 0) {
        printf("Error: Couldn't load %s\n", file_name);
        return 1;
    }
    gray_image = cvCreateImage(cvGetSize(image),8,1);
    cvCvtColor(image, gray_image, CV_BGR2GRAY);
    
    //UNDISTORT OUR IMAGE
    
    cvInitUndistortMap(
                       intrinsic,
                       distortion,
                       mapx,
                       mapy
                       );
    
    IplImage *t = cvCloneImage(image);
    cvRemap( t, image, mapx, mapy );
    
    //GET THE CHECKERBOARD ON THE PLANE
    cvNamedWindow("Checkers");
    
    
    int found = cvFindChessboardCorners(
                                        image,
                                        board_sz,
                                        corners,
                                        &corner_count,
                                        CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS
                                        );
    if(!found){
        printf("Couldn't aquire checkerboard on %s, only found %d of %d corners\n",
               file_name, corner_count, board_n);
        return 1;
    }
    //Get Subpixel accuracy on those corners
    cvFindCornerSubPix(gray_image, corners, corner_count,
                       cvSize(11,11),cvSize(-1,-1),
                       cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
    
    //GET THE IMAGE AND OBJECT POINTS:
    //Object points are at (r,c): (0,0), (board_w-1,0), (0,board_h-1), (board_w-1,board_h-1)
    //That means corners are at: corners[r*board_w + c]
    CvPoint2D32f objPts[4], imgPts[4];
    objPts[0].x = 0;         objPts[0].y = 0;
    objPts[1].x = board_w-1; objPts[1].y = 0;
    objPts[2].x = 0;         objPts[2].y = board_h-1;
    objPts[3].x = board_w-1; objPts[3].y = board_h-1;
    imgPts[0] = corners[0];
    imgPts[1] = corners[board_w-1];
    imgPts[2] = corners[(board_h-1)*board_w];
    imgPts[3] = corners[(board_h-1)*board_w + board_w-1];
    
    //DRAW THE POINTS in order: B,G,R,YELLOW
    cvCircle(image,cvPointFrom32f(imgPts[0]),9,CV_RGB(0,0,255),3);
    cvCircle(image,cvPointFrom32f(imgPts[1]),9,CV_RGB(0,255,0),3);
    cvCircle(image,cvPointFrom32f(imgPts[2]),9,CV_RGB(255,0,0),3);
    cvCircle(image,cvPointFrom32f(imgPts[3]),9,CV_RGB(255,255,0),3);
    
    //DRAW THE FOUND CHECKERBOARD
    
    cvDrawChessboardCorners(image, board_sz, corners, corner_count, found);
    cvShowImage( "Checkers", image );
    
    //FIND THE HOMOGRAPHY
    CvMat *H = cvCreateMat( 3, 3, CV_32F);
    //CvMat *H_invt = cvCreateMat(3,3,CV_32F);
    cvGetPerspectiveTransform(objPts,imgPts,H);
    
    //LET THE USER ADJUST THE Z HEIGHT OF THE VIEW
    float Z = 25;
    int key = 0;
    IplImage *birds_image = cvCloneImage(image);
    cvNamedWindow("Birds_Eye");
    while(key != 27) {//escape key stops
        CV_MAT_ELEM(*H,float,2,2) = Z;
        //	   cvInvert(H,H_invt); //If you want to invert the homography directly
        //	   cvWarpPerspective(image,birds_image,H_invt,CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
        //USE HOMOGRAPHY TO REMAP THE VIEW
        cvWarpPerspective(image,birds_image,H,
                          CV_INTER_LINEAR+CV_WARP_INVERSE_MAP+CV_WARP_FILL_OUTLIERS );
        cvShowImage("Birds_Eye", birds_image);
        key = cvWaitKey();
        if(key == 'u') Z += 0.5;
        if(key == 'd') Z -= 0.5;
    }
    
    cout << "end" << endl;
    
    //SHOW ROTATION AND TRANSLATION VECTORS
    image_points  = cvCreateMat(4,1,CV_32FC2);
    object_points = cvCreateMat(4,1,CV_32FC3);
    for(int i=0;i<4;++i){
        CV_MAT_ELEM(*image_points,CvPoint2D32f,i,0) = imgPts[i];
        CV_MAT_ELEM(*object_points,CvPoint3D32f,i,0) = cvPoint3D32f(objPts[i].x,objPts[i].y,0);
    }
    
    CvMat *RotRodrigues   = cvCreateMat(3,1,CV_32F);
    CvMat *Rot   = cvCreateMat(3,3,CV_32F);
    CvMat *Trans = cvCreateMat(3,1,CV_32F);
    cvFindExtrinsicCameraParams2(object_points,image_points,
                                 intrinsic,distortion,
                                 RotRodrigues,Trans);
    cvRodrigues2(RotRodrigues,Rot);
    
    delete [] corners;
    
    return 0;
}






