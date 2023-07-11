#include <string.h>
#include <ctime>
#include <iomanip>
#include <exception>
#include <sstream>
#include <streambuf>
#include <iterator>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <liblicense.h>
#include <aes256.h>

constexpr static unsigned char gKey[] = "n#If^*:Y4;-xH&<Ozj/Zybq]~@%,JC'o"; // custom

std::ostream &operator<<(std::ostream &os, const License &license) {
    auto expire_day = std::chrono::system_clock::to_time_t(license.m_expire);
    os << license.m_mac << " " << license.m_cpu_id << " " << license.m_disk_id << " " << license.m_scene_name << " " << license.m_scene_version << " " << std::put_time(std::localtime(&expire_day), "%F %T");

    return os;
}

std::ostream &operator<<(std::ostream &os, const LicenseCrypto &licCrypto) {
    aes256_context ctx;
    aes256_init(&ctx, gKey);

    std::ostringstream oss;
    oss << *licCrypto.m_pLic;
    std::string plaintext = oss.str();
    std::vector<unsigned char> ciphertext(16 * (plaintext.size() / 16 + 1), 0);
    std::copy(plaintext.begin(), plaintext.end(), ciphertext.begin());
    for (size_t i = 0; i < ciphertext.size(); i += 16)
        aes256_encrypt_ecb(&ctx, &ciphertext[i]);

    for (auto c : ciphertext)
        os << c;

    return os;
}

License::License(const std::string &strMac, const std::string &cpuId, const std::string &diskId, const std::string &sceneName, const std::string &sceneVersion, int month) :
    m_mac(strMac), m_cpu_id(cpuId), m_disk_id(diskId), m_scene_name(sceneName), m_scene_version(sceneVersion), m_expire(std::chrono::system_clock::now() + months{month}) {
}

License::License(std::istream &is) {
    std::tm tm = {};
    try {
        is >> m_mac >> m_cpu_id >> m_disk_id >> m_scene_name >> m_scene_version >> std::get_time(&tm, "%Y-%m-%d %T");
        m_expire = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    } catch (const std::exception &e) {
        std::cout << e.what();
    }
}

std::pair<bool, std::string> License::Check(const std::string &strMac, const std::string &cpuId, const std::string &diskId, const std::string &sceneName, const std::string &sceneVersion) const {
    if (strMac != m_mac)
        return std::make_pair(false, "The MAC Address does not match.");

    if (cpuId != m_cpu_id)
        return std::make_pair(false, "The CPU ID does not match.");

    if (diskId != m_disk_id)
        return std::make_pair(false, "The Disk ID does not match.");

    if (sceneName != m_scene_name)
        return std::make_pair(false, "The Scene Name does not match.");

    if (sceneVersion != m_scene_version)
        return std::make_pair(false, "The Scene Version does not match.");

    if (std::chrono::system_clock::now() > m_expire)
        return std::make_pair(false, "The license has expired.");

    return std::make_pair(true, "success");
}

LicenseCrypto::LicenseCrypto(std::istream &is) {
    aes256_context ctx;
    aes256_init(&ctx, gKey);

    try {
        std::vector<unsigned char> ciphertext((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        for (size_t i = 0; i < ciphertext.size(); i += 16)
            aes256_decrypt_ecb(&ctx, &ciphertext[i]);
        std::string plaintext(ciphertext.begin(), ciphertext.end());
        std::istringstream iss(plaintext);
        m_pLic = std::make_shared<License>(iss);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}
