#include <windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace std;

// Convert a wide string to lowercase
wstring toLower(const wstring& input) {
    wstring result = input;
    transform(result.begin(), result.end(), result.begin(), towlower);
    return result;
}

// Structure to store information about a file
struct FileInfo {
    wstring filename;
    wstring extension;
    uintmax_t size = 0;
    FILETIME creationTime = {};
    FILETIME modificationTime = {};
    DWORD attributes = 0;
};



// Function to recursively get all files in a directory
vector<FileInfo> GetFilesInDirectory(const wstring& directory) {
    vector<FileInfo> files;

    // Construct the search path with a wildcard to get all files in the specified directory
    wstring searchPath = directory + L"\\*.*"; // L stands for "wide" or "long" character literal
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        // Display an error message if the directory cannot be opened
        wcerr << L"Error opening directory: " << directory << endl;
        return files;
    }

    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // If the item is a directory, recursively call GetFilesInDirectory
            // Skip the special entries "." and ".."
            if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
                wstring subdirectory = directory + L"\\" + findFileData.cFileName;
                vector<FileInfo> subdirectoryFiles = GetFilesInDirectory(subdirectory);
                files.insert(files.end(), subdirectoryFiles.begin(), subdirectoryFiles.end());
            }
        }
        else {
            // If the item is a file, create a FileInfo structure and add it to the vector
            FileInfo fileInfo;
            fileInfo.filename = findFileData.cFileName;
            fileInfo.size = (static_cast<uintmax_t>(findFileData.nFileSizeHigh) << 32) | findFileData.nFileSizeLow;
            fileInfo.creationTime = findFileData.ftCreationTime;
            fileInfo.modificationTime = findFileData.ftLastWriteTime;
            fileInfo.attributes = findFileData.dwFileAttributes;

            // Extract file extension using rfind to find the last occurrence of '.'
            size_t pos = fileInfo.filename.rfind(L'.'); // rfind is a member function of the std::wstring class, it is used to find the last occurrence of a substring in a string
            if (pos != wstring::npos) {
                // If a dot is found, extract the extension
                fileInfo.extension = fileInfo.filename.substr(pos + 1); // pos is not equal to std::wstring::npos, it means that a period was found
            }

            // Add the FileInfo structure to the vector
            files.push_back(fileInfo);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        // Display an error message if there was an issue reading the directory
        wcerr << L"Error reading directory: " << directory << endl;
    }

    // Close the search handle
    FindClose(hFind);
    return files;
}


// Predicate to check if a file's name contains the specified partial name
bool PartialNamePredicate(const WIN32_FIND_DATA& findFileData, const wstring& filePath, const wstring& partialName) {
    wstring fileName = findFileData.cFileName;
    wstring lowercaseFileName = toLower(fileName);
    wstring lowercasePartialName = toLower(partialName);

    // Check if the lowercase file name contains the lowercase partial name
    return lowercaseFileName.find(lowercasePartialName) != wstring::npos;
}

// Predicate to check if a file's extension matches the specified extension
bool ExtensionPredicate(const WIN32_FIND_DATA& findFileData, const wstring& filePath, const wstring& extension) {
    if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { //constant that represents the attribute indicating that a file is a directory (folder) rather than a regular file
        // If the item is a file, extract the file extension and compare it to the specified extension
        size_t pos = filePath.rfind(L'.');
        if (pos != wstring::npos) {
            // If a dot is found, extract the extension and compare it (case-insensitive)
            wstring fileExtension = filePath.substr(pos + 1);
            return toLower(fileExtension) == toLower(extension);
        }
    }
    // If the item is a directory or has no extension, return false
    return false;
}

// Template function to search files based on a custom predicate
template <typename Predicate>
vector<wstring> SearchFiles(const wstring& directory, Predicate predicate) {
    vector<wstring> matchingFiles;

    // Construct the search path with a wildcard to get all files in the specified directory
    wstring searchPath = directory + L"\\*.*";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        // Display an error message if the directory cannot be opened
        wcerr << L"Error opening directory: " << directory << endl;
        return matchingFiles;
    }

    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // If the item is a directory, recursively call SearchFiles
            // Skip the special entries "." and ".."
            if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) { //wcscmp is a wide-character string comparison function, cFileName represents the name of the found file or directory when using functions like FindFirstFile and FindNextFile
                wstring subdirectory = directory + L"\\" + findFileData.cFileName;
                vector<wstring> subdirectoryFiles = SearchFiles(subdirectory, predicate);
                matchingFiles.insert(matchingFiles.end(), subdirectoryFiles.begin(), subdirectoryFiles.end());
            }
        }
        else {
            // If the item is a file, create the file path and check if it matches the predicate
            wstring fileName = findFileData.cFileName;
            wstring filePath = directory + L"\\" + fileName;

            // Check if the file matches the predicate
            if (predicate(findFileData, filePath)) {
                matchingFiles.push_back(filePath);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        // Display an error message if there was an issue reading the directory
        wcerr << L"Error reading directory: " << directory << endl;
    }

    // Close the search handle
    FindClose(hFind);
    return matchingFiles;
}

