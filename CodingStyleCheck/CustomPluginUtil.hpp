//
//  CustomPluginUtil.hpp
//  Testclang
//
//  Created by Yaso on 29/05/2018.
//  Copyright © 2018 Yaso. All rights reserved.
//

#ifndef CustomPluginUtil_hpp
#define CustomPluginUtil_hpp

#include <iostream>

using namespace std;

static void nslog(const string tip, const string param)
{
  std::cout << "~~~~~~~~~" << tip << "~~~~~~~~~" << param << "\n";
}

static inline string removePtrString(const string typeString)
{
  size_t lastindex = typeString.find_last_of("*");
  return typeString.substr(0, lastindex);
}

static inline void remove_char_from_string(string &str,char ch)
{
  str.erase(remove(str.begin(), str.end(), ch), str.end());
}

static inline void remove_blank(string &str)
{
  remove_char_from_string(str,' ');
  remove_char_from_string(str,'\t');
  remove_char_from_string(str,'\r');
  remove_char_from_string(str,'\n');
}

#endif /* CustomPluginUtil_hpp */
