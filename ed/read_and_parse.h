#pragma once
#include "string"
#include <list>

#include <iostream>
#include <fstream>
#include <sstream>

enum text_type standard, speech;

struct speech_at {
  int pos;
  text_type type;
};

struct high_text {
  std::string text;
// ---------------------------------------------------------
// cannot access a list element directly, just by traversal.
// rather wanting a vector here.
// ---------------------------------------------------------
  std::list<speech_at> list_of_speech; // https://www.geeksforgeeks.org/list-cpp-stl/?ref=lbp
};

void read_and_parse(const std::string filename, high_text& out)
{
// https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
  std::ifstream t(filename);
  std::stringstream buffer;
  buffer << t.rdbuf();
  out.text = buffer.str();

//  “ - start of speech         226 128 156
//  ” - end of speech
  std::string stmp = buffer.str();
  for (int i = 0; i < stmp.length(); i++)
  {
    char c = stmp[i];
/*    if (c == 'a')
//    if (c == '“') // ASCII to ...?
    {
      out.list_of_speech.push_back({ i, standard });
    }*/
// http://www.unit-conversion.info/texttools/ascii/
    if (stmp[i] == -30)//'0xe2') // ASCII to ...?
    {
      if (stmp[i+1] == -128)//'0x80') // ASCII to ...?
      {
        if (stmp[i+2] == -100) //'0x9c') --> start of speech
        {
          out.list_of_speech.push_back({ i, speech });
        }
        if (stmp[i + 2] == -99)//'0x9c') --> end of speech 
        {
          out.list_of_speech.push_back({ i+1, standard }); // include the closing citation mark
        }
      }
    }
  }
}