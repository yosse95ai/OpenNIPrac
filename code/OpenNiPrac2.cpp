#include <iostream>
#include <stdexcept>
#include <OpenNI.h>
#include <opencv2/opencv.hpp>

class DepthSensor
{
public:
    void initialize()
    {
        // デバイスを取得
        openni::Status ret = device.open(openni::ANY_DEVICE);
        if (ret != openni::STATUS_OK) {
            throw std::runtime_error("openni::Device::open() failed.");
        }

        // カラーストリームを有効にする
        colorStream.create(device, openni::SENSOR_COLOR);
        colorStream.start();

        // Depthストリームを有効にする
        depthStream.create(device, openni::SENSOR_DEPTH);
        depthStream.start();
    }

    // フレームの更新処理
    void update()
    {
        openni::VideoFrameRef colorFrame;
        openni::VideoFrameRef depthFrame;

        // 更新されたフレームの取得
        colorStream.readFrame(&colorFrame);
        depthStream.readFrame(&depthFrame);

        // フレームデータを表示できる形に変換する
        colorImage = showColorStream(colorFrame);
        depthImage = showDepthStream(depthFrame);

        // フレームデータを表示する
        cv::imshow("Color Stream", colorImage);
        cv::imshow("Depth Stream", depthImage);
    }

private:
    void changeResolutioin(openni::VideoStream& stream)
    {
        openni::VideoMode mode = stream.getVideoMode();
        mode.setResolution(640, 480);
        mode.setFps(30);
        stream.setVideoMode(mode);
    }

    // カラーストリームを表示できる形に変換する
    cv::Mat showColorStream(const openni::VideoFrameRef& colorFrame)
    {
        // OpeCVの形にする
        cv::Mat colorImage = cv::Mat( colorFrame.getHeight(), colorFrame.getWidth(),
            CV_8UC3, (unsigned char*)colorFrame.getData());

        // BGR 2 RGB
        cv::cvtColor(colorImage, colorImage, cv::COLOR_RGB2BGR);

        return colorImage;
    }

    // Depthストリームを表示できる形に変換する
    cv::Mat showDepthStream(const openni::VideoFrameRef& depthFrame)
    {
        // 距離データを画像化する(16bits)
        cv::Mat depthImage = cv::Mat(depthFrame.getHeight(),
            depthFrame.getWidth(),
            CV_16UC1, ((unsigned short*)depthFrame.getData()));

        // 0~10000mmまでのデータを0~255に正規化
        depthImage.convertTo(depthImage, CV_8U, 255.0 / 10000);

        // 中心点の距離を表示
        showCenterDistance(depthImage, depthFrame);

        return depthImage;
    }

    void showCenterDistance(cv::Mat& depthImage, const openni::VideoFrameRef& depthFrame)
    {
        openni::VideoMode videoMode = depthStream.getVideoMode();

        int centerX = videoMode.getResolutionX() / 2;
        int centerY = videoMode.getResolutionY() / 2;
        int centerIndex = (centerY * videoMode.getResolutionX()) + centerX;

        unsigned short* depth = (unsigned short*)depthFrame.getData();

        std::stringstream ss;
        ss << "Center Point :" << depth[centerIndex];
        cv::putText(depthImage, ss.str(), cv::Point(0, 50),
            cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255));
    }

    openni::Device device;              // 使用するデバイス
    openni::VideoStream colorStream;    // カラーストリーム
    openni::VideoStream depthStream;    //  Depthストリーム
    cv::Mat colorImage;                 // 表示用データ
    cv::Mat depthImage;                 // Depth表示用データ

};

int main(int argc, const char * argv[])
{
    try {
        // OpenNIを初期化
        openni::OpenNI::initialize();

        // センサーを初期化する
        DepthSensor sensor;
        sensor.initialize();

        // メインループ
        while (1) {
            sensor.update();
            int key = cv::waitKey(10);
            if (key == 'q') {
                break;
            }
        }
    }
    catch (std::exception&) {
        std::cout << openni::OpenNI::getExtendedError() << std::endl;
    }

    return 0;
}