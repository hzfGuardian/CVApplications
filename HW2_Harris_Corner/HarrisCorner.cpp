//
//  main.cpp
//  Harris
//
//  Created by hzfmacbook on 12/8/15.
//  Copyright © 2015 hzfmacbook. All rights reserved.
//

#include <iostream>
#include <cmath>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


//arguments
double K;
int Aperture_size;


//picture
int height, width;

//
Mat_<double> big_eigen;
Mat_<double> small_eigen;
Mat_<double> score;

//source image
Mat src;

//slider bar tools
const int slider_max = 1000, corner_max = 1000;
int slider, corn_slider;

double my_threshold = 1e6;

double corner_threshold = 1e9;

void drawOnImage(Mat &image, vector<Point> &points)
{
    int radius = 3, thickness = 2;
    
    // display all corners
    for (vector<Point>::iterator it = points.begin(); it != points.end(); it++) {
        circle(image, *it, radius, Scalar(0, 255, 0), thickness);
    }
}

void bigEigenImage()
{
    Mat my_mat(height, width, CV_8U);
    
    for (int x = Aperture_size / 2; x <= width - Aperture_size + 1; x++)
    {
        for (int y = Aperture_size / 2; y <= height - Aperture_size + 1; y++)
        {
            if (big_eigen(x, y) > my_threshold) {
                my_mat.at<uchar>(y, x) = 255;
            }
            else {
                my_mat.at<uchar>(y, x) = 0;
            }
        }
    }
    imshow("max", my_mat);
}

void smallEigenImage()
{
    Mat my_mat(height, width, CV_8U);
    
    for (int x = Aperture_size / 2; x <= width - Aperture_size + 1; x++)
    {
        for (int y = Aperture_size / 2; y <= height - Aperture_size + 1; y++)
        {
            if (small_eigen(x, y) > my_threshold) {
                my_mat.at<uchar>(y, x) = 255;
            }
            else {
                my_mat.at<uchar>(y, x) = 0;
            }
        }
    }
    imshow("min", my_mat);
}

void REigenImage()
{
    Mat my_mat(height, width, CV_8U);
    
    for (int x = Aperture_size / 2; x <= width - Aperture_size + 1; x++)
    {
        for (int y = Aperture_size / 2; y <= height - Aperture_size + 1; y++)
        {
            if (score(x, y) > corner_threshold) //corner
            {
                my_mat.at<uchar>(y, x) = 0;
            }
            else if (score(x, y) < 0 && score(x, y) < -corner_threshold) //edge
            {
                my_mat.at<uchar>(y, x) = 127;
            }
            else //flat
            {
                my_mat.at<uchar>(y, x) = 255;
            }
        }
    }
    imshow("R", my_mat);
}

void finalCornerImage()
{
    vector<Point> points;
    
    for (int x = Aperture_size / 2; x <= width - Aperture_size + 1; x++)
    {
        for (int y = Aperture_size / 2; y <= height - Aperture_size + 1; y++)
        {
            if (score(x, y) > corner_threshold
                && score(x, y) > score(x-1, y-1) && score(x, y) > score(x-1, y)
                && score(x, y) > score(x-1, y+1) && score(x, y) > score(x, y-1)
                && score(x, y) > score(x, y+1) && score(x, y) > score(x+1, y-1)
                && score(x, y) > score(x+1, y) && score(x, y) > score(x+1, y+1))
            {
                points.push_back(Point(x, y));
            }
        }
    }
    
    Mat res = src.clone();
    imshow("origin", src);
    
    drawOnImage(res, points);
    
    imshow("Harris Corner", res);
    
}

void on_trackbar(int pos, void *)
{
    if (pos > 0) {
        my_threshold = pos * 1000;
        bigEigenImage();
        smallEigenImage();
        REigenImage();
        finalCornerImage();
    }
}

void corn_trackbar(int pos, void *)
{
    if (pos > 0) {
        corner_threshold = pos * 1e6;
        bigEigenImage();
        smallEigenImage();
        REigenImage();
        finalCornerImage();
    }
}

int main(int argc, const char * argv[])
{
    // insert code here...
    if (argc < 3) {
        cout << "error: arguments lack." << endl;
        return 1;
    }
  
    vector<Point> points, maxEig, minEig;
    
    //transfer to right type
    sscanf(argv[1], "%lf", &K);
    sscanf(argv[2], "%d", &Aperture_size);
    
    
    src = imread("test.bmp");
    
    if (!src.data) {
        cout << "error: file does not exists." << endl;
        return 1;
    }
    
    //get the height and width of the picture
    height = src.rows;
    width = src.cols;
    
    Mat gray;
    Mat sobel_x;
    Mat sobel_y;
    
    //transfer to grayscale image
    cvtColor(src, gray, CV_BGR2GRAY);
    
    
    //use sobel to calculate the partial
    Sobel(gray, sobel_x, CV_16S, 1, 0, Aperture_size, 1, 0, BORDER_REPLICATE);
    Sobel(gray, sobel_y, CV_16S, 0, 1, Aperture_size, 1, 0, BORDER_REPLICATE);
    
    //store the score
    big_eigen = Mat_<double>(gray.cols, gray.rows, CV_64FC1);
    small_eigen = Mat_<double>(gray.cols, gray.rows, CV_64FC1);
    score = Mat_<double>(gray.cols, gray.rows, CV_64FC1);
    
    //calculate related matrix M for each pixel 
    for (int x = Aperture_size / 2; x <= gray.cols - Aperture_size + 1; x++)
    {
        for (int y = Aperture_size / 2; y <= gray.rows - Aperture_size + 1; y++)
        {
            double A = 0, B = 0, C = 0;
            
            for (int xx = x - Aperture_size / 2; xx <= x + Aperture_size / 2; xx++) {
                for (int yy = y - Aperture_size / 2; yy <= y + Aperture_size / 2; yy++) {
                    
                    double pdx = sobel_x.at<int16_t>(yy, xx);
                    double pdy = sobel_y.at<int16_t>(yy, xx);
                    A += pdx * pdx;
                    B += pdy * pdy;
                    C += pdx * pdy;
                }
            }
            
            //so the matrix is: [pdx^2 pdx*pdy; pdx*pdy pdy^2]
            double det = A * B - C * C;
            double trace = A + B;
            
            //calculate two eigen values
            big_eigen(x, y) = (trace + sqrt(trace*trace-4*det)) / 2;
            small_eigen(x, y) = trace - big_eigen(x, y);
            
            //calculate the score, compare it with threshold
            score(x, y) = det - K * trace * trace;
            
        }
    }
    
    //trackbar
    namedWindow("Harris Corner");
    createTrackbar("Threshold", "Harris Corner", &slider, slider_max, on_trackbar);
    createTrackbar("CornerThre", "Harris Corner", &corn_slider, corner_max, corn_trackbar);
    
    bigEigenImage();
    smallEigenImage();
    REigenImage();
    finalCornerImage();
    
    waitKey(0);
    
    return 0;
}




