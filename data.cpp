#include <assert.h>
#include <stdio.h>
#include<stdlib.h> 
#include "data.h"
#include "string.h"
#include "map"
#include <string>
#include <iostream>
#include <fstream>
#include "sstream"


c_data::c_data() {
}

c_data::~c_data() {
  for (size_t i = 0; i < m_vec_data.size(); i ++) {
    int* ids = m_vec_data[i];
    if (ids != NULL) delete [] ids;
  }
  m_vec_data.clear();
  m_vec_len.clear();
  m_vec_ids.clear();
  m_map_ids.clear();
}

void c_data::read_data(const char * data_filename, int OFFSET) {

  int length = 0, n = 0, id = 0, total = 0;

  FILE * fileptr;
  fileptr = fopen(data_filename, "r");

  while ((fscanf(fileptr, "%10d", &length) != EOF)) {
    int * ids = NULL;
    if (length > 0) {
      ids = new int[length];
      for (n = 0; n < length; n++) {
        fscanf(fileptr, "%10d", &id);
        ids[n] = id - OFFSET;
      }
    }
    m_vec_data.push_back(ids);
    m_vec_len.push_back(length);
    total += length;
  }
  fclose(fileptr);
  printf("read %d vectors with %d entries ...\n", (int)m_vec_len.size(), total);
}

void c_data::read_ids(const char * data_filename) {

  int index=0;
  std::string id;
  std::string line;

  std::fstream in;
  in.open(data_filename);

  while (std::getline(in, line)) {
    std::stringstream ss(line);
    ss >> index >> id;
    m_vec_ids.push_back(id);
    m_r_ids[id] = index;
  }
  in.close();
  printf("read %d ids ...\n", (int)m_vec_ids.size());
}

void c_data::read_selectIds(const char * data_filename) {
  std::fstream in;
  std::string line;
  std::string iid;
  std::string uid;
  in.open(data_filename);
  while (std::getline(in, line))
  {
    std::stringstream ss(line);
    ss >> uid >> iid;
    m_map_ids[uid] = iid;
  }
  in.close();
  printf("read %d selected ids ...\n", (int)m_map_ids.size());  
}

