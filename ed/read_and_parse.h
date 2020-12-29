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
enum text_type { tt_standard = 0, tt_unknown_speech = 1, tt_speech=2, tt_speaker=3 };

struct speaker
{
  std::string name;
  int occurence;
  inline bool operator < (const speaker& struct2) // https://stackoverflow.com/questions/1380463/sorting-a-vector-of-custom-objects
  {
    return (name < struct2.name);
  }
};

// this is our core feature, can be speech, a speaker or whatever marked part of the text
struct annotation_at { // = highlight_at
  int pos;
  text_type type;
  int idx_speaker; // speech = -1, speaker = <n>   ... 2do: set for speech :-)
};
// entities, which are extracted from the text, i.e. speakers, speech blocks ---------------------------------


struct highlighted_corpus {
  std::string text;
// ---------------------------------------------------------
// cannot access a list element directly, just by traversal.
// rather wanting a vector here.
// ---------------------------------------------------------
  std::list<annotation_at> annotation_list; // https://www.geeksforgeeks.org/list-cpp-stl/?ref=lbp
};

// 2do:
// Unicode text: https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
void read_and_parse(const std::string filename, highlighted_corpus& out)
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
          out.annotation_list.push_back({ i, tt_speech });
        }
        if (stmp[i + 2] == -99) // --> end of speech 
        {
          out.annotation_list.push_back({ i+1, tt_standard }); // include the closing citation mark
        }
      }
    }
  }
}

/*int find_speaker(const std::vector<std::string>& speakers, const std::string name)
{
	bool found = false;
	int cnt = 0;
	for (std::vector<speaker>::iterator it = std::begin(speakers); it != std::end(speakers); ++it)
	{
		if ((*it).name == name)
		{
			found = true;
			(*it).occurence++;
			break;
		}
		cnt++;
	}
}*/

// 1 pass parsing, search for speech and speaker.
// Algo: As soon as a speaker is found, it serves as a state for the subsequent speed tihout clear indicator
void read_and_parse2(const std::string filename, highlighted_corpus& out, std::vector<speaker>& speakers)
{
  // init
  speakers.clear();
  out.annotation_list.clear();

  // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
  std::ifstream t(filename);
  std::stringstream buffer;
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

  // pass 1 - search speech
  int ifound=0;
  int style = 1; // so 1-style will be 0 as a start!! ... (int)standard;
  std::string token;
  int tokstart = 0;
  std::string cur_speaker; // last found speaker token
  int idx_cur_speaker = 0;

  for (int i = 0; i < (int)stmp.length(); i++)
  {
    if (stmp[i] == '"')
    {
      style = 1 - style;
      std::string name;
      int start = 0;
// 2do: put the speech recognizer to external function (and include all found verbs)
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
          else
            if ((stmp[i + 1] == ' ') &&
              (stmp[i + 2] == 'r') &&
              (stmp[i + 3] == 'e') &&
              (stmp[i + 4] == 't') &&
              (stmp[i + 5] == 'u') &&
              (stmp[i + 6] == 'r') &&
              (stmp[i + 7] == 'n') &&
              (stmp[i + 8] == 'e') &&
              (stmp[i + 9] == 'd')) start = 11;

      int idx_speaker = -1;
      // said, 2do: added, answered, asked, continued, cried, exclaimed, inquired, interposed, interrupted,
      //            murmured, remarked, replied, responded, returned, sighed,  
      //            thought!!, whispered, ...
      if (start > 0)
      {
        style = (int)tt_speech;

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
        int cnt=0;
        for (std::vector<speaker>::iterator it = std::begin(speakers); it != std::end(speakers); ++it)
        {
          if ((*it).name == name)
          {
            found = true;
            (*it).occurence++;
            break;
          }
          cnt++;
        }
        if (!found)
        {
          idx_speaker = speakers.size(); // a) new speaker
          speakers.push_back({ name, 1 });
//		  out.annotation_list.push_back({ i + start, tt_standard }); // TRY it out!!
//		  out.annotation_list.push_back({ i + start + n, tt_speech, 1 }); // TRY it out!!
		}
        else
        {
          idx_speaker = cnt; // b) speaker found above
        }
        // 2do: return an index here, which can be used below ...
        style = 2;
		idx_cur_speaker = idx_speaker; // small hack, we can use this when the next speech comes right after "said XXX"
	  }
	  else
	  {
		  // no direct found via "said ..." or "answered ..."
		  // so we use the current speaker
		  idx_speaker = idx_cur_speaker;
	  }

#define MIN_STYLE_SPEAKER 3

      if ((style==1) || (style==2)) // 2do: > 0
        out.annotation_list.push_back( {i+1, (text_type)style, MIN_STYLE_SPEAKER+idx_speaker});
      else
        out.annotation_list.push_back( {i, (text_type)style, -1} );

/// 2do: 2 (e.g. Bilbo) or greater
      if (style == 2) style = 1; // hack! set back, so that style=1-style works as above
    }

	if (stmp[i] == ' ')
	{
		int cnt = 0;
		bool found = false;
		for (std::vector<speaker>::iterator it = std::begin(speakers); it != std::end(speakers); ++it)
		{
			if ((*it).name == token)
			{
				found = true;
				(*it).occurence++;
				break;
			}
			cnt++;
		}
		if (found)
		{
//			out.annotation_list.push_back({ i-tokstart, tt_standard });
			int toklen = i-tokstart-1;
			out.annotation_list.push_back({ i-toklen, tt_standard });
			out.annotation_list.push_back({ i, tt_speech, 1 });
			cur_speaker = token;
			idx_cur_speaker = cnt;
		}

		token.clear();
		tokstart = i;
	}
	else
		token += stmp[i];
  }

/*  // pass 2 - search speaker
//  std::vector<std::string> alltoks;
  const char* str = stmp.c_str();
  int cnt = 0;
  do
  {
    const char* begin = str;

    while (*str != ' ' && *str)
    {
      str++;
      cnt++;
    }

    std::string tok = std::string(begin, str);
//    alltok.push_back(tok);
    bool found = false;
    for (std::vector<speaker>::iterator it = std::begin(speakers); it != std::end(speakers); ++it)
    {
      if ((*it).name == tok)
      {
        found = true;
        (*it).occurence++;
        break;
      }
      cnt++;
    }
    if (found)
      out.annotation_list.push_back({ cnt, (text_type)1, 1 });

  } while (0 != *str++);
  */
}
