#ifndef __PROFILE_LEXER_H__
#define __PROFILE_LEXER_H__

#include <string>
#include <map>


class ProfileLexer {
public:
  enum Token {
    // from 'enum EncodingProfile::Profile':
    POINTERENCODING,
    GEPEXPANSION,
    CHECKAFTERDECODE,
    ACCUMULATEBEFOREDECODE,
    PINCHECKS,
    ACCUMULATECHECKS,
    IGNOREACCUMULATEOVERFLOW,
    DUPLICATELOAD,
    CHECKAFTERSTORE,
    // from 'enum EncodingProfile::Operation':
    ARITHMETIC,
    BITWISE,
    COMPARISON,
    GEP,
    MEMORY,
    CALL,
    // from 'enum EncodingProfile::Position':
    BEFORE,
    AFTER,
    // Separators:
    COLON,
    COMMA,
    SEMI,
    // Error token:
    ERROR,
    // End of file:
    END
  };

  ProfileLexer(std::ifstream &src) : Src(src) {}

  Token lex();

  static std::string tokenAsString(Token);

private:
  typedef std::map<std::string, Token> StringToTokenMap;
  static StringToTokenMap string2token;

  typedef std::map<Token, std::string> TokenToStringMap;
  static TokenToStringMap token2string;


  std::ifstream &Src;
};

#endif /* __PROFILE_LEXER_H__ */
