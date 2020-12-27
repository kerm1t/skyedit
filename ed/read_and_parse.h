#pragma once
#include "string"
#include <list>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <algorithm> // erase
#include <regex>

// entities, which are extracted from the text, i.e. speakers, speech blocks ---------------------------------
enum text_type { standard = 0, speech = 1, speech_bilbo=2 };

struct speaker
{
  std::string name;
  int occurence;
  inline bool operator < (const speaker& struct2) // https://stackoverflow.com/questions/1380463/sorting-a-vector-of-custom-objects
  {
    return (name < struct2.name);
  }
};

struct speech_at {
  int pos;
  text_type type;
  std::string name; // speaker
};
// entities, which are extracted from the text, i.e. speakers, speech blocks ---------------------------------


struct high_text {
  std::string text;
// ---------------------------------------------------------
// cannot access a list element directly, just by traversal.
// rather wanting a vector here.
// ---------------------------------------------------------
  std::list<speech_at> list_of_speech; // https://www.geeksforgeeks.org/list-cpp-stl/?ref=lbp
};

// 2do:
// Unicode text: https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
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

void read_and_parse2(const std::string filename, high_text& out, std::vector<speaker>& speakers)
{
  // init
  speakers.clear();
  out.list_of_speech.clear();

  // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
  std::ifstream t(filename);
  std::stringstream buffer;
///  buffer.unsetf(std::ios::skipws); // try out!!!
  buffer << t.rdbuf();
  out.text = buffer.str();

  //  " - just 1 type of citation mark
  std::string stmp = buffer.str();

// both methods don't work (on std::string)
//////// -> coloring not functioning anymore ...  stmp.erase(std::remove(stmp.begin(), stmp.end(), '\n'), stmp.end());
///  std::regex newlines_re("\n+");
///  auto result = std::regex_replace(stmp, newlines_re, "");
///  stmp = result;
//  for (int i=0;i<buffer.)

  int ifound=0;
  int style = 1; // so 1-style will be 0 as a start!! ... (int)standard;
  for (int i = 0; i < (int)stmp.length(); i++)
  {
    if (stmp[i] == '"')
    {
      style = 1 - style;
      std::string name;
      int start = 0;
      if ((stmp[i + 1] == ' ') &&
        (stmp[i + 2] == 's') &&
        (stmp[i + 3] == 'a') &&
        (stmp[i + 4] == 'i') &&
        (stmp[i + 5] == 'd')) start = 7;
      else
      if ((stmp[i + 1] == ' ') &&
        (stmp[i + 2] == 'c') &&
        (stmp[i + 3] == 'r') &&
        (stmp[i + 4] == 'i') &&
        (stmp[i + 5] == 'e') &&
        (stmp[i + 6] == 'd')) start = 8;
      else
      if ((stmp[i + 1] == ' ') &&
        (stmp[i + 2] == 'a') &&
        (stmp[i + 3] == 's') &&
        (stmp[i + 4] == 'k') &&
        (stmp[i + 5] == 'e') &&
        (stmp[i + 6] == 'd')) start = 8;
      else
        if ((stmp[i + 1] == ' ') &&
          (stmp[i + 2] == 'i') &&
          (stmp[i + 3] == 'n') &&
          (stmp[i + 4] == 'q') &&
          (stmp[i + 5] == 'u') &&
          (stmp[i + 6] == 'i') &&
          (stmp[i + 7] == 'r') &&
          (stmp[i + 8] == 'e') &&
          (stmp[i + 9] == 'd')) start = 11;
        else
          if ((stmp[i + 1] == ' ') &&
            (stmp[i + 2] == 'r') &&
            (stmp[i + 3] == 'e') &&
            (stmp[i + 4] == 'p') &&
            (stmp[i + 5] == 'l') &&
            (stmp[i + 6] == 'i') &&
            (stmp[i + 7] == 'e') &&
            (stmp[i + 8] == 'd')) start = 10;

      // said, 2do: added, answered, asked, continued, cried, exclaimed, inquired, interposed, interrupted,
      //            murmured, remarked, replied, responded, returned, sighed,  
      //            thought!!, whispered, ...
      if (start > 0)
      {
        style = (int)speech;

        // find name after "said ..."   stmp[i + 6] == ' '
        int n = 0;
        // 2do: M. de Villefort
        while ((stmp[i + start + n] != ' ') &&
          (stmp[i + start + n] != '.') &&
          (stmp[i + start + n] != ',') &&
          (stmp[i + start + n] != ';') &&
          (stmp[i + start + n] != ':'))
        {
          name = name + stmp[i+start+n];
          n++;
        }
        if ((name.compare("the") == 0) || (name.compare("a") == 0))
        {
          n++; // jump over the space
          name = name + " "; // add the space ;-)
          while ((stmp[i + start + n] != ' ') &&
            (stmp[i + start + n] != '.') &&
            (stmp[i + start + n] != ',') &&
            (stmp[i + start + n] != ';') &&
            (stmp[i + start + n] != ':'))
          {
            name = name + stmp[i + start + n];
            n++;
          }
        }

        std::cout << name << std::endl;

        // add speaker to list (if new) or increase occurence
        bool found = false;
        for (std::vector<speaker>::iterator it = std::begin(speakers); it != std::end(speakers); ++it)
        {
          if ((*it).name == name)
          {
            found = true;
            (*it).occurence++;
            break;
          }
        }
        if (!found) speakers.push_back({ name, 1 });
        // 2do: return an index here, which can be used below ...
        style = 2;
      }

      if ((style==1) || (style==2)) // 2do: > 0
        out.list_of_speech.push_back( {i+1, (text_type)style, name});
      else
        out.list_of_speech.push_back( {i, (text_type)style} );

/// 2do: 2 (e.g. Bilbo) or greater
      if (style == 2) style = 1; // hack! set back, so that style=1-style works as above
    }
  }
}
