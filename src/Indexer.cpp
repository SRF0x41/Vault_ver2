#include "mylib/Indexer.h"
#include "mylib/FileAnalyzer.h"
#include "mylib/SQlite_Client.h"
#include <cstddef>
#include <filesystem>
#include <iostream>
Indexer::Indexer() {};

void Indexer::index(const std::string &root_path, Client *client) {
  try {
    // Iterate recursively through all files and directories
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(root_path)) {
      if (std::filesystem::is_regular_file(entry.path()) &&
          FileAnalyzer::isMicrosoftCompressedXML(entry.path()) &&
          !FileAnalyzer::isMACOS_Metadata(entry.path())) { // only print files

        std::cout << "\n==================== START OF NEW INDEX "
                     "====================\n";

        // ====================
        // Get basic metadata on the file
        // ====================

        // Test get name of file
        std::cout << "FILE NAME: " << FileAnalyzer::getName(entry.path())
                  << "\n";

        // Test get extension of file
        std::cout << "FILE EXTENSION: " << FileAnalyzer::getExt(entry.path())
                  << "\n";

        std::cout << "FILE PATH: " << entry.path() << "\n";

        // Test get size of file
        size_t file_size_bytes = FileAnalyzer::getSize(entry.path());
        std::cout << "FILE SIZE: " << file_size_bytes << "\n";

        std::cout << "LAST WRITE TIME ISO: "
                  << FileAnalyzer::getLastModifiedISO(entry.path()) << '\n';

        std::cout << "LAST WRITE TIME UNIX: "
                  << FileAnalyzer::getLastModifiedUnixTime(entry.path())
                  << '\n';

        std::cout << "FILE PERMISSIONS: "
                  << FileAnalyzer::getPermissions(entry.path()) << '\n';

        // Basic database insertion
        /*INSERT INTO files (path, size, mode)
VALUES ('/home/user/test.txt', 1024, 420);*/

        auto escape = [](const std::string &s) {
          std::string out;
          out.reserve(s.size());
          for (char c : s) {
            if (c == '\'')
              out += "''"; // SQL escape single quote
            else
              out += c;
          }
          return out;
        };

        // Current unix time
        // Get the current time point
        long long duration_since_epoch =
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();

        std::string q = std::format(
            "INSERT OR IGNORE INTO file_index "
            "(file_name,file_extension,file_path,file_size_bytes,"
            "file_last_modified,file_permissions,last_indexed) "
            "VALUES ('{}','{}','{}',{},{},{},{})",
            escape(FileAnalyzer::getName(entry.path())),
            escape(FileAnalyzer::getExt(entry.path())),
            escape(entry.path().string()), FileAnalyzer::getSize(entry.path()),
            FileAnalyzer::getLastModifiedUnixTime(entry.path()),
            FileAnalyzer::getPermissions_int(entry.path()),
            duration_since_epoch);

        client->sendQuery(q);

        // populate metadata
        // client->incrementExtensionCount(FileAnalyzer::getExt(entry.path()));

        // ====================
        // Parsers
        // ====================

        if (FileAnalyzer::isDOCX(entry.path())) {
          FileAnalyzer::extractDOCX_text(entry.path());
        }

        if (FileAnalyzer::isRawText(entry.path())) {
          FileAnalyzer::extractRaw_text(entry.path());
        }

        if (FileAnalyzer::isPDF(entry.path())) {
        }

        // Test get keywords
        // std::cout << "KEYWORDS: " <<
        // FileAnalyzer::getKeywords(entry.path()) << " \n";
        // FileAnalyzer::getKeywords(entry.path());
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Filesystem error: " << e.what() << "\n";
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
  }
}

Indexer::~Indexer() {};