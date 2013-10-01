#include "configuration.h"

#include <cstdio>
#include <fstream>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

Configuration& Configuration::Instance()
{
  static Configuration instance;
  return instance;
}

Configuration::Configuration()
: reader_(Reader, this, "configuration.xml")
{
}

Configuration::~Configuration()
{
}

int Configuration::ReadInt(const string name)
{
  int value;
  stringstream ss(variables_.at(name));
  ss >> value;

  return value;
}

float Configuration::ReadFloat(const string name)
{
  lock_guard<mutex> lock(mutex_);
  float value;
  stringstream ss(variables_.at(name));
  ss >> value;

  return value;
}

bool Configuration::ReadBool(const string name)
{
  lock_guard<mutex> lock(mutex_);
  if (variables_.at(name) == "true") {
    return true;
  }

  return false;
}


string Configuration::ReadString(const string name)
{
  lock_guard<mutex> lock(mutex_);
  return variables_.at(name);
}

void Configuration::Reader(Configuration* instance, const string& filename)
{
  struct stat st;
  time_t timestamp = 0;

  while (true) {
    if (stat(filename.c_str(), &st)) {
      cout << "Configuration file " << filename << " not found!" << endl;
    } else {
      if (st.st_ctime > timestamp) {
        timestamp = st.st_ctime;

        // Update values from configuration file.
        instance->ReadFile(filename);
      }
    }

    usleep(500000);  // Wait .5 seconds.
  }
}

void Configuration::ReadFile(const string& filename)
{
  ifstream file(filename);
  string name, value;

  lock_guard<mutex> lock(mutex_);
  variables_.clear();
  while (file >> name >> value) {
    variables_[name] = value;
  }
}
