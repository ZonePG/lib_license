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
    std::chrono::system_clock::time_point m_expire;

public:
    License(const std::string &strMac, int month);
    License(std::istream &is);
    std::pair<bool, std::string> Check() const;
    //! @brief Output License object("MAC date time")
    friend std::ostream &operator<<(std::ostream &os, const License &license);
};

//! @brief Get local Netcard physical address.
std::string GetLocalMacAddr();

class LicenseCrypto {
    std::shared_ptr<License> m_pLic;
public:
    LicenseCrypto(const std::shared_ptr<License> &pLic) :
        m_pLic(pLic) {
    }
    LicenseCrypto(std::istream &is);
    std::pair<bool, std::string> Check() const {
        return m_pLic->Check();
    }
    //! @brief Output LicenseCrypto object("Passphrase")
    friend std::ostream &operator<<(std::ostream &os, const LicenseCrypto &licCrypto);
};
