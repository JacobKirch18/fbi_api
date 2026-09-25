#include "client.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <winhttp.h>

#include <stdexcept>
#include <string>
#include <system_error>

#pragma comment(lib, "winhttp.lib")

constexpr wchar_t kRequestUrl[] =
    L"https://api.fbi.gov/wanted/v1/list?field_offices=";

[[noreturn]] void throwWindowsError(const char* operation) {
    const DWORD error = GetLastError();

    throw std::system_error(
        static_cast<int>(error),
        std::system_category(),
        operation
    );
}

// Closes the Windows handle automatically, including on exceptions.
class HttpHandle {
public:
    explicit HttpHandle(HINTERNET handle) noexcept
        : handle_(handle) {
    }

    ~HttpHandle() {
        if (handle_) {
            WinHttpCloseHandle(handle_);
        }
    }

    HttpHandle(const HttpHandle&) = delete;
    HttpHandle& operator=(const HttpHandle&) = delete;

    HINTERNET get() const noexcept {
        return handle_;
    }

private:
    HINTERNET handle_;
};

// Encodes the UTF-8 bytes of one query parameter value.
std::wstring encodeQueryValue(const std::string& value) {
    constexpr wchar_t hex[] = L"0123456789ABCDEF";
    std::wstring encoded;

    for (unsigned char c : value) {
        const bool unreserved =
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~';

        if (unreserved) {
            encoded += static_cast<wchar_t>(c);
        }
        else {
            encoded += L'%';
            encoded += hex[c >> 4];
            encoded += hex[c & 0x0F];
        }
    }

    return encoded;
}

ApiClient::Response ApiClient::request(
    const std::string& query
) const {
    const std::wstring url =
        std::wstring(kRequestUrl) + encodeQueryValue(query);

    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    parts.dwHostNameLength = static_cast<DWORD>(-1);
    parts.dwUrlPathLength = static_cast<DWORD>(-1);
    parts.dwExtraInfoLength = static_cast<DWORD>(-1);

    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &parts)) {
        throwWindowsError("WinHttpCrackUrl");
    }

    if (parts.nScheme != INTERNET_SCHEME_HTTP &&
        parts.nScheme != INTERNET_SCHEME_HTTPS) {
        throw std::invalid_argument(
            "Endpoint must use HTTP or HTTPS"
        );
    }

    const std::wstring host(
        parts.lpszHostName,
        parts.dwHostNameLength
    );

    std::wstring target = L"/";

    if (parts.dwUrlPathLength > 0) {
        target.assign(
            parts.lpszUrlPath,
            parts.dwUrlPathLength
        );
    }

    if (parts.dwExtraInfoLength > 0) {
        target.append(
            parts.lpszExtraInfo,
            parts.dwExtraInfoLength
        );
    }

    HttpHandle session(WinHttpOpen(
        L"ApiClient/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    ));

    if (!session.get()) {
        throwWindowsError("WinHttpOpen");
    }

    // Resolve, connect, send, and receive timeouts in milliseconds.
    if (!WinHttpSetTimeouts(
        session.get(),
        10000,
        10000,
        30000,
        30000)) {
        throwWindowsError("WinHttpSetTimeouts");
    }

    HttpHandle connection(WinHttpConnect(
        session.get(),
        host.c_str(),
        parts.nPort,
        0
    ));

    if (!connection.get()) {
        throwWindowsError("WinHttpConnect");
    }

    const DWORD flags =
        parts.nScheme == INTERNET_SCHEME_HTTPS
        ? WINHTTP_FLAG_SECURE
        : 0;

    HttpHandle requestHandle(WinHttpOpenRequest(
        connection.get(),
        L"GET",
        target.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags
    ));

    if (!requestHandle.get()) {
        throwWindowsError("WinHttpOpenRequest");
    }

    if (!WinHttpSendRequest(
        requestHandle.get(),
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0)) {
        throwWindowsError("WinHttpSendRequest");
    }

    if (!WinHttpReceiveResponse(
        requestHandle.get(),
        nullptr)) {
        throwWindowsError("WinHttpReceiveResponse");
    }

    DWORD status = 0;
    DWORD statusSize = sizeof(status);

    if (!WinHttpQueryHeaders(
        requestHandle.get(),
        WINHTTP_QUERY_STATUS_CODE |
        WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &status,
        &statusSize,
        WINHTTP_NO_HEADER_INDEX)) {
        throwWindowsError("WinHttpQueryHeaders");
    }

    Response response;
    response.statusCode = static_cast<unsigned int>(status);

    char buffer[8192];

    for (;;) {
        DWORD bytesRead = 0;

        if (!WinHttpReadData(
            requestHandle.get(),
            buffer,
            static_cast<DWORD>(sizeof(buffer)),
            &bytesRead)) {
            throwWindowsError("WinHttpReadData");
        }

        if (bytesRead == 0) {
            break;
        }

        response.body.append(buffer, bytesRead);
    }

    return response;
}