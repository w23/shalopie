#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace shmach {

class Ashembler {
public:
  Ashembler();

  bool register_opcode(const char *name, uint8_t opcode);
  
  struct program_t {
    std::string error_desc;
    std::unordered_map<std::string, std::vector<uint8_t> > sections;
  }; // struct program_t
  
  program_t parse(const char *program, uint32_t length) const;

private:
  std::unordered_map<std::string, uint8_t> opcodes_;
};
} // namespace shmach