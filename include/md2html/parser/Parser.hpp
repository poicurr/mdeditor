#pragma once

#include <algorithm>
#include <string>

#include "../ParsingUtility.hpp"
#include "../tokenizer/Token.hpp"
#include "Node.hpp"
#include "ParsingContext.hpp"

namespace m2h {

using token_iterator = std::vector<Token>::iterator;

class Parser {
 public:
  Parser() {}

  std::vector<Node *> parse(std::vector<Token> &tokens) {
    Node *root = new RootNode();
    context.parent = root;
    context.index = 0;
    context.indent = 0;

    token_iterator it = tokens.begin();
    while (it != tokens.end()) {
      auto bak = it;

      if (parseIndent(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseBlockQuote(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseOrderedList(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseUnorderedList(root, it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseCodeBlock1(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseCodeBlock2(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseInlineCode1(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseInlineCode2(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseImage(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseLink(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseEmphasis(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseHeading(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseHorizontal(it)) {
        goto next;
      } else {
        it = bak;
      }

      if (parseNewline(root, it)) {
        goto next;
      } else {
        it = bak;
      }

      // fallback
      parseParagraph(it);

    next:
      ++it;
    }
    return root->children;
  }

 private:
  bool parseParagraph(token_iterator &it) {
    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto paragraph = static_cast<ParagraphNode *>(prevSibling);
      if (paragraph->index == context.index) {
        paragraph->text += "\n" + it->value;
        return true;
      }
    }
    context.append(new ParagraphNode(context.index, it->value));
    return true;
  }

  bool parseHeading(token_iterator &it) {
    if (it->kind != TokenKind::Prefix) return false;
    int level = 0;
    for (char c : it->value) {
      if (c != '#') break;
      ++level;
    }
    if (level == 0) return false;
    ++it;
    if (it->kind != TokenKind::Text) return false;
    const std::string value = it->value;
    context.append(new HeadingNode(level, value));
    return true;
  }

  bool parseIndent(token_iterator &it) {
    if (it->kind != TokenKind::Indent) return false;
    context.index += it->value.size();
    context.indent += it->value.size();
    return true;
  }

  bool parseHorizontal(token_iterator &it) {
    if (it->kind != TokenKind::Horizontal) return false;
    context.append(new HorizontalNode());
    return true;
  }

  bool parseNewline(Node *root, token_iterator &it) {
    if (it->kind != TokenKind::NewLine) return false;
    auto prevToken = it - 1;
    if (prevToken->value == "> ") {
      context.append(new EmptyLineNode());
    }
    if (prevToken->kind == TokenKind::NewLine) {
      context.append(new EmptyLineNode());
    }
    context.parent = root;
    context.index = 0;
    context.indent = 0;
    return true;
  }

  bool parseInlineCode1(token_iterator &it) {
    for (int i = 0; i < 2; ++i, ++it)
      if (it->kind != TokenKind::BackQuote) return false;

    auto code = std::string{};
    while (it->kind != TokenKind::BackQuote ||
           (it + 1)->kind != TokenKind::BackQuote) {
      if ((it + 1)->kind == TokenKind::Eof) return false;
      code += it->value;
      ++it;
    }

    for (int i = 0; i < 2; ++i, ++it)
      if (it->kind != TokenKind::BackQuote) return false;
    --it;

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto paragraph = static_cast<ParagraphNode *>(prevSibling);
      paragraph->text += "<code>" + escape(code) + "</code>";
    }

    return true;
  }

  bool parseInlineCode2(token_iterator &it) {
    if (it->value != "`") return false;
    ++it;

    auto code = std::string{};
    while (it->kind != TokenKind::BackQuote && it->kind != TokenKind::Eof) {
      if (it->kind == TokenKind::Eof) return false;
      code += it->value;
      ++it;
    }

    if (it->value != "`") return false;

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto paragraph = static_cast<ParagraphNode *>(prevSibling);
      paragraph->text += "<code>" + escape(code) + "</code>";
    } else {
      context.append(new ParagraphNode(context.index,
                                       "<code>" + escape(code) + "</code>"));
    }
    return true;
  }

  bool parseImage(token_iterator &it) {
    if (it->kind != TokenKind::Exclamation) return false;
    ++it;

    if (it->kind != TokenKind::Bracket) return false;
    if (it->value != "[") return false;
    ++it;

    if (it->kind != TokenKind::Text) return false;
    auto alt = it->value;
    ++it;

    if (it->kind != TokenKind::Bracket) return false;
    if (it->value != "]") return false;
    ++it;

    if (it->kind != TokenKind::Bracket) return false;
    if (it->value != "(") return false;
    ++it;

    if (it->kind != TokenKind::Text) return false;
    auto url = it->value;
    ++it;

    if (it->kind != TokenKind::Bracket) return false;
    if (it->value != ")") return false;

    auto link = "<img src=\"" + url + "\" alt=\"" + alt + "\">";

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto paragraph = static_cast<ParagraphNode *>(prevSibling);
      paragraph->text += link;
    } else {
      context.append(new ParagraphNode(context.index, link));
    }

    return true;
  }

  bool parseLink(token_iterator &it) {
    if (it->kind != TokenKind::Bracket) return false;
    if (it->value != "[") return false;
    ++it;

    if (it->kind != TokenKind::Text) return false;
    auto text = it->value;
    ++it;

    if (it->kind != TokenKind::Bracket) return false;
    if (it->value != "]") return false;
    ++it;

    if (it->kind != TokenKind::Bracket) return false;
    if (it->value != "(") return false;
    ++it;

    if (it->kind != TokenKind::Text) return false;
    auto url = it->value;
    ++it;

    if (it->kind != TokenKind::Bracket) return false;
    if (it->value != ")") return false;

    auto link = "<a href=\"" + url + "\">" + text + "</a>";

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto paragraph = static_cast<ParagraphNode *>(prevSibling);
      paragraph->text += link;
    } else {
      context.append(new ParagraphNode(context.index, link));
    }

    return true;
  }

  bool parseEmphasis(token_iterator &it) {
    int c1 = 0;
    while (it->kind == TokenKind::Emphasis) {
      ++c1;
      ++it;
    }
    if (c1 == 0) return false;

    if (it->kind != TokenKind::Text) return false;
    auto text = std::string{};
    if (c1 == 1) text = "<em>" + it->value + "</em>";
    if (c1 == 2) text = "<strong>" + it->value + "</strong>";
    if (c1 >= 3) text = "<em><strong>" + it->value + "</strong></em>";
    ++it;

    for (int i = 0; i < c1; ++i, ++it)
      if (it->kind != TokenKind::Emphasis) return false;
    --it;

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::Paragraph) {
      auto prevPara = static_cast<ParagraphNode *>(prevSibling);
      if (prevPara->index == context.index) {
        prevPara->text += text;
      }
    } else {
      context.append(new ParagraphNode(context.index, text));
    }

    return true;
  }

  bool parseBlockQuote(token_iterator &it) {
    if (it->value != "> ") return false;
    context.indent = 0;

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::BlockQuote) {
      context.parent = prevSibling;
    } else {
      auto blockquote = new BlockQuoteNode();
      context.append(blockquote);
      context.parent = blockquote;
    }
    return true;
  }

  bool parseCodeBlock1(token_iterator &it) {
    if (context.indent < 4) return false;

    auto code = std::string{};
    while (it->kind != TokenKind::NewLine) {
      code += it->value;
      ++it;
    }
    --it;
    code = std::string(context.indent - 4, ' ') + code;
    context.index = 0;
    context.indent = 0;

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::CodeBlock) {
      auto codeblock = static_cast<CodeBlockNode *>(prevSibling);
      codeblock->text += "\n" + escape(code);
    } else {
      context.append(new CodeBlockNode(escape(code)));
    }

    return true;
  }

  bool parseCodeBlock2(token_iterator &it) {
    for (int i = 0; i < 3; ++i) {
      if (it->kind != TokenKind::BackQuote) return false;
      ++it;
    }
    if (it->kind != TokenKind::NewLine) return false;
    ++it;

    auto code = std::string{};
    while (it->kind != TokenKind::BackQuote) {
      if (it->kind == TokenKind::Eof) return false;
      if (it->kind == TokenKind::NewLine)
        code += "\n";
      else
        code += it->value;
      ++it;
    }
    if (!code.empty()) code.resize(code.size() - 1);

    for (int i = 0; i < 3; ++i) {
      if (it->kind != TokenKind::BackQuote) return false;
      ++it;
    }
    --it;

    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::CodeBlock) {
      auto codeblock = static_cast<CodeBlockNode *>(prevSibling);
      codeblock->text += "\n" + escape(code);
    } else {
      context.append(new CodeBlockNode(escape(code)));
    }
    return true;
  }

  bool parseUnorderedList(Node *root, token_iterator &it) {
    auto prevSibling = context.prevSibling();
    const bool isAfterUnorderedList =
        prevSibling && prevSibling->type == NodeType::UnorderedList;
    if (!isAfterUnorderedList && context.indent >= 4) {
      // should be codeblock
      return false;
    }

    if (it->kind != TokenKind::Prefix) return false;
    if (it->value[0] == '#') return false;
    context.index += it->value.size();
    context.indent = 0;

    // ul
    if (prevSibling && prevSibling->type == NodeType::UnorderedList) {
      UnorderedListNode *prevlist =
          static_cast<UnorderedListNode *>(prevSibling);

      int currDepth = context.index / 4;
      int prevDepth = prevlist->index / 4;

      if (currDepth > prevDepth) {
        auto parent = prevlist;
        for (int i = 1; i < currDepth; ++i) {
          auto nodes = parent->children;
          std::reverse(nodes.begin(), nodes.end());
          for (auto node : nodes) {
            if (node->type == NodeType::UnorderedList) {
              parent = static_cast<UnorderedListNode *>(node);
              break;
            }
          }
        }
        // add
        context.parent = parent;
        auto unorderedlist = new UnorderedListNode(context.index);
        context.append(unorderedlist);
        context.parent = unorderedlist;
      } else {
        // merge
        context.parent = prevlist;
      }
    } else {
      // add
      auto unorderedlist = new UnorderedListNode(context.index);
      context.append(unorderedlist);
      context.parent = unorderedlist;
    }

    // li
    auto item = new UnorderedListItemNode();
    context.append(item);
    context.parent = item;

    return true;
  }

  bool parseOrderedList(token_iterator &it) {
    if (it->kind != TokenKind::Prefix) return false;
    if (it->value[0] != '1') return false;
    context.index += it->value.size();
    context.indent = 0;
    ++it;

    while (it->kind == TokenKind::Indent) {
      context.index += it->value.size();
      context.indent += it->value.size();
      ++it;
    }
    --it;

    // ol
    auto prevSibling = context.prevSibling();
    if (prevSibling && prevSibling->type == NodeType::OrderedList) {
      context.parent = prevSibling;
    } else {
      auto orderedlist = new OrderedListNode(context.index);
      context.append(orderedlist);
      context.parent = orderedlist;
    }

    // li
    auto item = new OrderedListItemNode();
    context.append(item);
    context.parent = item;

    return true;
  }

 private:
  ParsingContext context;
};

}  // namespace m2h
