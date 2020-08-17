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
        //cangeResolution(colorStream);
        colorStream.start();
    }

    // フレームの更新処理
    void update()
    {
        openni::VideoFrameRef colorFrame;

        // 更新されたフレームの取得
        colorStream.readFrame(&colorFrame);

        // フレームデータを表示できる形に変換する
        colorImage = showColorStream(colorFrame);

        // フレームデータを表示する
        cv::imshow("ColotStream", colorImage);
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


    openni::Device device;              // 使用するデバイス
    openni::VideoStream colorStream;    // カラーストリーム
    cv::Mat colorImage;                 // 表示用データ

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