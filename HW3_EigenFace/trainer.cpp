//
//  mytest.cpp
//  EigenFace
//
//  Created by hzfmacbook on 1/2/16.
//  Copyright © 2016 hzfmacbook. All rights reserved.
//

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int PREV = 100;

Mat histEqualize(Mat src)
{
    Mat gray, eql;
    
    cvtColor(src, gray, CV_BGR2GRAY);
    
    equalizeHist(gray, eql);
    
    return eql;
}

void getFileList(std::vector<Mat>& src)
{
    //get folders
    std::vector<string> folders;
    
    std::vector<string> image_file;
    
    //get path of all test files
    string path = "CroppedYale/";
    
    //get pictures name and store in vector
    Directory dir;
    
    //get all .pgm files on current directory in unix system
    
    folders = dir.GetListFolders(path, "");
    
    for (int i = 0; i < folders.size(); i++) {
        std::vector<string> tmp = dir.GetListFiles(path + folders[i] + "/", ".pgm");
        
        for (int j = 0; j < tmp.size(); j++) {
            //image_file.push_back(tmp[j]);
            if (tmp[j].substr(tmp[j].length()-7, tmp[j].length()-1) == "+00.pgm") {
                src.push_back(histEqualize(imread(path + folders[i] + "/" + tmp[j])));
            }
        }
    }
    
    //src.push_back(histEqualize(imread(path+"myfolder/king.png")));
    //src.push_back(histEqualize(imread(path+"myfolder/me.png")));
}

int main(int argc, const char *argv[])
{
    
    if (argc < 3) {
        cout << "Argument lacks." << endl;
        return 1;
    }
    
    int M = 192, N = 168, K;
    
    vector<Mat> src;
    
    getFileList(src);
    
    //get file list
    K = (int)src.size();
    
    Mat eigenvectors;
    
    //read eigenvectors
    FileStorage fs(argv[2], FileStorage::READ);
    
    fs["Eigenvectors"] >> eigenvectors;
    
    //Mat
    Mat average;
    
    fs["Average"] >> average;
    
    Mat coordinate;
    
    fs["Coordinate"] >> coordinate;
    
    vector<Mat> eigenfaces;
    
    //read eigenfaces
    PREV = coordinate.rows;
    
    for (int t = 0; t < PREV; t++) {
        
        Mat tmpface;
        
        char str[15];
        sprintf(str, "eface%dth", t);
        
        fs[str] >> tmpface;
        
        eigenfaces.push_back(tmpface);
    }
    
    //测试部分
    Mat gray = imread(argv[1]);
    
    //show
    imshow("gray", gray);
    
    Mat qry;
    cvtColor(gray, qry, CV_BGR2GRAY);
    
    Mat line_qry(M * N, 1, CV_64FC1);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            line_qry.at<double>(i * N + j, 0) = qry.at<uchar>(i, j);
        }
    }
    
    //Projecting the query image into the PCA subspace.
    
    //100*mn mn*1
    Mat query(PREV, 1, CV_64FC1);
    
    query = eigenvectors.t() * (line_qry - average);
    
    Mat submat = Mat::zeros(M, N, CV_64FC1);
    for (int t = 0; t < PREV; t++) {
        double weight = query.at<double>(t, 0);
        submat += weight * eigenfaces[t];
    }
    

    submat.convertTo(submat, CV_8UC1);
    
    imwrite("submat.png", 0.5 * (qry + submat));
    
    
    //
    int min_id = 0;

    double min = norm(query - coordinate.col(0), NORM_L2);
    
    for (int i = 1; i < K; i++) {
        double form = norm(query - coordinate.col(i), NORM_L2);
        
        if (form < min) {
            min = form;
            min_id = i;
        }
    }
    
    //
    cout << min_id << endl;
    
    imshow("dst", src[min_id]);
    
    fs.release();
    
    waitKey();
    
    return 0;
}

