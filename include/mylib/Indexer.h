#pragma once
#include "SQlite_Client.h"
#include <iostream>
#include <string>

class Indexer {
private:
  std::string root_path;

public:
  Indexer();
  ~Indexer();

  void index(const std::string &root_path, Client *client);
};