/* This method points out that there is no options to find file by creation date only
bool CreationTimePredicate(const WIN32_FIND_DATA& findFileData, const wstring& filePath, const FILETIME& creationTimeToFind) {
    int comparisonResult = CompareFileTime(&findFileData.ftCreationTime, &creationTimeToFind);
    wcout << L"Comparison result: " << comparisonResult << endl;
    return comparisonResult == 0;
}


vector<wstring> SearchFilesByCreationTime(const wstring& directory) {
    vector<wstring> matchingFiles;

    // Specify the desired creation time for searching
    SYSTEMTIME creationTimeToFind = { 2023, 11, 4, 16, 0, 0, 0, 0 }; //Year, month, weekday (Sun == 0), day, hour, min, sec, millisec
    FILETIME creationTime;

    wstring searchPath = directory + L"\\*.*";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        wcerr << L"Error opening directory: " << directory << endl;
        return matchingFiles;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            // Check if the file's creation time matches the specified time
            SystemTimeToFileTime(&creationTimeToFind, &creationTime);
            wcout << L"File: " << directory + L"\\" + findFileData.cFileName << endl;
            wcout << L"File creation time: " << findFileData.ftCreationTime.dwLowDateTime << L", " << findFileData.ftCreationTime.dwHighDateTime << endl;
            wcout << L"Specified creation time: " << creationTime.dwLowDateTime << L", " << creationTime.dwHighDateTime << endl;
            if (CreationTimePredicate(findFileData, directory + L"\\" + findFileData.cFileName, creationTime)) {
                matchingFiles.push_back(directory + L"\\" + findFileData.cFileName);
            }
        }
        else if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
            wstring subdirectory = directory + L"\\" + findFileData.cFileName;
            vector<wstring> subdirectoryFiles = SearchFilesByCreationTime(subdirectory);
            matchingFiles.insert(matchingFiles.end(), subdirectoryFiles.begin(), subdirectoryFiles.end());
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        wcerr << L"Error reading directory: " << directory << endl;
    }

    FindClose(hFind);
    return matchingFiles;
} */

// Template function to sort files
template <typename CompareFunction>
void SortFiles(vector<FileInfo>& files, CompareFunction compareFunction) {
    // Use the std::sort function to sort the files based on the specified comparison function
    sort(files.begin(), files.end(), compareFunction);
}

int main() {
    // Specify the target directory
    wstring directory = L"C:\\Users\\User\\Desktop\\VIA_5_sem";

    // Specify the criteria for file search
    wstring partialName = L"cpp"; // Specify the partial file name for searching
    wstring extension = L"sql";  // Specify the desired extension for searching
    
    // Uncomment the respective lines to use particular search methods
    /*
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
    } */

    // Search files by extension
    
    vector<wstring> matchingFiles = SearchFiles(directory, [&](const WIN32_FIND_DATA& findFileData, const wstring& filePath) {
        return ExtensionPredicate(findFileData, filePath, extension);
        });
    wcout << L"Matching files searching by file extension:\n";
    
    wcout << L"Matching files searching by creation time:\n";
    for (const auto& file : matchingFiles) {
        wcout << file << endl;
    }
    
    // Check if matchingFiles is empty and display appropriate message
    if (matchingFiles.empty()) {
        wcout << L"No files matching the criteria found in the entire directory tree." << endl;
        wcout << endl; // Add an extra empty line
    }

    // Display retrieved file information using recursive method
    auto getFilesStart = chrono::high_resolution_clock::now();
    vector<FileInfo> files = GetFilesInDirectory(directory);
    /*
    wcout << L"\nFiles listed by recursive method:\n";
    */
    auto getFilesStop = chrono::high_resolution_clock::now();
    auto getFilesDuration = chrono::duration_cast<chrono::milliseconds>(getFilesStop - getFilesStart);
    // Display time taken for files listing
    wcout << L"GetFiles Time: " << getFilesDuration.count() << L" milliseconds\n\n";

    // Uncomment the respective lines to use particular sorting methods

    // Sort files by size
    /* SortFiles(files, [](const FileInfo& a, const FileInfo& b) {
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
    
    SortFiles(files, [](const FileInfo& a, const FileInfo& b) {
        return CompareFileTime(&a.modificationTime, &b.modificationTime) < 0;
    });
    wcout << L"\nFiles sorted by modification time:\n";
    
    /*
    // Sort files by extension
    auto sortStart = chrono::high_resolution_clock::now();
    SortFiles(files, [](const FileInfo& a, const FileInfo& b) {
        return toLower(a.extension) < toLower(b.extension);
        });
    auto sortStop = chrono::high_resolution_clock::now();
    auto sortDuration = chrono::duration_cast<chrono::milliseconds>(sortStop - sortStart);

    wcout << L"\nFiles sorted by extension:\n"; */

    wcout << endl; // Add an extra empty line
    for (const auto& file : files) {
        wprintf(L"File: %s\n", file.filename.c_str()); // wprintf to print out also non-ascii characters
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
