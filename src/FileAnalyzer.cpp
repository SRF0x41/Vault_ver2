#include "mylib/FileAnalyzer.h"
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <zip.h>
#include <zipconf.h>

// Initialize the hashmap
std::unordered_map<std::string, size_t> FileAnalyzer::unique_file_words;

// =====================
// Stop Words and Punctuation
// =====================
const std::unordered_set<std::string_view> FileAnalyzer::stopWords = {
    "i",          "me",        "my",      "myself",  "we",         "our",
    "ours",       "ourselves", "you",     "your",    "yours",      "yourself",
    "yourselves", "he",        "him",     "his",     "himself",    "she",
    "her",        "hers",      "herself", "it",      "its",        "itself",
    "they",       "them",      "their",   "theirs",  "themselves", "what",
    "which",      "who",       "whom",    "this",    "that",       "these",
    "those",      "am",        "is",      "are",     "was",        "were",
    "be",         "been",      "being",   "have",    "has",        "had",
    "having",     "do",        "does",    "did",     "doing",      "a",
    "an",         "the",       "and",     "but",     "if",         "or",
    "because",    "as",        "until",   "while",   "of",         "at",
    "by",         "for",       "with",    "about",   "against",    "between",
    "into",       "through",   "during",  "before",  "after",      "above",
    "below",      "to",        "from",    "up",      "down",       "in",
    "out",        "on",        "off",     "over",    "under",      "again",
    "further",    "then",      "once",    "here",    "there",      "when",
    "where",      "why",       "how",     "all",     "any",        "both",
    "each",       "few",       "more",    "most",    "other",      "some",
    "such",       "no",        "nor",     "not",     "only",       "own",
    "same",       "so",        "than",    "too",     "very",       "s",
    "t",          "can",       "will",    "just",    "don't",      "should",
    "now",        "d",         "ll",      "m",       "o",          "re",
    "ve",         "y",         "ain't",   "aren't",  "couldn't",   "didn't",
    "doesn't",    "hadn't",    "hasn't",  "haven't", "isn't",      "ma",
    "mightn't",   "mustn't",   "needn't", "shan't",  "shouldn't",  "wasn't",
    "weren't",    "won't",     "wouldn't"};

const std::unordered_set<char> FileAnalyzer::punctuationSet = {
    '.', ',',  ';', '!', '?', '(', ')',  '[', ']', '{', '}',
    '"', '\'', ':', '-', '_', '/', '\\', '@', '#', '$', '%',
    '^', '&',  '*', '+', '=', '<', '>',  '|', '~', '`'};

// =====================
// Constructor
// =====================
FileAnalyzer::FileAnalyzer() {}

// =====================
// File Utilities
// =====================
std::size_t FileAnalyzer::getSize(const std::string &path) {
  return std::filesystem::file_size(path);
}

std::string FileAnalyzer::getName(const std::string &path) {
  return std::filesystem::path(path).filename().string();
}

std::string FileAnalyzer::getExt(const std::string &path) {
  return std::filesystem::path(path).extension().string();
}

bool FileAnalyzer::isMACOS_Metadata(const std::string &path) {
  if (FileAnalyzer::getName(path).substr(0, 2) == "._") {
    return true;
  }
  return false;
}

std::string FileAnalyzer::getPermissions(const std::string &path) {
  std::filesystem::perms perms = std::filesystem::status(path).permissions();
  std::string perms_str;

  auto show = [&](char op, std::filesystem::perms perm) {
    perms_str += (std::filesystem::perms::none == (perm & perms) ? '-' : op);
  };
  show('r', std::filesystem::perms::owner_read);
  show('w', std::filesystem::perms::owner_write);
  show('x', std::filesystem::perms::owner_exec);
  show('r', std::filesystem::perms::group_read);
  show('w', std::filesystem::perms::group_write);
  show('x', std::filesystem::perms::group_exec);
  show('r', std::filesystem::perms::others_read);
  show('w', std::filesystem::perms::others_write);
  show('x', std::filesystem::perms::others_exec);

  return perms_str;
}

