#include <iostream>
#include <vector>
#include <OpenNI.h>
#include <opencv2/opencv.hpp>

class DepthSensor
{
public:
    void initialize(const char* uri = openni::ANY_DEVICE)
    {
        // デバイスを取得
        openni::Status ret = device.open(uri);
        if (ret != openni::STATUS_OK) {
            throw std::runtime_error("openni::Device::open() failed.");
        }

        // URIを保存しておく
        this->uri = uri;
    }

    // フレームの更新処理
    void update()
    {


    }

private:

    openni::Device device;              // 使用するデバイス
    std::string uri;

};


class SampleApp
{
public:

    void initialize()
    {
        // 接続されているデバイス一覧を取得
        openni::Array<openni::DeviceInfo> deviceInfoList;
        openni::OpenNI::enumerateDevices(&deviceInfoList);

        // 接続されているデバイスの一覧を表示する
        std::cout << "接続されているデバイスの数：" << deviceInfoList.getSize() << std::endl;
        for (int i = 0; i < deviceInfoList.getSize(); ++i) {
            std::cout << deviceInfoList[i].getName() << ","
                << deviceInfoList[i].getVendor() << ","
                << deviceInfoList[i].getUri() << std::endl;
            openDevice(deviceInfoList[i].getUri());
        }
    }

    void update()
    {
        for (std::vector<DepthSensor*>::iterator it = sensors.begin(); it != sensors.end(); ++it) {
            (*it)->update();
        }
    }

private:

    void openDevice(const char* uri)
    {
        DepthSensor* sensor = new DepthSensor();
        sensor->initialize(uri);
        sensors.push_back(sensor);
    }

    std::vector<DepthSensor*> sensors;
};

int main(int argc, const char* argv[])
{
    try {
        // OpenNIを初期化
        openni::OpenNI::initialize();

        SampleApp app;
        app.initialize();
        while (1) {
            app.update();

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