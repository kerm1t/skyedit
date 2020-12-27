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

  //  � - start of speech, this is 3 bytes: 226 128 156 (0xe2, 0x80, 0x9c)
  //  � - end of speech
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
  int style = 1;// so 1-style will be 0 as a start!! ... (int)standard;
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
        (stmp[i + 5] == 'd'))
      {
        style = (int)speech;

        // Bilbo hack!
        if (
          (stmp[i + 7] == 'B') &&
          (stmp[i + 8] == 'i') &&
          (stmp[i + 9] == 'l') &&
          (stmp[i + 10] == 'b') &&
          (stmp[i + 11] == 'o')) style = (int)speech_bilbo;

        // find name after "said ..."   stmp[i + 6] == ' '
        int n = 7;
        std::string name;
        // M. de Villefort
        while ((stmp[i + n] != ' ') && (stmp[i + n] != '.') && (stmp[i + n] != ',') && (stmp[i + n] != ';'))
        {
          name = name + stmp[i+n];
          n++;
        }
        if ((name.compare("the") == 0) || (name.compare("a") == 0))
        {
          n++; // jump over the space
          name = name + " "; // add the space ;-)
          while ((stmp[i + n] != ' ') && (stmp[i + n] != '.') && (stmp[i + n] != ',') && (stmp[i + n] != ';'))
          {
            name = name + stmp[i + n];
            n++;
          }
        }

        std::cout << name << std::endl;
//        speaker.push_back(name);
/*        if (!(std::find(speakers.begin(), speakers.end(), name) != speakers.end()))
        {
          speakers.push_back({ name, 0 });
        }
*/
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
//        for (int i=0; i < speakers.size(); i++)
//        {
//          if (speakers[i].name == name) found = true;
        }
        if (!found) speakers.push_back({ name, 1 });
      }

      if ((style==1) || (style==2))
        out.list_of_speech.push_back( {i+1, (text_type)style} );
      else
        out.list_of_speech.push_back( {i, (text_type)style} );

      if (style == 2) style = 1; // hack! set back, so that style=1-style works as above
    }
  }
}
