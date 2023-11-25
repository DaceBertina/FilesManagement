#include <iostream>
#include <vector>
#include <filesystem>
#include <chrono>
#include <iomanip>

using namespace std;
namespace fs = std::filesystem;

struct FileInfo {
    wstring filename;
    wstring extension;
    uintmax_t size = 0; 
    fs::file_time_type creationTime = {}; 
    fs::file_time_type modificationTime = {};  
};

vector<FileInfo> GetFilesInDirectory(const wstring& directory) {
    vector<FileInfo> files;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_directory(entry)) {
                // Recursively get files from subdirectory
                wstring subdirectory = entry.path().wstring();
                vector<FileInfo> subdirectoryFiles = GetFilesInDirectory(subdirectory);
                files.insert(files.end(), subdirectoryFiles.begin(), subdirectoryFiles.end());
            }
            else {
                FileInfo fileInfo;
                fileInfo.filename = entry.path().filename().wstring();
                fileInfo.size = fs::file_size(entry);
                fileInfo.creationTime = fs::last_write_time(entry);
                fileInfo.modificationTime = fs::last_write_time(entry);

                // Extract extension from filename
                size_t pos = fileInfo.filename.rfind(L'.');
                if (pos != wstring::npos) {
                    fileInfo.extension = fileInfo.filename.substr(pos + 1);
                }

                files.push_back(fileInfo);
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        wcerr << L"Error opening directory: " << e.what() << endl;
    }

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

        // Convert file_time_type to local time and print
        auto creationTimePoint = chrono::time_point_cast<chrono::system_clock::duration>(file.creationTime - fs::file_time_type::clock::now() + chrono::system_clock::now());
        auto modificationTimePoint = chrono::time_point_cast<chrono::system_clock::duration>(file.modificationTime - fs::file_time_type::clock::now() + chrono::system_clock::now());

        time_t creationTime = chrono::system_clock::to_time_t(creationTimePoint);
        time_t modificationTime = chrono::system_clock::to_time_t(modificationTimePoint);

        tm creationSystemTime = {};
        tm modificationSystemTime = {};

        localtime_s(&creationSystemTime, &creationTime);
        localtime_s(&modificationSystemTime, &modificationTime);

        wcout << L"Creation Time: " << put_time(&creationSystemTime, L"%Y/%m/%d %H:%M:%S") << endl;
        wcout << L"Modification Time: " << put_time(&modificationSystemTime, L"%Y/%m/%d %H:%M:%S") << endl;

        wcout << L"---------------------------" << endl;
    }

    return 0;
}
