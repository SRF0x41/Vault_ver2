#include <iostream>
#include "mylib/sql_client.h"


Client::Client() {
  conn = mysql_init(nullptr);
  if (!mysql_real_connect(conn, "localhost", user.c_str(), "", db_name.c_str(),
                          3306, nullptr, 0)) {
    std::cerr << "Connection failed: " << mysql_error(conn) << "\n";
    return;
  }
  std::cout << "Connected to MariaDB/MySQL\n";
}


void Client::executeQuery(const std::string& query) {
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn) << "\n";
        return;
    }
    MYSQL_RES* result = mysql_store_result(conn);
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            for (unsigned int i = 0; i < mysql_num_fields(result); i++) {
                std::cout << (row[i] ? row[i] : "NULL") << " ";
            }
            std::cout << "\n";
        }
        mysql_free_result(result);
    } else {
        std::cout << "Query executed successfully (no results)\n";
    }
}


Client::~Client() {
  if (conn)
    mysql_close(conn);
}


/*Database structure
sqlite> .schema
CREATE TABLE file_index_metadata (
    total_indexed_files BIGINT DEFAULT 0,

    -- Readable text files
    n_txt BIGINT DEFAULT 0,
    n_log BIGINT DEFAULT 0,
    n_md BIGINT DEFAULT 0,
    n_csv BIGINT DEFAULT 0,
    n_xml BIGINT DEFAULT 0,
    n_json BIGINT DEFAULT 0,
    n_yaml BIGINT DEFAULT 0,
    n_yml BIGINT DEFAULT 0,
    n_ini BIGINT DEFAULT 0,
    
    -- Source code files
    n_c BIGINT DEFAULT 0,
    n_cpp BIGINT DEFAULT 0,
    n_h BIGINT DEFAULT 0,
    n_py BIGINT DEFAULT 0,
    n_java BIGINT DEFAULT 0,
    n_js BIGINT DEFAULT 0,
    n_html BIGINT DEFAULT 0,
    n_css BIGINT DEFAULT 0,
    
    -- Files that need special parser
    n_docx BIGINT DEFAULT 0,
    n_xlsx BIGINT DEFAULT 0,
    n_pptx BIGINT DEFAULT 0,
    n_pdf BIGINT DEFAULT 0,
    n_epub BIGINT DEFAULT 0,
    n_zip BIGINT DEFAULT 0,
    n_rar BIGINT DEFAULT 0,
    n_7z BIGINT DEFAULT 0,
    n_tar BIGINT DEFAULT 0,
    n_gz BIGINT DEFAULT 0,
    n_sqlite BIGINT DEFAULT 0,
    n_db BIGINT DEFAULT 0,
    n_png BIGINT DEFAULT 0,
    n_jpg BIGINT DEFAULT 0,
    n_jpeg BIGINT DEFAULT 0,
    n_bmp BIGINT DEFAULT 0,
    n_gif BIGINT DEFAULT 0,
    n_svg BIGINT DEFAULT 0,
    n_mp3 BIGINT DEFAULT 0,
    n_wav BIGINT DEFAULT 0,
    n_flac BIGINT DEFAULT 0,
    n_mp4 BIGINT DEFAULT 0,
    n_avi BIGINT DEFAULT 0,
    n_mkv BIGINT DEFAULT 0,

    -- Files that should never be treated as text
    n_exe BIGINT DEFAULT 0,
    n_dll BIGINT DEFAULT 0,
    n_so BIGINT DEFAULT 0,
    n_bin BIGINT DEFAULT 0,
    n_sys BIGINT DEFAULT 0,
    n_img BIGINT DEFAULT 0,
    n_iso BIGINT DEFAULT 0
);
CREATE TABLE file_index(
file_id INTEGER PRIMARY KEY,
file_name TEXT,
file_extension TEXT,
file_path TEXT,
file_size_bytes BIGINT,
file_keywords TEXT,
file_last_modified INTEGER,
file_first_created INTEGER,
file_permissions INTEGER);
*/