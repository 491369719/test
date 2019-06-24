#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QTextCodec>
#include <opencv2/highgui/highgui_c.h>
#pragma execution_character_set("utf-8")
using namespace cv;
using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}


MainWindow::~MainWindow()
{
    delete ui;
}

QImage cvMat2QImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        return QImage();
    }
}

int _System(const char * cmd, char *pRetMsg, int msg_len)
{
    FILE * fp;
    char * p = NULL;
    int res = -1;
    if (cmd == NULL || pRetMsg == NULL || msg_len < 0)
    {
        printf("Param Error!\n");
        return -1;
    }
    if ((fp = _popen(cmd, "r")) == NULL)
    {
        printf("Popen Error!\n");
        return -2;
    }
    else
    {
        memset(pRetMsg, 0, msg_len);

        while (fgets(pRetMsg, msg_len, fp) != NULL&&pRetMsg[0]==' ');
        printf("%s", pRetMsg);
        if ((res = _pclose(fp)) == -1)
        {
            printf("close popenerror!\n");
            return -3;
        }
        pRetMsg[strlen(pRetMsg) - 1] = '\0';
        return 0;
    }
}

void MainWindow::fun(QString path)
{
    double scale = 0.2;
    int max=0;
    std::string strpath = path.toLocal8Bit().toStdString();
    Mat src,src2,src3,binary_output;
    vector<Vec4i> hierarcy;
    vector<vector<Point>> contours;
    vector<Rect> boundRect(contours.size());  //定义外接矩形集合
    vector<RotatedRect> box(contours.size()); //定义最小外接矩形集合
    src=imread(strpath,1);
    src.copyTo(src2);
    Mat srcc = Mat::zeros(src.size(), CV_8UC1);
    cvtColor(src, binary_output, CV_BGR2HSV);
    inRange(binary_output, Scalar(80, 160, 103), Scalar(180, 255, 255), binary_output);
    findContours(binary_output, contours, hierarcy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    for (int i = 0; i<contours.size(); i++)
    {
        //绘制轮廓
        if(contourArea(contours[i])>=contourArea(contours[max])){
            max=i;
        }
    }
    drawContours(srcc, contours, max, Scalar(255), 1, 8, hierarcy);
    RotatedRect rect = minAreaRect(contours[max]);
        Point2f P[4];
        rect.points(P);
    for (int j = 0; j <= 3; j++){
        line(binary_output, P[j], P[(j + 1) % 4], Scalar(0,0,255), 1);
        line(srcc, P[j], P[(j + 1) % 4], Scalar(111), 2);
    }
    Size dst_sz(src.cols, src.rows);
    cv::Mat rot_mat = cv::getRotationMatrix2D(rect.center, rect.angle, 1.0);
    cv::warpAffine(src2, src2, rot_mat, dst_sz);
    src3=src2(Rect((rect.center.x-(rect.size.width/2)),(rect.center.y-(rect.size.height/2)),rect.size.width,rect.size.height));
    imshow("sss",src3);
    cvtColor(src3, src3, CV_BGR2HSV);
    inRange(src3, Scalar(0, 0, 130), Scalar(180, 130, 255), src3);
    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(src3, src3, MORPH_OPEN, element);
    subtract(Scalar(255,255,255),src3,src3);
    imshow( "Display window3", src3 );
    QImage aaa=cvMat2QImage(src3);
    aaa.save(QString("%1\67.jpg").arg(QCoreApplication::applicationDirPath()));//QImage保存方法
    char *cmd = "tesseract E:/Project/qt/build-Test2-Desktop_Qt_5_12_3_MinGW_64_bit-Debug/debug.jpg stdout -l chi_sim";
    char a8Result[128] = { 0 };
    int ret = 0;
    ret = _System(cmd, a8Result, sizeof(a8Result));
    printf("%s",a8Result);
    ui->label_4->setScaledContents(true);
    ui->label_4->setPixmap(QPixmap::fromImage(aaa));
    QString s1 =QString::fromUtf8(a8Result);
    ui->label_6->setText(s1);
}

void MainWindow::on_pushButton_clicked()
{

    QString path = QFileDialog::getOpenFileName(this, tr("选择图片"), ".", tr("Image Files(*.jpg *.png)"));
    QImage* img=new QImage;
		if(! ( img->load(path) ) ) //加载图像
		{
			QMessageBox::information(this,
										tr("打开图像失败"),
										tr("打开图像失败!"));
			delete img;
			return;
		}
		ui->yuantu->setScaledContents(true);
		ui->yuantu->setPixmap(QPixmap::fromImage(*img));
		fun(path);


}