// Ai promptware
int FileAnalyzer::getPermissions_int(const std::string &path) {
  namespace fs = std::filesystem;

  fs::perms p = fs::status(path).permissions();
  int mode = 0;

  // Owner
  if ((p & fs::perms::owner_read) != fs::perms::none)
    mode |= 0400;
  if ((p & fs::perms::owner_write) != fs::perms::none)
    mode |= 0200;
  if ((p & fs::perms::owner_exec) != fs::perms::none)
    mode |= 0100;

  // Group
  if ((p & fs::perms::group_read) != fs::perms::none)
    mode |= 0040;
  if ((p & fs::perms::group_write) != fs::perms::none)
    mode |= 0020;
  if ((p & fs::perms::group_exec) != fs::perms::none)
    mode |= 0010;

  // Others
  if ((p & fs::perms::others_read) != fs::perms::none)
    mode |= 0004;
  if ((p & fs::perms::others_write) != fs::perms::none)
    mode |= 0002;
  if ((p & fs::perms::others_exec) != fs::perms::none)
    mode |= 0001;

  return mode;
}

long long FileAnalyzer::getLastModifiedUnixTime(const std::string &path) {
  std::filesystem::path p = path;

  // Get last write time
  auto ftime = std::filesystem::last_write_time(p);

  // Convert to system_clock::time_point
  auto sys_time = std::chrono::file_clock::to_sys(ftime);

  // Convert to Unix timestamp (seconds since epoch)
  long long unix_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                 sys_time.time_since_epoch())
                                 .count();

  return unix_timestamp;
}

std::string FileAnalyzer::getLastModifiedISO(const std::string &path) {
  // get path then file time
  std::filesystem::path p = path;
  std::filesystem::file_time_type ftime = std::filesystem::last_write_time(p);

  // Convert to unix time
  auto system_time = std::chrono::file_clock::to_sys(ftime);

  std::string iso_time = std::format("{:%Y-%m-%dT%H:%M:%S%z}", system_time);

  return iso_time;
}

// std::string FileAnalyzer

bool FileAnalyzer::isCompressed(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return false;

  unsigned char header[4];
  file.read(reinterpret_cast<char *>(header), 4);

  return header[0] == 0x50 && header[1] == 0x4B && header[2] == 0x03 &&
         header[3] == 0x04;
}

bool FileAnalyzer::isMicrosoftCompressedXML(const std::string &path) {
  static const std::unordered_set<std::string>
      MicrosoftCompressedXML_format_extension = {".docx", ".xlsx", ".pptx",
                                                 ".epub"};
  return MicrosoftCompressedXML_format_extension.find(getExt(path)) !=
         MicrosoftCompressedXML_format_extension.end();
}

bool FileAnalyzer::isDOCX(const std::string &path) {
  if (!isMicrosoftCompressedXML(path)) {
    return false;
  }
  if (getExt(path) != ".docx") {
    return false;
  }
  return true;
}

bool FileAnalyzer::isPDF(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return false;

  char header[4];
  file.read(header, 4);
  return std::string(header, 4) == "%PDF";
}

bool FileAnalyzer::isRawText(const std::string &path) {
  static const std::unordered_set<std::string> textExts = {
      // Plain text, logs, markdown, config
      ".txt", ".log", ".md", ".csv", ".json", ".xml", ".yaml", ".yml", ".ini",
      // Source code and scripts
      ".c", ".cpp", ".h", ".py", ".java", ".js", ".html", ".css"};
  return textExts.find(getExt(path)) != textExts.end();
}

// =====================
// Stop Word Utilities
// =====================
bool FileAnalyzer::isStopWord(const std::string &word) {
  if (stopWords.find(word) != stopWords.end()) {
    return true;
  }
  return false;
}

// =====================
// Keyword Extraction (Raw Text)
// =====================

int FileAnalyzer::removeTrailingPunctuation(std::string &word) {
  if (word.size() == 0) {
    return 1;
  }
  if (punctuationSet.find(word[0]) == punctuationSet.end()) {
    word.erase(0, 1);
  }
  if (punctuationSet.find(word[word.size() - 1]) == punctuationSet.end()) {
    word.erase(word.size() - 1, 1);
  }
  if (word.size() == 0) {
    return 1;
  }
  return 0;
}

