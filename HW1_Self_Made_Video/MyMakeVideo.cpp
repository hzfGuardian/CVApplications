//
//  main.cpp
//  MyMakeVideo
//
//  Created by hzfmacbook on 11/27/15.
//  Copyright © 2015 hzfmacbook. All rights reserved.
//
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;


//rotate an image
void rotateImage(IplImage* img, IplImage *img_rotate, int degree)
{
    //center rotate
    CvPoint2D32f center;
    center.x = float (img->width/2.0+0.5);
    center.y = float (img->height/2.0+0.5);
    
    //calculate the relating matrix
    float m[6];
    CvMat M = cvMat( 2, 3, CV_32F, m );
    cv2DRotationMatrix( center, degree,1, &M);
    
    //transform and fill the resting pixels with black
    cvWarpAffine(img,img_rotate, &M,CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS,cvScalarAll(0) );
}


IplImage* waveImage(IplImage *src, int arg)
{
    IplImage *dst = cvCloneImage(src);
    int width=dst->width;
    int height=dst->height;
    int step=dst->widthStep;
    int channel=dst->nChannels;
    uchar* data=(uchar *)dst->imageData;
    int sign=-1;
    for(int i=0;i<height;i++)
    {
        int cycle = arg;
        int margin=(i%cycle);
        if((i/cycle)%2==0)
        {
            sign=-1;
        }
        else
        {
            sign=1;
        }
        if(sign==-1)
        {
            margin=cycle-margin;
            for(int j=0;j<width-margin;j++)
            {
                for(int k=0;k<channel;k++)
                {
                    data[i*step+j*channel+k]=data[i*step+(j+margin)*channel+k];
                }
            }
        }
        else if(sign==1)
        {
            for(int j=0;j<width-margin;j++)
            {
                for(int k=0;k<channel;k++)
                {
                    data[i*step+j*channel+k]=data[i*step+(j+margin)*channel+k];
                }
            }
        }
    }
    
    return dst;
}


int main(int argc, const char * argv[])
{
    // insert code here...
    if (argc < 2) {
        cout << "Lack of path name." << endl;
        return 1;
    }
    
    int height, width;
    
    //names of image files
    vector<string> image_file;
    
    //name of video .avi
    vector<string> video_file;
    
    //get path of all test files
    string path = string(argv[1]) + "/";
    
    //get pictures name and store in vector
    Directory dir;
    
    //get all .jpg files on current directory in unix system
    image_file = dir.GetListFiles(path, ".jpg");
    
    //maybe you run it in windows system
    if (image_file.empty()) {
        //in windows system, the 3rd argument is "*.jpg"
        image_file = dir.GetListFiles(path, "*.jpg");
        
    }
    
    //get video name
    video_file = dir.GetListFiles(path, ".avi");
    
    if (video_file.empty()) {
        //in windows system
        video_file = dir.GetListFiles(path, "*.avi");
    }
    
    //open the video test.avi
    if (video_file.empty()) {
        cout << "no video file exists in current directory." << endl;
        return 1;
    }
    VideoCapture capture(video_file[0]);
    
    //judge whether successfully open
    if (!capture.isOpened()) {
        return 1;
    }
    
    //get frame rate
    double frame_rate = capture.get(CV_CAP_PROP_FPS);
    
    //get size
    height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    
    //Text
    CvFont font;
    
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_HERSHEY_SCRIPT_SIMPLEX, 1.0, 1.0, 0, 1 );
    
    //write video
    
    CvVideoWriter* cwriter = cvCreateVideoWriter("res.avi", capture.get(CV_CAP_PROP_FOURCC), frame_rate, cvSize(width, height));
    
    for (int i = 0; i < image_file.size(); i++) {
        
        IplImage* img = cvLoadImage(image_file[i].c_str());
        IplImage* img2 = cvCreateImage(cvSize(width, height), img->depth, img->nChannels);
        
        //IplImage resize
        cvResize(img, img2, CV_INTER_CUBIC);
        
        cvPutText(img2, "3130100677 Zhuofei Huang", cvPoint(100,100), &font, cvScalar(255,0,0) );
        
        //cvShowImage("MyMakeVideo", img2);
        //waitKey(3000);
        
        
        for (int j = 0; j < 1 * frame_rate; j++) {
        
            cvWriteFrame(cwriter, img2);
            
        }
        

        IplImage* old = img2, *newImg = cvCreateImage(cvSize(img2->width, img2->height), IPL_DEPTH_8U, 3);
        
        //special efficacy
        if (i % 2 == 0) {
            
            for (int angle = 0; angle < 180; angle+=2) {
                
                //each time we rotate 2 degree
                rotateImage(old, newImg, 2);
                
                //cvShowImage("MyMakeVideo", newImg);
                
                cvWriteFrame(cwriter, newImg);
                
                old = newImg;
                
                //waitKey(1);
            }
            
        }
        
        else {
            
            for (int k = 1; k <= 50; k++) {
                //wave
                newImg = waveImage(old, k);
                
                //cvShowImage("MyMakeVideo", newImg);
                
                cvWriteFrame(cwriter, newImg);
                
                old = newImg;
                
            }
        }
        
    }
    
    bool stop = false;
    Mat frame;//current video's frame
    
    int delay = 1000 / frame_rate;
    
    while (!stop) {
        //attempt to read next frame
        if (!capture.read(frame))
            break;
        
        IplImage pImg(frame);
        
        cvPutText(&pImg, "3130100677 Zhuofei Huang", cvPoint(100,100),&font, cvScalar(255,255,255));
        //cvShowImage("MyMakeVideo", &pImg);
        
        cvWriteFrame(cwriter, &pImg);
        waitKey(delay);
        
    }
    
    //close the video
    capture.release();
    
    cout << "finished" << endl;
    
    return 0;
    
}





