#ifndef __PARSER_H__
#define __PARSER_H__

#include "ProfileLexer.h"
#include "Profile.h"

class Parser {
public:
  Parser(ProfileLexer &lexer) : Lexer(lexer), Error(0) {}

  void parseConfiguration();
  void parseDirectivesList();
  void parseDirective();
  void parseOperationDirective();
  void parseOperationPositions(EncodingProfile::Operation op);

  EncodingProfile::Profile parseProfile();
  EncodingProfile::Operation parseOperation();
  EncodingProfile::Position parsePosition();

  bool error() { return Error; }
  EncodingProfile get() { return result; }

  EncodingProfile parse();

private:
  ProfileLexer &Lexer;
  unsigned Error;

  ProfileLexer::Token curToken;
  EncodingProfile result;

  static std::set<ProfileLexer::Token> profileTokens;
  static std::set<ProfileLexer::Token> operationTokens;
};

#endif /* __PARSER_H__ */
