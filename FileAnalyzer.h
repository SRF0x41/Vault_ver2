#pragma once
#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>

class FileAnalyzer {
public:
  // =====================
  // Constructor
  // =====================
  FileAnalyzer();
  ~FileAnalyzer();

  // =====================
  // File Information
  // =====================
  static std::size_t getSize(const std::string &path);
  static std::string getName(const std::string &path);
  static std::string getExt(const std::string &path);
  static std::string getLastModifiedISO(const std::string &path);
  static long long getLastModifiedUnixTime(const std::string &path);
  static std::string getPermissions(const std::string &path);
  static int getPermissions_int(const std::string &path);
  // These are platform specific
  // static std::string getFirstCreatedISO(const std::string &path);
  // static long long getFirstCreatedUnixTime(const std::string &path);

  // =====================
  // File Type Checks
  // =====================
  static bool isPDF(const std::string &path);
  static bool isCompressed(const std::string &path);
  static bool isRawText(const std::string &path);
  static bool isMicrosoftCompressedXML(const std::string &path);
  static bool isDOCX(const std::string &path);

  // =====================
  // Stop Word Utilities
  // =====================
  static bool isStopWord(const std::string &word);

  // =====================
  // Keyword Extraction
  // =====================
  static std::vector<std::string> getKeywords(const std::string &path);

  // =====================
  // Text Extraction
  // =====================
  static int extractDOCX_text(const std::string &path);
  static int extractRaw_text(const std::string &path);

private:
  // =====================
  // Internal Data
  // =====================
  static const std::unordered_set<std::string_view> stopWords;
  static const std::unordered_set<char> punctuationSet;
};

/* Notes:

Can be read as raw text:
.txt, .log, .md, .csv, .xml, .json, .yaml, .yml, .ini
.c, .cpp, .h, .py, .java, .js, .html, .css


Needs a special parser:
.docx, .xlsx, .pptx, .pdf, .epub
.zip, .rar, .7z, .tar, .gz
.sqlite, .db
.png, .jpg, .jpeg, .bmp, .gif, .svg
.mp3, .wav, .flac, .mp4, .avi, .mkv

Never treat as text
.exe, .dll, .so, .bin
.sys, .img, .iso
.jpg, .png, .mp4, .mp3
.rar, .7z, .gz


Database structure



*/