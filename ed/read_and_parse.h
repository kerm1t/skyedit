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

bool compare(const std::string& stmp, int i, const char* search, int len)
{
  for (int j = i; j < i+len; j++)
  {
    if (stmp[j + 2] != *search++)
    {
      return false;
    }
  }
  return true;
}

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
  int ifound = 0;
  int state = 0;
  int scintilla_state = 1; // formerly "style". so 1-state will be 0 as a start!! ... (int)standard;
  std::string token;
  int tokstart = 0;
  std::string cur_speaker; // last found speaker token
  int idx_cur_speaker = 0;

  for (int i = 0; i < (int)stmp.length(); i++)
  {
    // -----------------
    // (A) detect speech
    // -----------------
    if (stmp[i] == '"')
    {
      state = 1 - state;
      scintilla_state = 1 - scintilla_state;
      std::string name;
      int start = 0;

// 2do: include all found verbs, s. below
// 2do: speedup, evtl. als inline?
      if      (compare(stmp, i, "said", 4)) start = 7;
      else if (compare(stmp, i, "cried", 5)) start = 8;
      else if (compare(stmp, i, "asked", 5)) start = 8;
      else if (compare(stmp, i, "inquired", 8)) start = 11;
      else if (compare(stmp, i, "replied", 7)) start = 10;
      else if (compare(stmp, i, "returned", 8)) start = 11;


      int idx_speaker = -1;
      bool new_speaker = false;
      bool known_speaker = false;
      int n = 0;
      // said, 2do: added, answered, asked, continued, cried, exclaimed, inquired, interposed, interrupted,
      //            murmured, remarked, replied, responded, returned, sighed,  
      //            thought!!, whispered, ...
      if (start > 0)
      {
        scintilla_state = (int)tt_speech; // state override (which also corrects problems)

        // find name after "said ..."   stmp[i + 6] == ' '
///        int n = 0;
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
          new_speaker = true;
        }
        else
        {
          idx_speaker = cnt; // b) speaker found above
          known_speaker = true;
        }
        // 2do: return an index here, which can be used below ...
        scintilla_state = 2;
        idx_cur_speaker = idx_speaker; // small hack, we can use this when the next speech comes right after "said XXX"
      }
      else
      {
        // no direct found via "said ..." or "answered ..."
        // so we use the current speaker
        idx_speaker = idx_cur_speaker;
      }

#define MIN_STYLE_SPEAKER 4

      if ((scintilla_state ==1) || (scintilla_state ==2)) // 2do: > 0
        out.annotation_list.push_back( {i+1, (text_type)scintilla_state, MIN_STYLE_SPEAKER+idx_speaker});
      else
        out.annotation_list.push_back( {i, (text_type)scintilla_state, -1} );

/// 2do: 2 (e.g. Bilbo) or greater
      if (scintilla_state == 2) scintilla_state = 1; // hack! set back, so that state=1-state works as above

      if (new_speaker || known_speaker
///        && (state != 1) && (state != 2) // within speech doesn't work right now
        )
      {
        out.annotation_list.push_back({ i + start, tt_standard }); // TRY it out!!
        out.annotation_list.push_back({ i + start + n, tt_speech, 1 }); // TRY it out!!
      }
    }

    // -------------------------------------------------
    // (A) detect token (limited by SPACE), e.g. speaker
    // -------------------------------------------------
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
  //      out.annotation_list.push_back({ i-tokstart, tt_standard });
        int toklen = i-tokstart-1;
  ///      if ((state != 1) && (state != 2)) // within speech doesn't work right now
        {
///          out.annotation_list.push_back({ i - toklen, (text_type)scintilla_state });
///          out.annotation_list.push_back({ i, tt_speaker, 2 });
        }
        cur_speaker = token;
        idx_cur_speaker = cnt;
      }

      token.clear();
      tokstart = i;
    }
    else
      token += stmp[i];
  }

  // pass 2 - search speaker   --> ATM just 1-pass
}
