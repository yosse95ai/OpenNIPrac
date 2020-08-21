#include <iostream>
#include <OpenNI.h>


int main(int argc, const char* argv[])
{
    try {
        // OpenNIを初期化
        openni::OpenNI::initialize();

        // 接続されているデバイス一覧を取得
        openni::Array<openni::DeviceInfo> deviceInfoList;
        openni::OpenNI::enumerateDevices(&deviceInfoList);

        // 接続されているデバイスの一覧を表示する
        std::cout << "接続されているデバイスの数：" << deviceInfoList.getSize() << std::endl;
        for (int i = 0; i < deviceInfoList.getSize(); ++i) {
            std::cout << deviceInfoList[i].getName() << "," 
                << deviceInfoList[i].getVendor() << "," 
                << deviceInfoList[i].getUri() << std::endl;
        }
    }
    catch (std::exception&) {
        std::cout << openni::OpenNI::getExtendedError() << std::endl;
    }

    return 0;
}