#include "mylib/FileAnalyzer.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <zip.h>
#include <zipconf.h>

using namespace std;

// =====================
// Static Members
// =====================

// Initialize the hashmap
unordered_map<string, size_t> FileAnalyzer::unique_file_words;

// Stop Words and Punctuation
const unordered_set<string_view> FileAnalyzer::stopWords = {
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

const unordered_set<char> FileAnalyzer::punctuationSet = {
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
size_t FileAnalyzer::getSize(const string &path) {
  return filesystem::file_size(path);
}

string FileAnalyzer::getName(const string &path) {
  return filesystem::path(path).filename().string();
}

string FileAnalyzer::getExt(const string &path) {
  return filesystem::path(path).extension().string();
}

bool FileAnalyzer::isMACOS_Metadata(const string &path) {
  return getName(path).substr(0, 2) == "._";
}

string FileAnalyzer::getPermissions(const string &path) {
  filesystem::perms perms = filesystem::status(path).permissions();
  string perms_str;

  auto show = [&](char op, filesystem::perms perm) {
    perms_str += (filesystem::perms::none == (perm & perms) ? '-' : op);
  };
  show('r', filesystem::perms::owner_read);
  show('w', filesystem::perms::owner_write);
  show('x', filesystem::perms::owner_exec);
  show('r', filesystem::perms::group_read);
  show('w', filesystem::perms::group_write);
  show('x', filesystem::perms::group_exec);
  show('r', filesystem::perms::others_read);
  show('w', filesystem::perms::others_write);
  show('x', filesystem::perms::others_exec);

  return perms_str;
}

int FileAnalyzer::getPermissions_int(const string &path) {
  filesystem::perms p = filesystem::status(path).permissions();
  int mode = 0;

  if ((p & filesystem::perms::owner_read) != filesystem::perms::none)
    mode |= 0400;
  if ((p & filesystem::perms::owner_write) != filesystem::perms::none)
    mode |= 0200;
  if ((p & filesystem::perms::owner_exec) != filesystem::perms::none)
    mode |= 0100;

  if ((p & filesystem::perms::group_read) != filesystem::perms::none)
    mode |= 0040;
  if ((p & filesystem::perms::group_write) != filesystem::perms::none)
    mode |= 0020;
  if ((p & filesystem::perms::group_exec) != filesystem::perms::none)
    mode |= 0010;

  if ((p & filesystem::perms::others_read) != filesystem::perms::none)
    mode |= 0004;
  if ((p & filesystem::perms::others_write) != filesystem::perms::none)
    mode |= 0002;
  if ((p & filesystem::perms::others_exec) != filesystem::perms::none)
    mode |= 0001;

  return mode;
}

long long FileAnalyzer::getLastModifiedUnixTime(const string &path) {
  filesystem::path p = path;
  auto ftime = filesystem::last_write_time(p);
  auto sys_time = chrono::file_clock::to_sys(ftime);
  return chrono::duration_cast<chrono::seconds>(sys_time.time_since_epoch())
      .count();
}

string FileAnalyzer::getLastModifiedISO(const string &path) {
  filesystem::path p = path;
  auto ftime = filesystem::last_write_time(p);
  auto system_time = chrono::file_clock::to_sys(ftime);
  return format("{:%Y-%m-%dT%H:%M:%S%z}", system_time);
}

bool FileAnalyzer::isCompressed(const string &path) {
  ifstream file(path, ios::binary);
  if (!file)
    return false;

  unsigned char header[4];
  file.read(reinterpret_cast<char *>(header), 4);
  return header[0] == 0x50 && header[1] == 0x4B && header[2] == 0x03 &&
         header[3] == 0x04;
}

bool FileAnalyzer::isMicrosoftCompressedXML(const string &path) {
  static const unordered_set<string> MicrosoftCompressedXML_format_extension = {
      ".docx", ".xlsx", ".pptx", ".epub"};
  return MicrosoftCompressedXML_format_extension.find(getExt(path)) !=
         MicrosoftCompressedXML_format_extension.end();
}

bool FileAnalyzer::isDOCX(const string &path) {
  return isMicrosoftCompressedXML(path) && getExt(path) == ".docx";
}

bool FileAnalyzer::isPDF(const string &path) {
  ifstream file(path, ios::binary);
  if (!file)
    return false;

  char header[4];
  file.read(header, 4);
  return string(header, 4) == "%PDF";
}

bool FileAnalyzer::isRawText(const string &path) {
  static const unordered_set<string> textExts = {
      ".txt", ".log", ".md", ".csv", ".json", ".xml", ".yaml", ".yml", ".ini",
      ".c",   ".cpp", ".h",  ".py",  ".java", ".js",  ".html", ".css"};
  return textExts.find(getExt(path)) != textExts.end();
}

// =====================
// Stop Word Utilities
// =====================
bool FileAnalyzer::isStopWord(const string &word) {
  return stopWords.find(word) != stopWords.end();
}

// =====================
// Keyword Extraction
// =====================
int FileAnalyzer::removeTrailingPunctuation(string &word) {
  if (word.size() == 0)
    return 1;
  if (punctuationSet.find(word[0]) != punctuationSet.end())
    word.erase(0, 1);
  if (punctuationSet.find(word[word.size() - 1]) != punctuationSet.end())
    word.erase(word.size() - 1, 1);
  if (word.size() == 0)
    return 1;
  return 0;
}

void FileAnalyzer::addKeyword(string &word) {
  if (unique_file_words.size() > TOTAL_KEYWORDS)
    return;
  if (word.size() == 0)
    return;
  if (isStopWord(word))
    return;
  if (removeTrailingPunctuation(word))
    return;
  unique_file_words[word]++;
}

void FileAnalyzer::addKeyword(char *word) {
  if (unique_file_words.size() > TOTAL_KEYWORDS)
    return;
  string s_word(word);
  if (s_word.size() == 0)
    return;
  if (isStopWord(s_word))
    return;
  if (removeTrailingPunctuation(s_word))
    return;
  unique_file_words[s_word]++;
}

void FileAnalyzer::clearUniqueFileWords() { unique_file_words.clear(); }

void FileAnalyzer::printKeywords() {
  cout << "Number of keywords: " << unique_file_words.size() << "\n";
  for (const auto &[word, count] : unique_file_words)
    cout << word << ":" << count << " | ";
  cout << "\n";
}

bool keyword_comparator(const pair<string, size_t> &a,
                        const pair<string, size_t> &b) {
  return a.second > b.second; // descending
}

string *FileAnalyzer::sortKeywordsDescending() {
  vector<pair<string, size_t>> sorted_vector;
  string *out_str = new string();
  for (auto &keyword : unique_file_words)
    sorted_vector.push_back(keyword);

  sort(sorted_vector.begin(), sorted_vector.end(), keyword_comparator);

  for (auto &a : sorted_vector)
    *out_str += a.first + " ";
  return out_str;
}

// =====================
// Extract Raw Text Word by Word
// =====================
int FileAnalyzer::extractRaw_text(const string &path) {
  ifstream input_file(path);
  if (!input_file) {
    cerr << "Error opening raw text file." << '\n';
    return 1;
  }

  string word;
  while (input_file >> word)
    addKeyword(word);

  printKeywords();
  string *sorted_keywords = sortKeywordsDescending();
  cout << *sorted_keywords << '\n';
  delete (sorted_keywords);
  clearUniqueFileWords();
  input_file.close();
  return 0;
}

// =====================
// Extract DOCX Text
// =====================
int FileAnalyzer::extractDOCX_text(const string &path) {
  int err = 0;
  zip *opened_zip = zip_open(path.c_str(), ZIP_RDONLY, &err);
  if (!opened_zip) {
    cerr << "Failed to open zip" << '\n';
    return 1;
  }

  zip_int64_t document_xml_index =
      zip_name_locate(opened_zip, "word/document.xml", 0);
  if (document_xml_index < 0)
    return 1;

  zip_stat_t st;
  zip_stat_init(&st);
  zip_stat_index(opened_zip, document_xml_index, 0, &st);
  cout << st.name << '\n';

  zip_file *zf = zip_fopen_index(opened_zip, document_xml_index, 0);
  if (!zf)
    return 1;

  char buffer[8192];
  zip_int64_t n;
  bool inside_tag = false;
  size_t word_size = 0;

  while ((n = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
    for (zip_int64_t i = 0; i < n; ++i) {
      char c = buffer[i];
      if (c == '<') {
        inside_tag = true;
      } else if (c == '>') {
        inside_tag = false;
      } else if (!inside_tag) {
        if (c == ' ') {
          char word[word_size + 1];
          zip_int64_t count = 0;
          for (zip_int64_t x = i - word_size; x < i; x++)
            word[count++] = buffer[x];
          count = 0;
          word[word_size] = '\0';
          word_size = 0;

          addKeyword(word);
        } else {
          word_size++;
        }
      }
    }
  }

  printKeywords();
  string *sorted_keywords = sortKeywordsDescending();
  cout << *sorted_keywords << '\n';
  delete (sorted_keywords);

  clearUniqueFileWords();

  zip_fclose(zf);
  zip_close(opened_zip);

  return 0;
}
