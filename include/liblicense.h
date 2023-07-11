#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class License {
    using days = std::chrono::duration<int,
                                       std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>;
    using years = std::chrono::duration<int, std::ratio_multiply<std::ratio<146097, 400>, days::period>>;
    using months = std::chrono::duration<int, std::ratio_divide<years::period, std::ratio<12>>>;

    std::string m_mac;
    std::string m_cpu_id;
    std::string m_disk_id;
    std::string m_scene_name;
    std::string m_scene_version;
    std::chrono::system_clock::time_point m_expire;

public:
    License(const std::string &strMac, const std::string &cpuId, const std::string &diskId, const std::string &sceneName, const std::string &sceneVersion, int month);
    License(std::istream &is);
    std::pair<bool, std::string> Check(const std::string &cpuId, const std::string &diskId, const std::string &sceneName, const std::string &sceneVersion) const;
    friend std::ostream &operator<<(std::ostream &os, const License &license);
};

class LicenseCrypto {
    std::shared_ptr<License> m_pLic;

public:
    LicenseCrypto(const std::shared_ptr<License> &pLic) :
        m_pLic(pLic) {
    }
    LicenseCrypto(std::istream &is);
    std::pair<bool, std::string> Check(const std::string &cpuId, const std::string &diskId, const std::string &sceneName, const std::string &sceneVersion) const {
        return m_pLic->Check(cpuId, diskId, sceneName, sceneVersion);
    }
    friend std::ostream &operator<<(std::ostream &os, const LicenseCrypto &licCrypto);
};
