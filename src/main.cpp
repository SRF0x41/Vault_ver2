#include <iostream>
#include <string>
#include "mylib/SQlite_Client.h"
#include "mylib/Indexer.h"

int main() {

    std::string root_path = "/home/user1/Desktop/MacBackup3_25_2025";
    std::string input_line;

    Client client = Client();
    Indexer indexer;

    while(input_line != "exit"){
        std::getline(std::cin, input_line);
        if(input_line == "index"){
            indexer.index(root_path, &client);
        }

        if(input_line == "test-client"){

        }

        if(input_line == "get-fileindex-head"){

        }

        if(input_line == "get-metadata-head"){

        }
    }

}
