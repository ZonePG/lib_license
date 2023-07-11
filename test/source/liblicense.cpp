#include <doctest/doctest.h>
#include <liblicense/version.h>
#include <liblicense.h>

#include <string>
#include <fstream>

TEST_CASE("encrypt license") {
    License license("08:bf:b8:19:39:2f", "CPU序列号1", "磁盘序列号2", "场景名称3", "场景版本号4", 2);
    LicenseCrypto licenseCrypto(std::make_shared<License>(license));

    std::ofstream file("license.lic");
    file << licenseCrypto;
    file.close();
}

TEST_CASE("dencrypt license") {
    std::ifstream rFile("license.lic");
    LicenseCrypto rLicenseCrypto(rFile);
    auto result = rLicenseCrypto.Check("CPU序列号1", "磁盘序列号2", "场景名称3", "场景版本号4");
    CHECK(result.first);
}

TEST_CASE("encrypt license mac fixed") {
    License license("08:bf:b8:19:39:2e", "CPU序列号1", "磁盘序列号2", "场景名称3", "场景版本号4", 2);
    LicenseCrypto licenseCrypto(std::make_shared<License>(license));

    std::ofstream file("license_mac_fixed.lic");
    file << licenseCrypto;
    file.close();
}

TEST_CASE("dencrypt license mac failed") {
    std::ifstream rFile("license_mac_fixed.lic");
    LicenseCrypto rLicenseCrypto(rFile);
    auto result = rLicenseCrypto.Check("CPU序列号1", "磁盘序列号2", "场景名称3", "场景版本号4");
    CHECK(!result.first);
}

TEST_CASE("encrypt license outdated") {
    License license("08:bf:b8:19:39:2f", "CPU序列号1", "磁盘序列号2", "场景名称3", "场景版本号4", 0);
    LicenseCrypto licenseCrypto(std::make_shared<License>(license));

    std::ofstream file("license_outdated.lic");
    file << licenseCrypto;
    file.close();
}

TEST_CASE("dencrypt outdated license") {
    std::ifstream rFile("license_outdated.lic");
    LicenseCrypto rLicenseCrypto(rFile);
    auto result = rLicenseCrypto.Check("CPU序列号1", "磁盘序列号2", "场景名称3", "场景版本号4");
    CHECK(!result.first);
}
