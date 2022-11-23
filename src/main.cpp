#include <fstream>
#include <iostream>
#include <string>

#include "parser/Parser.hpp"
#include "tokenizer/Tokenizer.hpp"

const std::string styletag =
    "<link rel=\"stylesheet\" href=\"./result/result.css\" />";

int main(int argc, char const* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: ./md2html /path/to/markdown.md" << std::endl;
    return 1;
  }

  std::ifstream ifs(argv[1]);
  if (!ifs) {
    std::cerr << "failed to open: '" << argv[1] << "'" << std::endl;
    return 1;
  }

  std::string s;
  char buf[1024];
  while (!ifs.eof()) {
    ifs.read(buf, 1024);
    s.append(buf, ifs.gcount());
  }

  std::cout << "[info] start tokenizing" << std::endl;
  m2h::Tokenizer tokenizer;
  std::vector<m2h::Token> tokens = tokenizer.tokenize(s.c_str());

  std::cout << "[info] start parsing" << std::endl;
  m2h::Parser parser;
  std::vector<m2h::Node*> nodes = parser.parse(tokens);

  std::cout << "[info] generating html (./result/result.html)" << std::endl;
  std::ofstream ofs("./result/result.html");
  ofs << styletag << std::endl;
  for (auto&& node : nodes) {
    node->print(ofs, "");
  }
}