/* std::unordered_map<int, std::string> fruits = {
        {1, "Apple"},
        {2, "Banana"},
        {3, "Orange"}
    };

    // Insert new elements using operator[]
    fruits[4] = "Grape";
    fruits[5] = "Kiwi";

    // Insert using the insert method
    fruits.insert({6, "Peach"});

    std::cout << "Fruits map contains:" << std::endl;
    for (const auto& pair : fruits) {
        // Elements are not stored in any specific order
        std::cout << pair.first << ": " << pair.second << std::endl;
    }

    bool FileAnalyzer::isStopWord(const std::string &word) {
  if (stopWords.find(word) != stopWords.end()) {
    return true;
  }
  return false;
}
*/

void FileAnalyzer::addKeywords(std::string &word) {
  // If the word is empty, return
  if (word.size() == 0)
    return;

  // If word is a stop word return
  if (isStopWord(word)) {
    return;
  }

  // Remove trailing punctiation
  if (!FileAnalyzer::removeTrailingPunctuation(word)) {
    return;
  };

  FileAnalyzer::unique_file_words[word]++;
}

void FileAnalyzer::clearUniqueFileWords() { unique_file_words.clear(); }

// =====================
// Extract Raw Text Word by Word
// =====================
int FileAnalyzer::extractRaw_text(const std::string &path) {
  std::ifstream input_file(path);
  if (!input_file) {
    std::cerr << "Error opening raw text file." << '\n';
    return 1;
  }

  std::string word;
  while (input_file >> word) {
    if (!isStopWord(word)) {

      std::cout << word << " ";
    }
  }

  input_file.close();
  return 0;
}

// =====================
// Extract DOCX Text
// =====================
int FileAnalyzer::extractDOCX_text(const std::string &path) {
  int err = 0; // Store error codes

  // open the zip in read only
  zip *opened_zip = zip_open(path.c_str(), ZIP_RDONLY, &err);
  if (!opened_zip) {
    std::cerr << "Failed to open zip" << '\n';
    return 1;
  }

  zip_int64_t document_xml_index =
      zip_name_locate(opened_zip, "word/document.xml", 0);
  if (document_xml_index < 0) {
    return 1;
  }

  zip_stat_t st;
  zip_stat_init(&st);
  zip_stat_index(opened_zip, document_xml_index, 0, &st);

  std::cout << st.name << '\n';

  // open file inside zip
  zip_file *zf = zip_fopen_index(opened_zip, document_xml_index, 0);
  if (!zf)
    return 1;

  // create a buffer for storing data chunks, lzip can do 8192 bytes
  char buffer[8192];

  // number of bytes read per iteration
  zip_int64_t n;

  // write to disk 'out'
  while ((n = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
    // char word[n+1];
    bool inside_tag = false;
    // int word_index = 0;
    size_t word_size = 0;

    for (zip_int64_t i = 0; i < n; ++i) {
      char c = buffer[i];
      if (c == '<') {
        inside_tag = true;
      } else if (c == '>') {
        inside_tag = false;
      } else if (!inside_tag) {
        // This works too
        // if(c == ' '){
        //   word[word_index] = '\0';
        //   std::cout << word << '\n';
        //   word_index = 0;
        // } else {
        //   word[word_index] = c;
        //   word_index++;
        // }
        // std::cout << c;

        if (c == ' ') {
          char word[word_size + 1];
          zip_int64_t count = 0;
          for (zip_int64_t x = i - word_size; x < i; x++) {
            word[count++] = buffer[x];
          }
          count = 0;
          word[word_size] = '\0';
          word_size = 0;

          if (!FileAnalyzer::isStopWord(word)) {
            std::cout << word << " ";
          }
        } else {
          word_size++;
        }
      }
    }
  }

  // close zf
  zip_fclose(zf);

  // close za
  zip_close(opened_zip);

  return 0;
}
