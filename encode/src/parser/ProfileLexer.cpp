
#include <fstream>
#include <iostream>
#include <string>
#include <map>

#include <assert.h>
#include <cctype>

#include "ProfileLexer.h"


ProfileLexer::StringToTokenMap ProfileLexer::string2token = {
    {"PointerEncoding", POINTERENCODING},
    {"GEPExpansion", GEPEXPANSION},
    {"AccumulateBeforeDecode", ACCUMULATEBEFOREDECODE},
    {"CheckAfterDecode", CHECKAFTERDECODE},
    {"PinChecks", PINCHECKS},
    {"AccumulateChecks", ACCUMULATECHECKS},
    {"DuplicateLoad", DUPLICATELOAD},
    {"CheckAfterStore", CHECKAFTERSTORE},
    {"Arithmetic", ARITHMETIC},
    {"Bitwise", BITWISE},
    {"Comparison", COMPARISON},
    {"GEP", GEP},
    {"Memory", MEMORY},
    {"Before", BEFORE},
    {"After", AFTER}
};

ProfileLexer::TokenToStringMap ProfileLexer::token2string = {
    {POINTERENCODING, "PointerEncoding"},
    {GEPEXPANSION, "GEPexpansion"},
    {ACCUMULATEBEFOREDECODE, "AccumulateBeforeDecode"},
    {CHECKAFTERDECODE, "CheckAfterDecode"},
    {PINCHECKS, "PinChecks"},
    {ACCUMULATECHECKS, "AccumulateChecks"},
    {DUPLICATELOAD, "DuplicateLoad"},
    {CHECKAFTERSTORE, "CheckAfterStore"},
    {ARITHMETIC, "Arithmetic"},
    {BITWISE, "Bitwise"},
    {COMPARISON, "Comparison"},
    {GEP, "GEP"},
    {MEMORY, "Memory"},
    {BEFORE, "Before"},
    {AFTER, "After"}
};

ProfileLexer::Token ProfileLexer::lex() {
  char c;
  Src.get(c);

  while (!Src.eof()) {
    if (isspace(c)) {
      Src.get(c);
      continue;
    } else if (c == ',') {
      return ProfileLexer::COMMA;
    } else if (c == ';') {
      return ProfileLexer::SEMI;
    } else if (c == ':') {
      return ProfileLexer::COLON;
    } else if (isalpha(c)) {
      std::string s = ""; s += c;
      Src.get(c);
      while (!Src.eof() && isalpha(c)) {
        s += c;
        Src.get(c);
      }
      Src.unget();
      if (string2token.find(s) != string2token.end()) {
        return string2token[s];
      } else {
        return ProfileLexer::ERROR;
      }
    } else {
      return ProfileLexer::ERROR;
    }
  }
  return ProfileLexer::END;
}

std::string ProfileLexer::tokenAsString(Token t) {
  std::string s = "";

  switch(t) {
  case END:
  case ERROR:
    break;
  case COLON:
    s = ":";
    break;
  case COMMA:
    s = ",";
    break;
  case SEMI:
    s = ";";
    break;
  default:
    if (token2string.find(t) != token2string.end()) {
      s = token2string[t];
    }
    break;
  }

  return s;
}

/* For testing purposes:
int main() {
  std::ifstream is("test.ep");

  ProfileLexer L(is);
  ProfileLexer::Token tok = L.lex();

  while (tok != ProfileLexer::END) {
    assert(tok != ProfileLexer::ERROR);

    std::cout << ProfileLexer::tokenAsString(tok);
    if (tok == ProfileLexer::COMMA || tok == ProfileLexer::COLON)
      std::cout << " ";
    else if (tok == ProfileLexer::SEMI)
      std::cout << std::endl;

    tok = L.lex();
  }

}
*/
