// class for reading the sparse matrix data
// for both user matrix and item matrix
// user matrix:
// number_of_items item1 item2 ...
// item matrix:
// number_of_users user1 user2 ...

#ifndef DATA_H
#define DATA_H

#include <vector>
#include "string.h"
#include <string>
#include "map"

using namespace std;

class c_data {
public:
  c_data();
  ~c_data();
  void read_data(const char * data_filename, int OFFSET=0);
  void read_ids(const char * data_filename);
  void read_selectIds(const char * data_filename);
public:
  vector<int*> m_vec_data;
  vector<int> m_vec_len;
  vector<string> m_vec_ids;
  map<std::string, std::string> m_map_ids;
};

#endif // DATA_H
