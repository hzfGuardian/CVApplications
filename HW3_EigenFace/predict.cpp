
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
        cout << "Arguments lacks." << endl;
        return 1;
    }
    
    
    //get
    double percent;
    sscanf(argv[1], "%lf", &percent);

    std::vector<Mat> src;
    
    getFileList(src);
    

    //get the amount of samples
    int K = (int)src.size();
    
    //choose K*percent eigen vectors
    PREV = K * percent;
    
    //get height and width of this
    int M = src[0].rows, N = src[0].cols;
    
    //transfer the total matrix into one column
    
    //MN rows, K cols
    Mat X_matrix(M * N, K, CV_64FC1);
    
    for (int i = 0; i < M * N; i++) {
        for (int j = 0; j < K; j++) {
            X_matrix.at<double>(i, j) = src[j].at<uchar>(i / N,  i % N);//(row, col)
        }
    }
    
    
    //get miu
    Mat average;
    X_matrix.col(0).copyTo(average);

    for (int i = 1; i < K; i++) {
        average += X_matrix.col(i);
    }
    average /= K;
    
    FileStorage fs(argv[2], FileStorage::WRITE);
    
    fs << "Average" << average;
    
    //X-u
    for (int i = 0; i < K; i++) {
        X_matrix.col(i) -= average;
    }
    
    
    Mat eigenvalues(K, 1, CV_64FC1), eigenvectors(K, K, CV_64FC1);
    //eigen(cov_matrix, eigenvalues, eigenvectors);
    
    eigen(X_matrix.t() * X_matrix, eigenvalues, eigenvectors);
    
    //transform
    eigenvectors = eigenvectors.t();
    
    //calculate real eigen vectors(d*100) d=MN
    Mat real_eig_vectors(M * N, PREV, CV_64FC1);
    
    for (int i = 0; i < PREV; i++) {
        real_eig_vectors.col(i) = X_matrix * eigenvectors.col(i);
        real_eig_vectors.col(i) /= norm(real_eig_vectors.col(i), NORM_L2);
    }
    
    //store the eigen vectors by decreasing order
    fs << "Eigenvectors" << real_eig_vectors;
    

    //2*5 * M*N
    Mat total_eigenface(2 * M, 5 * N, CV_8UC1);
    
    std::vector<Mat> eigenfaces;
    
    for (int t = 0; t < PREV; t++) {
        
        Mat tmpface(M, N, CV_64FC1);
        
        //一维转二维
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                tmpface.at<double>(i, j) = real_eig_vectors.at<double>(i * N + j, t);
            }
        }
        
        Mat dst;
        Mat eigenface;
        
        normalize(tmpface, dst, 0, 255, NORM_MINMAX);
        
        dst.convertTo(eigenface, CV_8UC1);
        
        //save
        eigenfaces.push_back(tmpface);
        
        char str[15];
        sprintf(str, "eface%dth", t);

        fs << str << tmpface;
        
        //merge to one window
        //start x, y
        if (t >= 10) {
            continue;
        }
        
        int startX = t / 5 * M, startY = (t % 5) * N;
        
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                total_eigenface.at<uchar>(startX + i, startY + j) = eigenface.at<uchar>(i, j);
            }
        }

    }
    
    //ten eigenfaces shown in one big window
    imshow("Eigenfaces", total_eigenface);
    
    //计算样本K张图片在新坐标系下的坐标
    Mat coordinate(PREV, K, CV_64FC1);
    for (int i = 0; i < K; i++) {
        coordinate.col(i) = real_eig_vectors.t() * (X_matrix.col(i) - average);
    }
    
    //store the coordinate
    fs << "Coordinate" << coordinate;
    
    fs.release();
    
    waitKey();
    
    return 0;
}






