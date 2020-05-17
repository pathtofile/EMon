#include "sealighter_krabs.h"
#include <sstream>
#include <fstream>
#include <codecvt>
#include "sealighter_json.h"
#include "sealighter_util.h"


std::string convert_json_string
(
    json item,
    bool pretty_print
)
{
    if (pretty_print) {
        return item.dump(4, ' ', false, nlohmann::detail::error_handler_t::replace);
    }
    else {
        return item.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);
    }
}

std::string convert_str_str_lowercase(
    const std::string& from
)
{
    std::string to = from;
    std::transform(to.begin(), to.end(), to.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return to;
}


std::wstring convert_wstr_wstr_lowercase(
    const std::wstring& from
)
{
    std::wstring to = from;
    std::transform(to.begin(), to.end(), to.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return to;
}

std::string convert_wstr_str
(
    const std::wstring& from
)
{
    std::string to(from.begin(), from.end());
    return to;
}


std::wstring convert_str_wstr
(
    const std::string& from
)
{
    std::wstring to(from.begin(), from.end());
    return to;
}

std::wstring convert_str_wstr_lowercase(
    const std::string& from
)
{
    std::string from_lower = convert_str_str_lowercase(from);
    std::wstring to = convert_str_wstr(from_lower);
    return to;
}

std::vector<BYTE> convert_str_bytes_lowercase(
    const std::string& from
)
{
    std::string from_lower = convert_str_str_lowercase(from);
    std::vector<BYTE> to(from_lower.begin(), from_lower.end());
    return to;
}

std::vector<BYTE> convert_str_wbytes_lowercase(
    const std::string& from
)
{
    std::wstring from_wide_lower = convert_str_wstr_lowercase(from);
    BYTE* from_bytes = (BYTE*)from_wide_lower.c_str();
    // Size returns string len, so double as they are widechars
    // But don't copy in trailing NULL so we can match mid-string
    size_t from_bytes_size = from_wide_lower.size() * sizeof(WCHAR);

    std::vector<BYTE> to(from_bytes, from_bytes + from_bytes_size);
    return to;
}

std::string convert_guid_str
(
    const GUID& from
)
{
    char guid_string[39]; // 2 braces + 32 hex chars + 4 hyphens + null terminator
    snprintf(
        guid_string, sizeof(guid_string),
        "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        from.Data1, from.Data2, from.Data3,
        from.Data4[0], from.Data4[1], from.Data4[2],
        from.Data4[3], from.Data4[4], from.Data4[5],
        from.Data4[6], from.Data4[7]);
    std::string to(guid_string);
    return to;
}

std::string convert_timestamp_string
(
    const LARGE_INTEGER from
)
{
    // Both LARGE_INTEGER timestamps and FILETIMES are
    // "100-nanosecond intervals since midnight, January 1, 1601"
    FILETIME ft;

    ft.dwHighDateTime = from.HighPart;
    ft.dwLowDateTime = from.LowPart;
    
    std::string to = convert_filetime_string(ft);
    return to;
}

std::string convert_filetime_string
(
    const FILETIME from
)
{
    SYSTEMTIME stime;
    ::FileTimeToSystemTime(std::addressof(from), std::addressof(stime));
    std::string to = convert_systemtime_string(stime);
    return to;
}


std::string convert_systemtime_string
(
    const SYSTEMTIME from
)
{
    std::ostringstream stm;
    const auto w2 = std::setw(2);
    stm << std::setfill('0') << std::setw(4) << from.wYear << '-' << w2 << from.wMonth
        << '-' << w2 << from.wDay << ' ' << w2 << from.wHour
        << ':' << w2 << from.wMinute << ':' << w2 << from.wSecond << 'Z';

    std::string to = stm.str();
    return to;
}


std::string convert_bytes_sidstring
(
    const std::vector<BYTE>& from
)
{
#define MAX_NAME 256
    char domain_name[MAX_NAME] = "";
    char user_name[MAX_NAME] = "";
    DWORD user_name_size = MAX_NAME;
    DWORD domain_name_size = MAX_NAME;
    SID_NAME_USE name_use;
    const BYTE* data = (from.data());
    std::string to;
    if (LookupAccountSidA(NULL, (PSID)data, user_name, &user_name_size, domain_name, &domain_name_size, &name_use)) {
        to = domain_name;
        to += "\\";
        to += user_name;
    }
    else {
        // Fallback to printing the raw bytes
        to = convert_bytes_hexstring(from);
    }
    return to;
}


std::string convert_bytes_hexstring
(
    const std::vector<BYTE>& from
)
{
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    for (int c : from) {
        ss << std::setw(2) << c;
    }

    std::string to = ss.str();
    return to;
}


int convert_bytes_sint32
(
    const std::vector<BYTE>& from
)
{
    int to = 0;
    if (from.size() == 4) {
        to = (from[3] << 24) | (from[2] << 16) | (from[1] << 8) | (from[0]);
    }
    return to;
}

bool convert_bytes_bool
(
    const std::vector<BYTE>& from
)
{
    if (convert_bytes_sint32(from)) {
        return true;
    }
    return false;
}


bool file_exists
(
    std::string fileName
)
{
    std::ifstream infile(fileName);
    return infile.good();
}
