#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

wstring toLower(const wstring& input) {
    wstring result = input;
    transform(result.begin(), result.end(), result.begin(), towlower);
    return result;
}

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

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        wcerr << L"Error reading directory: " << directory << endl;
    }

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

    wstring lowercasePartialName = toLower(partialName);

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
            wstring lowercaseFileName = toLower(fileName);

            if (lowercaseFileName.find(lowercasePartialName) != wstring::npos) {
                matchingFiles.push_back(directory + L"\\" + findFileData.cFileName);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        wcerr << L"Error reading directory: " << directory << endl;
    }

    FindClose(hFind);
    return matchingFiles;
}

vector<wstring> SearchFilesByExtension(const wstring& directory, const wstring& extension) {
    vector<wstring> matchingFiles;

    wstring searchPath = directory + L"\\*.*";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        wcerr << L"Error opening directory: " << directory << endl;
        return matchingFiles;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            wstring fileName = findFileData.cFileName;
            size_t pos = fileName.rfind(L'.');
            if (pos != wstring::npos) {
                wstring fileExtension = fileName.substr(pos + 1);
                if (toLower(fileExtension) == toLower(extension)) {
                    matchingFiles.push_back(directory + L"\\" + findFileData.cFileName);
                }
            }
        }
        else if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
            wstring subdirectory = directory + L"\\" + findFileData.cFileName;
            vector<wstring> subdirectoryFiles = SearchFilesByExtension(subdirectory, extension);
            matchingFiles.insert(matchingFiles.end(), subdirectoryFiles.begin(), subdirectoryFiles.end());
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        wcerr << L"Error reading directory: " << directory << endl;
    }

    FindClose(hFind);
    return matchingFiles;
}


// Function to sort files by size
bool CompareFileSize(const FileInfo& a, const FileInfo& b) {
    return a.size < b.size;
}

// Function to sort files by creation time
bool CompareFileCreationTime(const FileInfo& a, const FileInfo& b) {
    return CompareFileTime(&a.creationTime, &b.creationTime) < 0;
}

// Function to sort files by modification time
bool CompareFileModificationTime(const FileInfo& a, const FileInfo& b) {
    return CompareFileTime(&a.modificationTime, &b.modificationTime) < 0;
}

void SortFilesBySize(vector<FileInfo>& files) {
    sort(files.begin(), files.end(), CompareFileSize);
}

void SortFilesByCreationTime(vector<FileInfo>& files) {
    sort(files.begin(), files.end(), CompareFileCreationTime);
}

void SortFilesByModificationTime(vector<FileInfo>& files) {
    sort(files.begin(), files.end(), CompareFileModificationTime);
}

int main() {
    wstring directory = L"C:\\Users\\User\\Desktop\\VIA_5_sem";
    wstring partialName = L"sun"; // Specify the partial file name for searching
    wstring extension = L"sql";  // Specify the desired extension for searching

    // Uncomment the respective lines to use particular search methods

    // Search files by partial name
    /*
    vector<wstring> matchingFilesByName = SearchFilesByPartialName(directory, partialName);
    wcout << L"Matching files searching by partial file name:\n";
    for (const auto& file : matchingFilesByName) {
        wcout << file << endl;
    }
    */

    // Search files by extension
    vector<wstring> matchingFilesByExtension = SearchFilesByExtension(directory, extension);
    wcout << L"Matching files searching by file extension:\n";
    for (const auto& file : matchingFilesByExtension) {
        wcout << file << endl;
    }

    // Check if matchingFiles is empty and display appropriate message
    if (matchingFilesByExtension.empty()) {
        wcout << L"No files matching the criteria found in the entire directory tree." << endl;
        wcout << endl; // Add an extra empty line
    }

    // Display retrieved file information using recursive method
    vector<FileInfo> files = GetFilesInDirectory(directory);

    // Uncomment the respective lines to use particular sorting methods

    // Sort files by size
    /*
    SortFilesBySize(files);
    wcout << L"\nFiles sorted by size:\n";
    */

    // Sort files by creation time
    /*
    SortFilesByCreationTime(files);
    wcout << L"\nFiles sorted by creation time:\n";
    */

    // Sort files by modification time
    /*
    SortFilesByModificationTime(files);
    wcout << L"\nFiles sorted by modification time:\n";
    */
    wcout << endl; // Add an extra empty line
    for (const auto& file : files) {
        wprintf(L"File: %s\n", file.filename.c_str());
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
