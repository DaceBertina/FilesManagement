#include <iostream>
#include <filesystem>
#include <queue>
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

void ProcessFilesInDirectoryQueue(const fs::path& directory, vector<FileInfo>& files) {
    // Measure execution time
    auto start = chrono::high_resolution_clock::now();

    queue<fs::path> directoryQueue;
    directoryQueue.push(directory);

    while (!directoryQueue.empty()) {
        fs::path currentDir = directoryQueue.front();
        directoryQueue.pop();

        for (const auto& entry : fs::directory_iterator(currentDir)) {
            if (entry.is_directory()) {
                // Recursively get files from subdirectory
                wstring subdirectory = entry.path().wstring();
                directoryQueue.push(subdirectory);
            }
            else if (entry.is_regular_file()) {
                // Process the file
                FileInfo fileInfo;
                fileInfo.filename = entry.path().filename().wstring();
                fileInfo.size = fs::file_size(entry);
                fileInfo.creationTime = fs::last_write_time(entry);
                fileInfo.modificationTime = fs::last_write_time(entry);

                // Extract extension from filename
                size_t pos = fileInfo.filename.rfind(L'.');
                if (pos != wstring::npos && pos < fileInfo.filename.size()) {
                    fileInfo.extension = fileInfo.filename.substr(pos + 1);
                }

                files.push_back(fileInfo);
            }
        }
    }

    // Stop measuring time
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    // Display the execution time
    wcout << L"Execution Time: " << duration.count() << L" milliseconds" << endl;
}

int main() {
    wstring directory = L"C:\\Users\\User\\Desktop\\VIA_5_sem";
    vector<FileInfo> files;

    // Process files and collect information
    ProcessFilesInDirectoryQueue(directory, files);

    // Display the retrieved file information
    wcout << L"\nQueue Method:\n";
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
