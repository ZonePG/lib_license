#include <iostream>
#include <fstream>
#include "liblicense.h"

int main() {
    License license("08:bf:b8:19:39:2f", "CPU序列号", "磁盘序列号", "场景名称", "场景版本号", 2);
    std::cout << license << std::endl;

    LicenseCrypto licenseCrypto(std::make_shared<License>(license));

    std::ofstream file("license.lic");
    file << licenseCrypto;
    file.close();

    std::ifstream rFile("license.lic");
    LicenseCrypto rLicenseCrypto(rFile);
    auto result = rLicenseCrypto.Check("CPU序列号", "磁盘序列号", "场景名称", "场景版本号");
    std::cout << std::boolalpha << result.first << ": " << result.second << std::endl;
}