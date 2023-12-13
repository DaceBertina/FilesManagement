#include <windows.h>
#include <iostream>
#include <chrono>
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

// Template function to search files based on a custom predicate
template <typename Predicate>
vector<wstring> SearchFiles(const wstring& directory, Predicate predicate) {
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
                vector<wstring> subdirectoryFiles = SearchFiles(subdirectory, predicate);
                matchingFiles.insert(matchingFiles.end(), subdirectoryFiles.begin(), subdirectoryFiles.end());
            }
        }
        else {
            wstring fileName = findFileData.cFileName;
            wstring filePath = directory + L"\\" + fileName;

            if (predicate(findFileData, filePath)) {
                matchingFiles.push_back(filePath);
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

// Predicate to check if a file's name contains the specified partial name
bool PartialNamePredicate(const WIN32_FIND_DATA& findFileData, const wstring& filePath, const wstring& partialName) {
    wstring fileName = findFileData.cFileName;
    wstring lowercaseFileName = toLower(fileName);
    wstring lowercasePartialName = toLower(partialName);
    return lowercaseFileName.find(lowercasePartialName) != wstring::npos;
}

// Predicate to check if a file's extension matches the specified extension
bool ExtensionPredicate(const WIN32_FIND_DATA& findFileData, const wstring& filePath, const wstring& extension) {
    if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        size_t pos = filePath.rfind(L'.');
        if (pos != wstring::npos) {
            wstring fileExtension = filePath.substr(pos + 1);
            return toLower(fileExtension) == toLower(extension);
        }
    }
    return false;
}

// Predicate to check if a file's creation time matches the specified time
bool CreationTimePredicate(const WIN32_FIND_DATA& findFileData, const wstring& filePath, const FILETIME& creationTime) {
    return CompareFileTime(&findFileData.ftCreationTime, &creationTime) == 0;
}

// Template function to sort files
template <typename CompareFunction>
void SortFiles(vector<FileInfo>& files, CompareFunction compareFunction) {
    sort(files.begin(), files.end(), compareFunction);
}

int main() {
    wstring directory = L"C:\\Users\\User\\Desktop\\VIA_5_sem";
    wstring partialName = L"aes_utils"; // Specify the partial file name for searching
    wstring extension = L"sql";  // Specify the desired extension for searching
    FILETIME creationTime = {};   // Specify the desired creation time for searching

    // Uncomment the respective lines to use particular search methods

    // Search files by partial name
    auto searchStart = chrono::high_resolution_clock::now();

    vector<wstring> matchingFiles = SearchFiles(directory, [&](const WIN32_FIND_DATA& findFileData, const wstring& filePath) {
        return PartialNamePredicate(findFileData, filePath, partialName);
    });

    auto searchStop = chrono::high_resolution_clock::now();
    auto searchDuration = chrono::duration_cast<chrono::milliseconds>(searchStop - searchStart);

    // Display time taken for searching
    wcout << L"Search Time: " << searchDuration.count() << L" milliseconds\n\n";

    wcout << L"Matching files searching by partial file name:\n";
    for (const auto& file : matchingFiles) {
        wcout << file << endl;
    }

        // Search files by extension
    /*
    vector<wstring> matchingFiles = SearchFiles(directory, [&](const WIN32_FIND_DATA& findFileData, const wstring& filePath) {
        return ExtensionPredicate(findFileData, filePath, extension);
        });
    wcout << L"Matching files searching by file extension:\n";
    */

    // Search files by creation time
    /*
    vector<wstring> matchingFiles = SearchFiles(directory, [&](const WIN32_FIND_DATA& findFileData, const wstring& filePath) {
        return CreationTimePredicate(findFileData, filePath, creationTime);
    });
    wcout << L"Matching files searching by creation time:\n";
    for (const auto& file : matchingFiles) {
        wcout << file << endl;
    }
    */
    // Check if matchingFiles is empty and display appropriate message
    if (matchingFiles.empty()) {
        wcout << L"No files matching the criteria found in the entire directory tree." << endl;
        wcout << endl; // Add an extra empty line
    }

    
    // Display retrieved file information using recursive method
    auto getFilesStart = chrono::high_resolution_clock::now();

    vector<FileInfo> files = GetFilesInDirectory(directory);
    wcout << L"\nFiles listed by recursive method:\n"; 

    auto getFilesStop = chrono::high_resolution_clock::now();
    auto getFilesDuration = chrono::duration_cast<chrono::milliseconds>(searchStop - searchStart);

    // Display time taken for searching
    wcout << L"Search Time: " << searchDuration.count() << L" milliseconds\n\n";

    // Uncomment the respective lines to use particular sorting methods

    // Sort files by size
    /*SortFiles(files, [](const FileInfo& a, const FileInfo& b) {
        return a.size < b.size;
        });
    wcout << L"\nFiles sorted by size:\n";
    */

    // Sort files by creation time
    /*
    SortFiles(files, [](const FileInfo& a, const FileInfo& b) {
        return CompareFileTime(&a.creationTime, &b.creationTime) < 0;
    });
    wcout << L"\nFiles sorted by creation time:\n";
    */

    // Sort files by modification time
    /*
    SortFiles(files, [](const FileInfo& a, const FileInfo& b) {
        return CompareFileTime(&a.modificationTime, &b.modificationTime) < 0;
    });
    wcout << L"\nFiles sorted by modification time:\n";
    */

    // Sort files by extension
    auto sortStart = chrono::high_resolution_clock::now();

    SortFiles(files, [](const FileInfo& a, const FileInfo& b) {
        return toLower(a.extension) < toLower(b.extension);
        });

    auto sortStop = chrono::high_resolution_clock::now();
    auto sortDuration = chrono::duration_cast<chrono::milliseconds>(sortStop - sortStart);
    auto sortDuration = chrono::duration_cast<chrono::milliseconds>(sortStop - sortStart);

    wcout << L"\nFiles sorted by extension:\n";

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
