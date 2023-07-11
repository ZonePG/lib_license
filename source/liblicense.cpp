#include <algorithm>
#include <string.h>
#include <ctime>
#include <iomanip>
#include <exception>
#include <sstream>
#include <streambuf>
#include <iterator>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <liblicense.h>
#include <aes256.h>
#include <vector>
#include <dirent.h>
#include <fstream>

constexpr static unsigned char gKey[] = "n#If^*:Y4;-xH&<Ozj/Zybq]~@%,JC'o"; // custom

std::ostream &operator<<(std::ostream &os, const License &license) {
    auto expire_day = std::chrono::system_clock::to_time_t(license.m_expire);
    os << license.m_mac << " " << license.m_scene_name << " " << license.m_scene_version << " " << std::put_time(std::localtime(&expire_day), "%F %T");

    return os;
}

std::vector<std::string> GetAllFiles(const std::string &folder, bool recursive = false) {
    // uses opendir, readdir, and struct dirent.
    std::vector<std::string> files;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folder.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string name = ent->d_name;
            if (name == "." || name == "..")
                continue;
            files.push_back(folder + name);
            if (recursive && ent->d_type == DT_DIR) {
                std::vector<std::string> more = GetAllFiles(folder + name + "/", recursive);
                files.insert(files.end(), more.begin(), more.end());
            }
        }
        closedir(dir);
    }

    return files;
}

bool ReadFileContents(const std::string &folder, const std::string &fname, std::string &contents) {
    // uses ifstream to read entire contents
    std::ifstream ifs(folder + "/" + fname);
    if (ifs.is_open()) {
        ifs >> contents;
        return true;
    }
    return false;
}

std::vector<std::string> GetAllMacAddresses() {
    std::vector<std::string> macs;
    std::string address;

    // from: https://stackoverflow.com/questions/9034575/c-c-linux-mac-address-of-all-interfaces
    //  ... just read /sys/class/net/eth0/address

    // NOTE: there may be more than one: /sys/class/net/*/address
    //  (1) so walk /sys/class/net/* to find the names to read the address of.

    std::vector<std::string> nets = GetAllFiles("/sys/class/net/", false);
    for (auto it = nets.begin(); it != nets.end(); ++it) {
        // we don't care about the local loopback interface
        if (0 == strcmp((*it).substr(it->size() - 3).c_str(), "/lo"))
            continue;
        address.clear();
        if (ReadFileContents(*it, "address", address)) {
            if (!address.empty()) {
                std::transform(address.begin(), address.end(), address.begin(), ::tolower);
                macs.push_back(address);
            }
        }
    }
    return macs;
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

License::License(const std::string &strMac, const std::string &sceneName, const std::string &sceneVersion, int month) :
    m_mac(strMac), m_scene_name(sceneName), m_scene_version(sceneVersion), m_expire(std::chrono::system_clock::now() + months{month}) {
    std::transform(m_mac.begin(), m_mac.end(), m_mac.begin(), ::tolower);
}

License::License(std::istream &is) {
    std::tm tm = {};
    try {
        is >> m_mac >> m_scene_name >> m_scene_version >> std::get_time(&tm, "%Y-%m-%d %T");
        m_expire = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    } catch (const std::exception &e) {
        std::cout << e.what();
    }
}

std::pair<bool, std::string> License::Check(const std::string &sceneName, const std::string &sceneVersion) const {
    std::vector<std::string> all_macs = GetAllMacAddresses();
    if (std::find(all_macs.begin(), all_macs.end(), m_mac) == all_macs.end())
        return std::make_pair(false, "The MAC Address does not match.");

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
