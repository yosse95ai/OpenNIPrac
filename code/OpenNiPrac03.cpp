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

        // ストリーム情報を表示する
        std::cout << "Color Stream" << std::endl;
        showStreamParameter(colorStream);

        std::cout << "Depth Stream" << std::endl;
        showStreamParameter(depthStream);
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
        cv::Mat colorImage = cv::Mat(colorFrame.getHeight(), colorFrame.getWidth(),
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

    void showStreamParameter(openni::VideoStream& stream)
    {
        std::cout << "Cropping support : " << stream.isCroppingSupported() << std::endl;
        std::cout << "Horizontal FOV   : " << stream.getHorizontalFieldOfView() << std::endl;
        std::cout << "Vertical FOV     : " << stream.getVerticalFieldOfView() << std::endl;
        std::cout << "Max Pixel Value  : " << stream.getMaxPixelValue() << std::endl;
        std::cout << "Min PIxel Value  : " << stream.getMinPixelValue() << std::endl;
        
        const openni::SensorInfo& sensorInfo = stream.getSensorInfo();
        std::cout << "Sensor Type      : " << getSensorTypeToString(sensorInfo.getSensorType()) << std::endl;

        std::cout << std::endl;
        std::cout << "Supported VideoMode : " << std::endl;
        const openni::Array<openni::VideoMode>& videoModes = sensorInfo.getSupportedVideoModes();

        for (int i = 0; i < videoModes.getSize(); ++i) {
            std::cout << " ResolutionX : " << videoModes[i].getResolutionX() << std::endl;
            std::cout << " ResolutionY : " << videoModes[i].getResolutionY() << std::endl;
            std::cout << " FPS         : " << videoModes[i].getFps() << std::endl;
            std::cout << " PixelFormat : " << getPixelFormatToString(videoModes[i].getPixelFormat()) << std::endl;

            std::cout << std::endl;
        }

        openni::CameraSettings* cameraSettings = stream.getCameraSettings();

        if (cameraSettings != 0) {
            std::cout << "CameraSettings" << std::endl;
            std::cout << "Auto Exposure Enabled     : " << cameraSettings->getAutoExposureEnabled() << std::endl;
            std::cout << "Auto WhiteBalance Enabled : " << cameraSettings->getAutoWhiteBalanceEnabled() << std::endl;
            std::cout << "Exposure                  : " << cameraSettings->getExposure() << std::endl;
            std::cout << "Gain                      : " << cameraSettings->getGain() << std::endl;
        }

        std::cout << std::endl;
    }

    const char* getSensorTypeToString(openni::SensorType sensorType)
    {
        if (sensorType == openni::SensorType::SENSOR_COLOR) {
            return "SENSOR_COLOR";
        }
        else if (sensorType == openni::SensorType::SENSOR_DEPTH) {
            return "SENSOR_DEPTH";
        }
        else if (sensorType == openni::SensorType::SENSOR_IR) {
            return "SENSOR_IR";
        }
        
        return "Unknown SensorType";
    }

    const char* getPixelFormatToString(openni::PixelFormat pixelFormat)
    {
        // Depth
        if (pixelFormat == openni::PixelFormat::PIXEL_FORMAT_DEPTH_1_MM) {
            return "PIXEL_FORMAT_DEPTH_1_MM";
        }
        else if (pixelFormat == openni::PixelFormat::PIXEL_FORMAT_DEPTH_100_UM) {
            return "PIXEL_FORMAT_DEPTH_100_UM";
        }
        else if (pixelFormat == openni::PixelFormat::PIXEL_FORMAT_SHIFT_9_2) {
            return "PIXEL_FORMAT_SHIFT_9_2";
        }
        else if (pixelFormat == openni::PixelFormat::PIXEL_FORMAT_SHIFT_9_3) {
            return "PIXEL_FORMAT_SHIFT_9_3";
        }
        // Color
        else if (pixelFormat == openni::PixelFormat::PIXEL_FORMAT_RGB888) {
            return "PIXEL_FORMAT_RGB888";
        }
        else if (pixelFormat == openni::PixelFormat::PIXEL_FORMAT_YUV422) {
            return "PIXEL_FORMAT_YUV422";
        }
        else if (pixelFormat == openni::PixelFormat::PIXEL_FORMAT_GRAY16) {
            return "PIXEL_FORMAT_GRAY16";
        }
        else if (pixelFormat == openni::PixelFormat::PIXEL_FORMAT_GRAY8) {
            return "PIXEL_FORMAT_GRAY8";
        }
        else if (pixelFormat == openni::PixelFormat::PIXEL_FORMAT_JPEG) {
            return "PIXEL_FORMAT_JPEG";
        }
        else if(pixelFormat == openni::PixelFormat::PIXEL_FORMAT_YUYV){
            return "PIXEL_FORMAT_YUYV";
        }

        return "Unknown PixelFormat";

    }

    openni::Device device;              // 使用するデバイス
    openni::VideoStream colorStream;    // カラーストリーム
    openni::VideoStream depthStream;    //  Depthストリーム
    cv::Mat colorImage;                 // 表示用データ
    cv::Mat depthImage;                 // Depth表示用データ

};

int main(int argc, const char* argv[])
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