#include <windows.h>
#include <iostream>
#include <vector>

using namespace std;

struct FileInfo {
    wstring filename;
    wstring extension;
    ULONGLONG size = 0;  // Initialize to 0
    FILETIME creationTime = {};  // Initialize to empty FILETIME
    FILETIME modificationTime = {};  // Initialize to empty FILETIME
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
            // Ignore "." and ".." directories
            if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
                // Recursively get files from subdirectory
                wstring subdirectory = directory + L"\\" + findFileData.cFileName;
                vector<FileInfo> subdirectoryFiles = GetFilesInDirectory(subdirectory);
                files.insert(files.end(), subdirectoryFiles.begin(), subdirectoryFiles.end());
            }
        }
        else {
            FileInfo fileInfo;
            fileInfo.filename = findFileData.cFileName;
            fileInfo.size = (static_cast<ULONGLONG>(findFileData.nFileSizeHigh) << 32) | findFileData.nFileSizeLow;
            fileInfo.creationTime = findFileData.ftCreationTime;
            fileInfo.modificationTime = findFileData.ftLastWriteTime;

            // Extract extension from filename
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

int main() {
    wstring directory = L"C:\\Users\\User\\Desktop\\VIA_5_sem";
    vector<FileInfo> files = GetFilesInDirectory(directory);

    // Display the retrieved file information
    for (const auto& file : files) {
        wcout << L"File: " << file.filename << endl;
        wcout << L"Extension: " << file.extension << endl;
        wcout << L"Size: " << file.size << L" bytes" << endl;

        // Convert FILETIME to local time and print
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


