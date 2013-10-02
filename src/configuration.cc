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
: idx_(0)
{
}

Configuration::~Configuration()
{
}

void Configuration::Load(const string& filename)
{
  ReadFile(filename);
  reader_ = thread(Reader, this, filename);
}

int Configuration::ReadInt(const string& name)
{
  int value;
  stringstream ss(variables_[idx_].at(name));
  ss >> value;

  return value;
}

float Configuration::ReadFloat(const string& name)
{
  float value;
  stringstream ss(variables_[idx_].at(name));
  ss >> value;

  return value;
}

double Configuration::ReadDouble(const string& name)
{
  double value;
  stringstream ss(variables_[idx_].at(name));
  ss >> value;

  return value;
}

bool Configuration::ReadBool(const string& name)
{
  if (variables_[idx_].at(name) == "true") {
    return true;
  }

  return false;
}


string Configuration::ReadString(const string& name)
{
  return variables_[idx_].at(name);
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
  int newIdx = (idx_ + 1) % 2;
  ifstream file(filename);
  string name, value;
  auto& variables = variables_[newIdx];

  variables.clear();
  while (file >> name >> value) {
    variables[name] = value;
  }

  idx_ = newIdx;
}
