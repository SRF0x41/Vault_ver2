#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <zip.h>
// g++ test_microsoft.cpp -lzip -o microslop
int unzip(const std::string &zip_path, const std::string &out_dir);
int extractDOCX_text(const std::string &path);
int main() {
  std::string path =
      "/home/user1/Desktop/MacBackup3_25_2025/PhysicsAstronomy/orion.docx";

  unzip(path, "output_docx");
}
int unzip(const std::string &zip_path, const std::string &out_dir) {
  int err = 0; // var that holds error codes
  // opens the zip in read only returns pointer to a zip or a nullptr
  zip *za = zip_open(zip_path.c_str(), ZIP_RDONLY, &err);
  if (!za) {
    std::cerr << "Failed to open zip\n";
    return 1;
  }

  // creates output directories
  std::filesystem::create_directories(out_dir);

  // get the number of zip entries
  zip_int64_t count = zip_get_num_entries(za, 0);
  for (zip_uint64_t i = 0; i < count; ++i) {
    // struct holding metadata, (name size compression etc)
    zip_stat_t st;
    // initialisez the struct safely
    zip_stat_init(&st);
    // fills st with metadata for entry i 
    zip_stat_index(za, i, 0, &st);

    std::string name = st.name;

    std::filesystem::path out_path = std::filesystem::path(out_dir) / name;

    // create the direcroty and continue if the zip path ends
    if (name.back() == '/') {
      std::filesystem::create_directories(out_path);
      continue;
    }

    // create the rest of the path
    std::filesystem::create_directories(out_path.parent_path());

    // open the file inside zip
    zip_file *zf = zip_fopen_index(za, i, 0);

    // if it cant be opened contine
    if (!zf)
      continue;

    // open file in binary mode
    std::ofstream out(out_path, std::ios::binary);

    // create a buffer for storing data chunks, lzip can do 8192 bytes
    char buffer[8192];

    // number of bytes read per iteration
    zip_int64_t n;

    // write to disk 'out'
    while ((n = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
      out.write(buffer, n);
    }

    // close zf
    zip_fclose(zf);
  }

  // close za
  zip_close(za);
  return 0;
}
