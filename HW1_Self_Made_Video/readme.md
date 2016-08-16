#Lab1 制作个人视频


##1. 实验内容
* 将输入的视频与照片处理成同样长宽后,合在一起生成一个视频
* 这个新视频中,编程生成一个片头,然后按幻灯片形式播放这些输 入照片,最后按视频原来速度播放输入的视频
* 新视频中要在底部打上含自己学号与姓名等信息的字幕
* 实验镜头的旋转切换效果
* 该工程在Mac OS 系统下用opencv-2.4.9实现。


##2. 算法具体步骤

###A) 获取当前目录下的相关文件--Directory类

* 介于程序的命令行格式，我们假设命令行参数`argv[1]`路径下只有唯一的`.avi`文件和若干`.jpg`文件，我们利用opencv的`cv::Directory`类来实现这一功能。该类的`GetListFiles`方法可以获取指定的类型文件。第一个参数为路径名，第二个参数为扩展名，注意Windows下用`*.jpg`，而Mac下用`.jpg`。执行完该函数后，返回`vector<string>`类型的批量文件名字符串，从而获取当前工作目录下的.avi和.jpg文件。

		//names of image files
	    vector<string> image_file;
	    
	    //name of video .avi
	    vector<string> video_file;
	    
	    //get path of all test files
	    string path = string(argv[1]) + "/";
	    
	    //get pictures name and store in vector
	    Directory dir;
	    
	    //get all .jpg files on current directory
	    image_file = dir.GetListFiles(path, ".jpg");
	    
	    //get video name
	    video_file = dir.GetListFiles(path, ".mp4");



###B) 获取视频信息

* 读取视频的基本原理就是读取图片，将视频流以图片形式一帧一帧读取到内存。读取视频这里用`cv::VideoCapture`类，用如下构造函数可以初始化一个对象:(`video_file[0]`是上一步读出的文件名。)
		
		//open the video test.avi
	    VideoCapture capture(video_file[0]);

* 用`isOpened`方法确定是否打开文件成功：

		//judge whether successfully open
    	if (!capture.isOpened()) {
        	return 1;
    	}

* 用`get`方法赋予不同的参数可以获取不同的属性值，如下为帧率，视频流的长宽:

		//get frame rate
	    double frame_rate = capture.get(CV_CAP_PROP_FPS);
	    
	    //get size
	    height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	    width = capture.get(CV_CAP_PROP_FRAME_WIDTH);



###C) 字体

* `CvFont`类实现字体功能，`cvInitFont`方法初始化字体：
		
		//Text
    	CvFont font;
    	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_HERSHEY_SCRIPT_SIMPLEX, 1.0, 1.0, 0, 1 );
    	
* `cvPutText`方法将字幕附加到图片中：

		cvPutText(img2, "3130100677 Zhuofei Huang", cvPoint(100,100), &font, cvScalar(255,0,0) );
		
	其中`cvScalar(255,0,0)`负责调整字体的颜色。



###D) 写入视频序列--CvVideoWriter类

* `CvVideoWriter`类实现写入视频序列的功能。写入视频的原理与读入类似，将图片一帧一帧地写回文件。其构造函数如下：

		CvVideoWriter* cwriter = cvCreateVideoWriter("res.avi", CV_FOURCC('D', 'I', 'V', 'X'), frame_rate, cvSize(width, height));

> 第一个参数：文件名
> 
> 第二个参数：解码格式，用MPEG-4，即`CV_FOURCC('D', 'I', 'V', 'X')`
> 
> 第三个参数：帧率，由读取视频时刻获取
> 
> 第四个参数：视频大小size，由读取视频时刻获取


* 首先要生成片头，片头由当前工作目录下的若干`.jpg`文件组合而成，要将这些图片处理成与读取的视频长宽相同。由于前面步骤B已经读取视频长宽保存在`weight`和`height`变量中，所以直接用：

		IplImage* img = cvLoadImage(image_file[i].c_str(), 1);
        IplImage* img2 = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
        
        //IplImage resize
        cvResize(img, img2, CV_INTER_LINEAR);

三行代码可以实现resize功能，将图片保存在img2指针中。其中注意在缩放过程中涉及到的需要填补空白像素的插值问题，这里选用线性插值`CV_INTER_LINEAR`。

* 这里对片头照片的设置是：每张照片停留3s，然后以自旋转效果切换到下一张图片。

(1) 停留3s：相当于往视频文件中写入`3*frame_size`张图片，代码为简单for循环即可

		for (int i = 0; i < 3 * frame_rate; i++) {
            cvWriteFrame(cwriter, img2);
        }

(2) 旋转：自定义旋转函数`rotateImage`，该函数实现图片旋转180度，每一次旋转6度，共30次，达到图片间的切换效果。该函数的前两个参数分别为输入和输出的图片IplImage指针，第三个参数为旋转角度。函数内部实现调用了`cv2DRotationMatrix`方法，并引入opencv的旋转变换矩阵进行计算。如下是单次旋转degree角度的代码：
	
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

整个旋转效果由一个for循环控制，旋转30次表示写入30帧图片，在视频中播放的效果是旋转过渡：

		IplImage* old = img2, *newImg = cvCreateImage(cvSize(img2->width, img2->height), IPL_DEPTH_8U, 3);
        
        //special efficacy
        for (int angle = 0; angle < 180; angle+=6) {
            
            //each time we rotate 2 degree
            rotateImage(old, newImg, 2);
            
            cvWriteFrame(cwriter, newImg);
            
            old = newImg;
         
        }



(3) 图像褶皱效果

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


* 写入读取的视频

	最后要将读取的视频写回目标视频文件中，此时要用到之前建立的`capture`对象来读取视频，`cwriter`对象来写视频。读取视频如下调用`capture.read(frame)`函数，写入视频文件可用`cvWriteFrame(cwriter, &pImg)`函数。写入视频前注意将字幕附加上去，调用`cvPutText`。
将字颜色设置成白色便于看清。
	
		bool stop = false;
	    Mat frame;//current video's frame
	    
    	int delay = 1000 / frame_rate;
    
	    while (!stop) {
	        //attempt to read next frame
	        if (!capture.read(frame))
	            break;
	        
	        IplImage pImg(frame);
	        
	        cvPutText(&pImg, "3130100677 Zhuofei Huang", cvPoint(100,100),&font, cvScalar(255,255,255));
	        
	        cvWriteFrame(cwriter, &pImg);
	        waitKey(delay);
	        
	    }
    
    	//close the video
    	capture.release();

