#pragma once

#include "Node.hpp"

namespace m2h {

struct ParsingContext {
  Node *parent;
  int index;
  int indent;

  Node *prevSibling() {
    auto children = parent->children;
    return children.empty() ? nullptr : children.back();
  }

  void append(Node *node) { parent->children.push_back(node); }
};

}  // namespace m2h
