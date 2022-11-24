#pragma once

#include <string>

namespace m2h {

// ------------------------------------
// Predicates
// ------------------------------------
inline char toLower(char c) { return 'A' <= c && c <= 'Z' ? c + 'a' - 'A' : c; }
inline char toUpper(char c) { return 'a' <= c && c <= 'z' ? c - 'a' - 'A' : c; }
inline bool isAlpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}
inline bool isNonZeroDigit(char c) { return '1' <= c && c <= '9'; }
inline bool isDigit(char c) { return '0' == c || isNonZeroDigit(c); }
inline bool isCR(char c) { return c == '\r'; }
inline bool isLF(char c) { return c == '\n'; }
inline bool isCrlf(char c) { return isCR(c) || isLF(c); }
inline bool isSpace(char c) { return c == ' ' || c == '\t'; }
inline bool isTab(char c) { return c == '\t'; }
inline bool isLetter(char c) { return isAlpha(c) || isDigit(c) || c == '_'; }

bool startWith(const char* p, const std::string& s) {
  const std::size_t len = s.size();
  for (int i = 0; i < len; ++i) {
    if (p[i] != s[i]) return false;
  }
  return true;
}

bool oneof(char p, const char* s) {
  while (*s != '\0') {
    if (p == *s) return true;
    ++s;
  }
  return false;
}

std::string escape(char c) {
  if (c == '<') return "&lt;";
  if (c == '>') return "&gt;";
  if (c == '&') return "&amp;";
  if (c == '"') return "&quot;";
  if (c == '\'') return "&#39;";
  std::string s;
  return s + c;
}

std::string escape(const std::string& s) {
  auto ret = std::string{};
  for (auto c : s) {
    ret += escape(c);
  }
  return ret;
}

template <class Predicate>
int skipWhile(const char*& p, Predicate&& pred) {
  int count = 0;
  while (pred(*p)) {
    ++p;
    ++count;
  }
  return count;
}

template <class Predicate>
std::string readWhile(const char*& p, Predicate&& pred) {
  const char* p0 = p;
  while (pred(*p)) ++p;
  return {p0, p};
}


int skipWs(const char*& p) { return skipWhile(p, isSpace); }



std::string trimLeft(const std::string& s) {
  auto first = s.begin(), last = s.end();
  while (isSpace(*first) && first != last) {
    ++first;
  }
  return {first, last};
}

std::string trimRight(const std::string& s) {
  auto first = s.begin(), last = first + s.size() - 1;
  while (isSpace(*last) && first != last) {
    --last;
  }
  return {first, last + 1};
}

std::string trim(const std::string& s) { return trimLeft(trimRight(s)); }

}
