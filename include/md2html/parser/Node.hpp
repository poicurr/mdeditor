#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace std::string_literals;

namespace m2h {

enum class NodeType {
  None = 0,
  Paragraph,
  BlockQuote,
  UnorderedList,
  UnorderedListItem,
  OrderedList,
  OrderedListItem,
  EmptyLine,
  Horizontal,
  Heading,
  InlineCode,
  CodeBlock,
};

struct Node {
  explicit Node(NodeType&& type) : type{type}, children{} {}
  virtual void print(std::ostream& ost, const std::string& prefix) = 0;
  virtual NodeType getType() { return type; }
  void addChild(Node* node) { children.push_back(node); }
  NodeType type;
  std::vector<Node*> children;
};

struct RootNode : Node {
  RootNode() : Node(NodeType::None) {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    for (auto&& child : children) {
      child->print(ost, "");
    }
  }
};

struct HeadingNode : Node {
  HeadingNode(int level, const std::string& heading)
      : Node(NodeType::Heading), level{level}, heading{heading} {}

  virtual void print(std::ostream& ost, const std::string& prefix) override {
    const std::string lvl = std::to_string(level);
    std::string t1 = "<h"s + lvl + ">"s;
    std::string t2 = "</h"s + lvl + ">"s;
    ost << prefix << t1 << heading << t2 << std::endl;
  }

  int level;
  std::string heading;
};

struct BlockQuoteNode : Node {
  BlockQuoteNode() : Node(NodeType::BlockQuote) {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    ost << prefix << "<blockquote>" << std::endl;
    for (auto&& childNode : children) {
      childNode->print(ost, prefix + "  ");
    }
    ost << prefix << "</blockquote>" << std::endl;
  }
};

struct ParagraphNode : Node {
  ParagraphNode(int index, const std::string& text)
      : Node(NodeType::Paragraph), index{index}, text{text} {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    ost << prefix << "<p>" << text << "</p>" << std::endl;
  }
  int index;
  std::string text;
};

struct OrderedListNode : Node {
  OrderedListNode(int index) : Node(NodeType::OrderedList), index{index} {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    ost << prefix << "<ol>" << std::endl;
    for (auto&& childNode : children) {
      childNode->print(ost, prefix + "  ");
    }
    ost << prefix << "</ol>" << std::endl;
  }
  int index;
};

struct OrderedListItemNode : Node {
  OrderedListItemNode() : Node(NodeType::OrderedListItem) {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    ost << prefix << "<li>" << std::endl;
    if (!children.empty()) children[0]->print(ost, prefix + "  ");
    ost << prefix << "</li>" << std::endl;
  }
};

struct UnorderedListNode : Node {
  UnorderedListNode(int index) : Node(NodeType::UnorderedList), index{index} {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    ost << prefix << "<ul>" << std::endl;
    for (auto&& childNode : children) {
      childNode->print(ost, prefix + "  ");
    }
    ost << prefix << "</ul>" << std::endl;
  }
  int index;
};

struct UnorderedListItemNode : Node {
  UnorderedListItemNode() : Node(NodeType::UnorderedListItem) {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    ost << prefix << "<li>" << std::endl;
    if (!children.empty()) children[0]->print(ost, prefix + "  ");
    ost << prefix << "</li>" << std::endl;
  }
};

struct HorizontalNode : Node {
  HorizontalNode() : Node(NodeType::Horizontal) {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    ost << "<hr />" << std::endl;
  }
};

struct CodeBlockNode : Node {
  CodeBlockNode(const std::string& text)
      : Node(NodeType::CodeBlock), text{text} {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    ost << "<pre><code>";
    ost << text << std::endl;
    ost << "</code></pre>" << std::endl;
  }
  std::string text;
};

struct EmptyLineNode : Node {
  EmptyLineNode() : Node(NodeType::EmptyLine) {}
  virtual void print(std::ostream& ost, const std::string& prefix) override {
    ost << prefix << "<p><!-- empty --></p>" << std::endl;
  }
};

}  // namespace m2h
