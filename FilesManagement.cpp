#include <windows.h>
#include <iostream>
#include <vector>
#include <wchar.h>

using namespace std;

struct FileInfo {
    wstring filename;
    wstring extension;
    uintmax_t size = 0;
    FILETIME creationTime = {};
    FILETIME modificationTime = {};
    DWORD attributes = 0;
};

vector<FileInfo> GetFilesInDirectory(const wstring& directory) {
    vector<FileInfo> files;

    wstring searchPath = directory + L"\\*.*";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        wcerr << L"Error opening directory: " << directory << endl;
        return files;
    }

    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
                wstring subdirectory = directory + L"\\" + findFileData.cFileName;
                vector<FileInfo> subdirectoryFiles = GetFilesInDirectory(subdirectory);
                files.insert(files.end(), subdirectoryFiles.begin(), subdirectoryFiles.end());
            }
        }
        else {
            FileInfo fileInfo;
            fileInfo.filename = findFileData.cFileName;
            fileInfo.size = (static_cast<uintmax_t>(findFileData.nFileSizeHigh) << 32) | findFileData.nFileSizeLow;
            fileInfo.creationTime = findFileData.ftCreationTime;
            fileInfo.modificationTime = findFileData.ftLastWriteTime;
            fileInfo.attributes = findFileData.dwFileAttributes;

            size_t pos = fileInfo.filename.rfind(L'.');
            if (pos != wstring::npos) {
                fileInfo.extension = fileInfo.filename.substr(pos + 1);
            }

            files.push_back(fileInfo);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return files;
}

vector<wstring> SearchFilesByPartialName(const wstring& directory, const wstring& partialName) {
    vector<wstring> matchingFiles;

    wstring searchPath = directory + L"\\*.*";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        wcerr << L"Error opening directory: " << directory << endl;
        return matchingFiles;
    }

    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
                wstring subdirectory = directory + L"\\" + findFileData.cFileName;
                vector<wstring> subdirectoryFiles = SearchFilesByPartialName(subdirectory, partialName);
                matchingFiles.insert(matchingFiles.end(), subdirectoryFiles.begin(), subdirectoryFiles.end());
            }
        }
        else {
            wstring fileName = findFileData.cFileName;
            if (_wcsistr(fileName.c_str(), partialName.c_str()) != nullptr) {
                matchingFiles.push_back(directory + L"\\" + fileName);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return matchingFiles;
}

int main() {
    wstring directory = L"C:\\Users\\User\\Desktop\\VIA_5_sem";
    wstring partialName = L"cpp";

    // Display matching files using Windows API
    vector<wstring> matchingFiles = SearchFilesByPartialName(directory, partialName);
    wcout << L"Matching Files using Windows API:\n";
    for (const auto& file : matchingFiles) {
        wcout << file << endl;
    }

    // Display retrieved file information using recursive method
    vector<FileInfo> files = GetFilesInDirectory(directory);
    wcout << L"\nRecursive Method:\n";
    for (const auto& file : files) {
        wcout << L"File: " << file.filename << endl;
        wcout << L"Attributes: " << file.attributes << endl;
        wcout << L"Extension: " << file.extension << endl;
        wcout << L"Size: " << file.size << L" bytes" << endl;

        SYSTEMTIME creationSystemTime, modificationSystemTime;
        FileTimeToSystemTime(&file.creationTime, &creationSystemTime);
        FileTimeToSystemTime(&file.modificationTime, &modificationSystemTime);

        wcout << L"Creation Time: "
            << creationSystemTime.wYear << L"/" << creationSystemTime.wMonth << L"/" << creationSystemTime.wDay
            << L" " << creationSystemTime.wHour << L":" << creationSystemTime.wMinute << L":" << creationSystemTime.wSecond << endl;

        wcout << L"Modification Time: "
            << modificationSystemTime.wYear << L"/" << modificationSystemTime.wMonth << L"/" << modificationSystemTime.wDay
            << L" " << modificationSystemTime.wHour << L":" << modificationSystemTime.wMinute << L":" << modificationSystemTime.wSecond << endl;

        wcout << L"---------------------------" << endl;
    }

    return 0;
}
