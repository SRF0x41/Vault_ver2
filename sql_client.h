#pragma once
#include <iostream>
#include <mysql/mysql.h>
#include <string>

class Client {
private:
  MYSQL *conn;
  std::string user = "vault_test_user";
  std::string db_name = "FileIndex";

public:
  Client();  // Constructor
  ~Client(); // Destructor

  void executeQuery(const std::string &query);
};
