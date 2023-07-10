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

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

constexpr static unsigned char gKey[] = "n#If^*:Y4;-xH&<Ozj/Zybq]~@%,JC'o"; // custom

std::ostream &operator<<(std::ostream &os, const License &license) {
    auto expire_day = std::chrono::system_clock::to_time_t(license.m_expire);
    os << license.m_mac << " " << std::put_time(std::localtime(&expire_day), "%F %T");

    return os;
}

std::string GetLocalMacAddr() {
    int fd;
    struct ifreq ifr;
    char *iface = "eno1";
    unsigned char *mac = NULL;

    memset(&ifr, 0, sizeof(ifr));

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

    if (0 == ioctl(fd, SIOCGIFHWADDR, &ifr)) {
        mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

        // display mac address
        printf("Mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    close(fd);
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(buf);
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

License::License(const std::string &strMac, int month) :
    m_mac(strMac), m_expire(std::chrono::system_clock::now() + months{month}) {
}

License::License(std::istream &is) {
    std::tm tm = {};
    try {
        is >> m_mac >> std::get_time(&tm, "%Y-%m-%d %T");
        m_expire = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    } catch (const std::exception &e) {
        std::cout << e.what();
    }
}

std::pair<bool, std::string> License::Check() const {
    if (GetLocalMacAddr() != m_mac)
        return std::make_pair(false, "The MAC Address does not match.");

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
