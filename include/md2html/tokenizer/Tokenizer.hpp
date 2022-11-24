#pragma once

#include <string>
#include <vector>

#include "../ParsingUtility.hpp"
#include "../TypeAlias.hpp"
#include "Token.hpp"

namespace m2h {

struct TokenizerContext {
  const char* savepoint = nullptr;
};

class Tokenizer {
 public:
  explicit Tokenizer() : tokens{}, context{} {}

  CRef<std::vector<Token>> tokenize(const char* p) {
    while (*p != '\0') {
      if (isSpace(*p)) {
        // Indent
        bool ok = tokenizeIndent(p);
        if (!ok) {
          p = context.savepoint;
          goto fallback;
        }
        continue;
      }

      if (isCrlf(*p)) {
        // NewLine
        tokenizeNewLine(p);
        continue;
      }

      if (*p == '`') {
        // BackQuotes
        context.savepoint = p;
        bool ok = tokenizeBackQuote(p);
        if (!ok) {
          p = context.savepoint;
          goto fallback;
        }
        continue;
      }

      if (oneof(*p, "[]()")) {
        // brackets
        context.savepoint = p;
        bool ok = tokenizeBracket(p);
        if (!ok) {
          p = context.savepoint;
          goto fallback;
        }
        continue;
      }

      if (isDigit(*p)) {
        // OrderedListItems
        context.savepoint = p;
        bool ok = tokenizeOrderedList(p);
        if (!ok) {
          p = context.savepoint;
          goto fallback;
        }
        continue;
      }

      if (oneof(*p, "+-*_")) {
        // Horizontal
        context.savepoint = p;
        bool ok = tokenizeHorizontal(p);
        if (ok) continue;
        p = context.savepoint;

        // UnorderedListItems
        ok = tokenizeUnorderedList(p);
        if (!ok) {
          p = context.savepoint;
          goto fallback;
        }
        continue;
      }

      if (*p == '>') {
        // BlockQuote
        context.savepoint = p;
        bool ok = tokenizeBlockQuote(p);
        if (!ok) {
          p = context.savepoint;
          goto fallback;
        }
        continue;
      }

      if (*p == '#') {
        // Heading
        context.savepoint = p;
        bool ok = tokenizeHeading(p);
        if (!ok) {
          p = context.savepoint;
          goto fallback;
        }
        continue;
      }

    fallback:
      tokenizeText(p);
    }
    tokens.emplace_back(TokenKind::Eof, "", p);
    return tokens;
  }

 private:
  bool tokenizeIndent(const char*& p) {
    int count = 0;
    const char* loc = p;
    while (isSpace(*p)) {
      if (*p == '\t') {
        count += 4;
      } else {
        count += 1;
      }
      ++p;
      if (count >= 4) break;
    }
    if (count == 0) return false;
    tokens.emplace_back(TokenKind::Indent, std::string(count, ' '), loc);
    return true;
  }

  bool tokenizeHeading(const char*& p) {
    const char* loc = p;
    int count = 0;
    while (*p == '#') {
      ++count;
      ++p;
    }
    if (!isSpace(*p)) return false;
    ++p;
    tokens.emplace_back(TokenKind::Prefix, std::string(count, '#') + " ", loc);
    return true;
  }

  bool tokenizeHorizontal(const char*& p) {
    const char* loc = p;
    int count = 0;
    while (oneof(*p, "-*_") || isSpace(*p)) {
      if (!isSpace(*p)) ++count;
      ++p;
    }

    if (count < 3) return false;
    if (!isCrlf(*p)) return false;
    ++p;

    tokens.emplace_back(TokenKind::Horizontal, std::string{loc, p}, loc);
    return true;
  }

  bool tokenizeBackQuote(const char*& p) {
    const char* loc = p;
    if (*p != '`') return false;
    ++p;
    tokens.emplace_back(TokenKind::BackQuote, "`", loc);
    return true;
  }

  bool tokenizeBlockQuote(const char*& p) {
    const char* loc = p;
    if (*p != '>') return false;
    ++p;
    if (isSpace(*p)) ++p;
    tokens.emplace_back(TokenKind::Prefix, "> ", loc);
    return true;
  }

  bool tokenizeBracket(const char*& p) {
    const char* loc = p;
    if (!oneof(*p, "[]()")) return false;
    tokens.emplace_back(TokenKind::Bracket, std::string{*p}, loc);
    ++p;
    return true;
  }

  bool tokenizeText(const char*& p) {
    const char* p1 = p;
    while (!isCrlf(*p)) {
      while (!oneof(*p, "!*`[]()_") && !isCrlf(*p)) ++p;
      auto text = std::string{p1, p};
      if (!text.empty()) {
        tokens.emplace_back(TokenKind::Text, text, p1);
        p1 = p;
        continue;
      }
      if (isCrlf(*p)) return true;
      if (oneof(*p, "*_")) {
        tokens.emplace_back(TokenKind::Emphasis, std::string{*p}, p1);
        p1 = ++p;
        continue;
      }
      if (*p == '!') {
        tokens.emplace_back(TokenKind::Exclamation, "!", p1);
        p1 = ++p;
        continue;
      }
      if (*p == '`') {
        tokens.emplace_back(TokenKind::BackQuote, "`", p1);
        p1 = ++p;
        continue;
      }
      if (oneof(*p, "[]()")) {
        tokens.emplace_back(TokenKind::Bracket, std::string{*p}, p1);
        p1 = ++p;
        continue;
      }
    }
    return true;
  }

  bool tokenizeNewLine(const char*& p) {
    const char* loc = p;
    if (isCR(*p)) ++p;
    if (isLF(*p)) ++p;
    if (loc == p) return false;
    tokens.emplace_back(TokenKind::NewLine, "", loc);
    return true;
  }

  bool tokenizeUnorderedList(const char*& p) {
    const char* loc = p;
    if (!oneof(*p, "*+-")) return false;
    char c = *p;
    ++p;
    if (!isSpace(*p)) return false;
    ++p;
    tokens.emplace_back(TokenKind::Prefix, std::string{c} + " ", loc);
    return true;
  }

  bool tokenizeOrderedList(const char*& p) {
    const char* loc = p;
    int count = skipWhile(p, isDigit);
    if (*p != '.') return false;
    ++p;
    tokens.emplace_back(TokenKind::Prefix, std::string("1", count) + ".", loc);
    return true;
  }

 private:
  std::vector<Token> tokens;
  TokenizerContext context;
};

}  // namespace m2h
