/* Grammar for encoding configurations is as follows:
 * (terminals are capitalized)
 *
 * configuration = directives_list
 *
 * directives_list = directive directives_list
 *                  | empty
 * (Note that a directive is always terminated by a 'SEMI'.)
 *
 * directive = PROFILE SEMI
 *           | operation_directive
 *
 * operation_directive = OPERATION COLON operation_positions SEMI
 *
 * operation_positions = POSITION COMMA operation_positions
 *                     | POSITION
 *
 */
#include "Parser.h"

#include <fstream>


std::set<ProfileLexer::Token> Parser::profileTokens = {
  ProfileLexer::POINTERENCODING,
  ProfileLexer::GEPEXPANSION,
  ProfileLexer::ACCUMULATEBEFOREDECODE,
  ProfileLexer::CHECKAFTERDECODE,
  ProfileLexer::PINCHECKS,
  ProfileLexer::ACCUMULATECHECKS,
  ProfileLexer::DUPLICATELOAD,
  ProfileLexer::CHECKAFTERSTORE
};

std::set<ProfileLexer::Token> Parser::operationTokens = {
  ProfileLexer::ARITHMETIC,
  ProfileLexer::BITWISE,
  ProfileLexer::COMPARISON,
  ProfileLexer::GEP,
  ProfileLexer::MEMORY,
  ProfileLexer::CALL,
};

EncodingProfile Parser::parse() {
  parseConfiguration();
  return get();
}

void Parser::parseConfiguration() {
  curToken = Lexer.lex();
  parseDirectivesList();
}

void Parser::parseDirectivesList() {
  while (curToken != ProfileLexer::END && !Error) {
    parseDirective();
  }
}

void Parser::parseDirective() {
  if (profileTokens.find(curToken) != profileTokens.end()) {
    EncodingProfile::Profile prof = parseProfile();
    Error |= (curToken != ProfileLexer::SEMI);
    if (Error) return;

    result.addProfile(prof);
  } else if (operationTokens.find(curToken) != operationTokens.end()) {
    parseOperationDirective();
    Error |= (curToken != ProfileLexer::SEMI);
  } else {
    Error |= 1;
  }
  curToken = Lexer.lex();
}

void Parser::parseOperationDirective() {
  EncodingProfile::Operation op = parseOperation();
  Error |= (curToken != ProfileLexer::COLON);
  if (Error) return;

  curToken = Lexer.lex();
  parseOperationPositions(op);
  Error |= (curToken != ProfileLexer::SEMI);
}

void Parser::parseOperationPositions(EncodingProfile::Operation op) {
  EncodingProfile::Position pos = parsePosition();
  Error |= (curToken != ProfileLexer::COMMA)
           && (curToken != ProfileLexer::SEMI);
  if (Error) return;

  result.addOperationWithPosition(op, pos);

  if (curToken == ProfileLexer::COMMA) {
    curToken = Lexer.lex();
    parseOperationPositions(op);
  }
}

EncodingProfile::Profile Parser::parseProfile() {
  EncodingProfile::Profile prof;
  switch (curToken) {
  case ProfileLexer::POINTERENCODING:
    prof = EncodingProfile::PointerEncoding;
    break;
  case ProfileLexer::GEPEXPANSION:
    prof = EncodingProfile::GEPExpansion;
    break;
  case ProfileLexer::ACCUMULATEBEFOREDECODE:
      prof = EncodingProfile::AccumulateBeforeDecode;
      break;
  case ProfileLexer::CHECKAFTERDECODE:
    prof = EncodingProfile::CheckAfterDecode;
    break;
  case ProfileLexer::PINCHECKS:
    prof = EncodingProfile::PinChecks;
    break;
  case ProfileLexer::ACCUMULATECHECKS:
    prof = EncodingProfile::AccumulateChecks;
    break;
  case ProfileLexer::DUPLICATELOAD:
    prof = EncodingProfile::DuplicateLoad;
    break;
  case ProfileLexer::CHECKAFTERSTORE:
    prof = EncodingProfile::CheckAfterStore;
    break;
  default:
    Error |= 1;
    return prof;
  };
  curToken = Lexer.lex();
  return prof;
}

EncodingProfile::Operation Parser::parseOperation() {
  EncodingProfile::Operation op;
  switch (curToken) {
  case ProfileLexer::ARITHMETIC:
    op = EncodingProfile::Arithmetic;
    break;
  case ProfileLexer::BITWISE:
    op = EncodingProfile::Bitwise;
    break;
  case ProfileLexer::COMPARISON:
    op = EncodingProfile::Comparison;
    break;
  case ProfileLexer::GEP:
    op = EncodingProfile::GEP;
    break;
  case ProfileLexer::MEMORY:
    op = EncodingProfile::Memory;
    break;
  case ProfileLexer::CALL:
    op = EncodingProfile::Call;
    break;
  default:
    Error |= 1;
    return op;
  };
  curToken = Lexer.lex();
  return op;
}

EncodingProfile::Position Parser::parsePosition() {
  EncodingProfile::Position pos;
  switch (curToken) {
  case ProfileLexer::BEFORE:
    pos = EncodingProfile::Before;
    break;
  case ProfileLexer::AFTER:
    pos = EncodingProfile::After;
    break;
  default:
    Error |= 1;
    return pos;
  };
  curToken = Lexer.lex();
  return pos;
}

// For testing purposes:
/*
int main() {
  std::ifstream is("test.ep");

  ProfileLexer L(is);
  Parser P(L);

  P.parseConfiguration();
  if (P.error()) return 1;
  P.get().print();

  return 0;
}
*/
