#pragma once
#include "string"
#include <list>

#include <iostream>
#include <fstream>
#include <sstream>

enum text_type { standard = 0, speech = 1, speech_bilbo=2 };

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

  //  “ - start of speech, this is 3 bytes: 226 128 156 (0xe2, 0x80, 0x9c)
  //  ” - end of speech
  std::string stmp = buffer.str();
  for (int i = 0; i < (int)stmp.length(); i++)
  {
// http://www.unit-conversion.info/texttools/ascii/
    if (stmp[i] == -30)
    {
      if (stmp[i+1] == -128)
      {
        if (stmp[i+2] == -100) // --> start of speech
        {
          out.list_of_speech.push_back({ i, speech });
        }
        if (stmp[i + 2] == -99) // --> end of speech 
        {
          out.list_of_speech.push_back({ i+1, standard }); // include the closing citation mark
        }
      }
    }
  }
}

void read_and_parse2(const std::string filename, high_text& out)
{
  // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
  std::ifstream t(filename);
  std::stringstream buffer;
  buffer << t.rdbuf();
  out.text = buffer.str();
  out.list_of_speech.clear();

  //  " - just 1 type of citation mark
  std::string stmp = buffer.str();
  int ifound=0;
  int style=(int)standard;
  for (int i = 0; i < (int)stmp.length(); i++)
  {
    if (stmp[i] == '"')
    {
//      int style = (ifound++ % 2); // alternate, this might cause error, when text is not clean
      style = 1 - style;
      // correction:
      if ((stmp[i + 1] == ' ') &&
        (stmp[i + 2] == 's') &&
        (stmp[i + 3] == 'a') &&
        (stmp[i + 4] == 'i') &&
        (stmp[i + 5] == 'd')) style = (int)speech;

      if ((stmp[i + 1] == ' ') &&
        (stmp[i + 2] == 's') &&
        (stmp[i + 3] == 'a') &&
        (stmp[i + 4] == 'i') &&
        (stmp[i + 5] == 'd') &&
        (stmp[i + 6] == ' ') && 
        (stmp[i + 7] == 'B') && 
        (stmp[i + 8] == 'i') && 
        (stmp[i + 9] == 'l') && 
        (stmp[i + 10] == 'b') && 
        (stmp[i + 11] == 'o')) style = (int)speech_bilbo;

      if ((style==1) || (style==2))
        out.list_of_speech.push_back( {i+1, (text_type)style} );
      else
        out.list_of_speech.push_back( {i, (text_type)style} );

      if (style == 2) style = 1; // hack! set back, so that style=1-style works as above
    }
  }
}
