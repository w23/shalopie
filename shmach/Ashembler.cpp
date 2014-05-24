#include <cstdint>
#include <cstdlib>
#include <map>
#include "shmach.h"
#include "Ashembler.h"

static const char *get_next_token(const char **string, uint32_t *length) {
  for (;;) {
    while (*length != 0 && isspace(**string)) { (*length)--; ++(*string); }
    if (length == 0) return nullptr;
    if (**string == ';')
      while (*length != 0 && '\n' != **string) { (*length)--; ++(*string); }
    else
      break;
  }
  const char *begin = *string;
  while (*length != 0 && !isspace(**string)) { (*length)--; ++(*string); }
  if (*length == 0)
    return nullptr;
  return begin;
}

enum token_type_e {
  TokenSection,
  TokenCommand,
  TokenInteger,
  TokenFloat,
  TokenLabelDefinition,
  TokenLabelUsage,
  //TokenConstant,
  TokenInvalid
};

token_type_e get_token_type(const char *begin, const char *end) {
  if (begin >= end)
    return TokenInvalid;
  
  enum {
    alpha = 1,
    digit = 2,
    dot = 4,
    other = 8
  };
  int contains = 0;
  
  if (begin[0] == '.') {
    if (begin+1 == end)
      return TokenInvalid;
    return TokenSection;
  }
  
  if (begin[0] == ':') {
    if (begin+1 == end)
      return TokenInvalid;
    return TokenLabelUsage;
  }
  
  if (end[-1] == ':') {
    if (begin+1 == end)
      return TokenInvalid;
    return TokenLabelDefinition;
  }
  
  for (const char *c = begin; c != end; ++c)
    if (isalpha(*c)) contains |= alpha;
    else if (isdigit(*c)) contains |= digit;
    else if (*c == '.') contains |= dot;
    else contains |= other;

  if ((contains&alpha) != 0 && (contains&other) == 0)
    return TokenCommand;
  
  if (contains == digit)
    return TokenInteger;
    
  if (contains == (digit | dot))
    return TokenFloat;
    
  return TokenInvalid;
}

namespace shmach {

  Ashembler::Ashembler() {
    register_opcode("load", SHMACH_OP_LOAD);
    register_opcode("nop", SHMACH_OP_NOP);
    register_opcode("ret", SHMACH_OP_RET);
    register_opcode("yield", SHMACH_OP_YIELD);
    register_opcode("jmp", SHMACH_OP_JMP);
    register_opcode("jnz", SHMACH_OP_JNZ);
    register_opcode("call", SHMACH_OP_CALL);
    register_opcode("dup", SHMACH_OP_DUP);
    register_opcode("swap", SHMACH_OP_SWAP);
    register_opcode("pop", SHMACH_OP_POP);
    register_opcode("oreatin", SHMACH_OP_ORETAIN);
    register_opcode("orelease", SHMACH_OP_ORELEASE);
    register_opcode("ocall", SHMACH_OP_OCALL);
    register_opcode("load0", SHMACH_OP_LOAD0);
    register_opcode("iload1", SHMACH_OP_ILOAD1);
    register_opcode("iadd", SHMACH_OP_IADD);
    register_opcode("isub", SHMACH_OP_ISUB);
    register_opcode("icmpeq", SHMACH_OP_ICMP_EQ);
    register_opcode("icmpgt", SHMACH_OP_ICMP_GT);
    register_opcode("fload1", SHMACH_OP_FLOAD1);
    register_opcode("fadd", SHMACH_OP_FADD);
    register_opcode("fsin", SHMACH_OP_FSIN);
    register_opcode("fphase", SHMACH_OP_FPHASE);
    register_opcode("fph2rad", SHMACH_OP_FPH2RAD);
  }

  bool Ashembler::register_opcode(const char *name, uint8_t opcode) {
    std::string sname(name);
    if (opcodes_.find(sname) != opcodes_.end())
      return false;
    opcodes_[std::move(sname)] = opcode;
    return true;
  }
  
  Ashembler::program_t Ashembler::parse(const char *str, uint32_t length) const
  {
    program_t prog;
    std::string section;
    for (;;) { // for all
      std::string new_section;
      std::vector<uint8_t> text;
      std::map<std::string, uint32_t> labels_definition;
      std::vector<std::pair<std::string, uint32_t> > labels_usage;
      for (;;) { // read section
        const char *tok = get_next_token(&str, &length);
        if (tok == nullptr)
          break;
        std::string token(tok, str);
        
        printf("%s\n", token.c_str());
      
        switch(get_token_type(tok, str)) {
        case TokenSection:
          new_section = token.c_str() + 1;
          if (new_section.empty()) {
            prog.error_desc = "invalid empty section name";
            return prog;
          }
          break;

        case TokenCommand: {
            auto code = opcodes_.find(token);
            if (code == opcodes_.end()) {
              prog.error_desc =
                std::string("invalid operation \"") + token + "\"";
              return prog;
            }
            text.push_back(code->second);
          }
          break;

        case TokenInteger: {
          uint32_t num = static_cast<uint32_t>(strtoul(token.c_str(), NULL, 10));
          text.push_back(SHMACH_OP_LOAD);
          text.push_back(num & 0xff); num >>= 8;
          text.push_back(num & 0xff); num >>= 8;
          text.push_back(num & 0xff); num >>= 8;
          text.push_back(num & 0xff);
          }
          break;

        case TokenFloat: {
            shmach_value_t v;
            v.v.f = strtof(token.c_str(), NULL);
            text.push_back(SHMACH_OP_LOAD);
            text.push_back(v.v.i & 0xff); v.v.i >>= 8;
            text.push_back(v.v.i & 0xff); v.v.i >>= 8;
            text.push_back(v.v.i & 0xff); v.v.i >>= 8;
            text.push_back(v.v.i & 0xff);
          }
          break;

        case TokenLabelDefinition:
          token.erase(token.end()-1, token.end());
          if (labels_definition.find(token) != labels_definition.end()) {
            prog.error_desc =
              std::string("label \"") + token + "\" is already defined";
            return prog;
          }
          labels_definition[token] = static_cast<uint32_t>(text.size());
          break;
          
        case TokenLabelUsage:
          token.erase(token.begin(), token.begin()+1);
          text.push_back(SHMACH_OP_LOAD);
          labels_usage.push_back(std::pair<std::string, uint32_t>(token, text.size()));
          text.push_back(0xff);
          text.push_back(0xff);
          text.push_back(0xff);
          text.push_back(0xff);
          break;
        
        case TokenInvalid:
        default:
          prog.error_desc =
            std::string("invalid token \"") + token + "\"";
          return prog;
        }
        
        if (!new_section.empty())
          break;
      }
      
      // done section
      if (!section.empty()) {
        // resolve all labels
        for (const auto it: labels_usage) {
          const auto def = labels_definition.find(it.first);
          if (def == labels_definition.end()) {
            prog.error_desc =
              std::string("label \"") + it.first + "\" definition not found";
            return prog;
          }
 
          uint32_t addr = def->second;
          text[it.second + 0] = (addr & 0xff); addr >>= 8;
          text[it.second + 1] = (addr & 0xff); addr >>= 8;
          text[it.second + 2] = (addr & 0xff); addr >>= 8;
          text[it.second + 3] = (addr & 0xff);
          
          prog.sections[std::move(section)] = std::move(text);
        }
      }

      if (new_section.empty())
        break;
      
      section = std::move(new_section);
    }
    return prog;
  }

} // namespace shmach