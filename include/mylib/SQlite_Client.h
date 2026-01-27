#pragma once
#include <cstddef>
#include <sqlite3.h>
#include <string>


class Client {
    public:
    // ====================
    // Constructors Desctructors
    // ====================
    Client();
    ~Client();

    int sendQuery(const std::string &query);
    int closeConnection();

    int getFileIndexHead();
    int getMetadata();

    int deleteFileIndex();
    int dropMetadata();

    int incrementExtensionCount(const std::string &extension);
    int decrementExtensionCount(const std::string &extension);

    int incrementExtensionCount_getcount(const std::string &extension);
    int decrementExtensionCount_getcount(const std::string &extension);

    int checkForExistense(const std::string &path);

    // int updateFileName(const std::string &extension);
    // int updateFileExtension(const std::string &extension);
    // int updateFilePath(const std::string &extension);
    // int updateFileSize(const size_t file_size);
    // int updateFileKeywords(const std::string &extension);
    // int updateFileLastModified(const long long unix_time);
    // int updateFilePermissions(const int);


    private:
    sqlite3* database = nullptr;
};