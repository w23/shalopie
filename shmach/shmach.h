#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
  
enum {
  SHMACH_MAX_STACK = 128,
  SHMACH_MAX_CORES = 16
};
  
struct shmach_core_t_;
struct shmach_object_t_;

// \return stack items to pop
typedef uint32_t (*shmach_object_method_f)(struct shmach_object_t_ *obj, struct shmach_core_t_ *core);

struct shmach_object_t_ {
  uint32_t ref;
  void (*dtor)(struct shmach_object_t_ *obj);
  void (*func)(struct shmach_object_t_ *obj, struct shmach_core_t_ *core, uint32_t index);
};
typedef struct shmach_object_t_ shmach_object_t;

void shmach_object_retain(shmach_object_t *obj);
void shmach_object_release(shmach_object_t *obj);
  
typedef struct {
  union {
    uint32_t i;
    float f;
    shmach_object_t *o;
  } v;
} shmach_value_t;
  
typedef uint8_t shmach_op_t;

/*typedef struct {
  shmach_op_t *text;
  uint32_t size;
} shmach_section_t;*/
typedef const shmach_op_t* shmach_section_t;

typedef struct {
  enum {
    shmach_core_return_result_yield,
    shmach_core_return_result_return,
    shmach_core_return_result_hang
  } result;
  shmach_value_t *values;
  uint32_t count;
} shmach_core_return_t;

struct shmach_core_t_ {
  const shmach_op_t *text;
  shmach_value_t stack[SHMACH_MAX_STACK];
  shmach_value_t *sp;
  uint32_t pc;
};
typedef struct shmach_core_t_ shmach_core_t;
  
shmach_value_t *shmach_core_init(
  shmach_core_t *core,
  shmach_op_t *text,
  uint32_t arguments);
shmach_value_t *shmach_core_reset(shmach_core_t *core, uint32_t argc);
shmach_core_return_t shmach_core_run(shmach_core_t *core, uint32_t max_cycles);

enum {
// control
  SHMACH_OP_NOP,
  SHMACH_OP_DUMP,
  SHMACH_OP_RET,
  SHMACH_OP_YIELD,
  SHMACH_OP_JMP,
  SHMACH_OP_JNZ,
  SHMACH_OP_CALL,
  SHMACH_OP_LOOPDECNZ,
  
// data manip
  SHMACH_OP_DUP,
  SHMACH_OP_SWAP,
  SHMACH_OP_POP,
  SHMACH_OP_GET,
  SHMACH_OP_PUT,
  SHMACH_OP_DUPN, // duplicate one from depth
  SHMACH_OP_NDUP, // duplicate N from top
  // GET (N .<-. v, PUT (N v .->. v)
  // DUPN (N {.N.})
  // SWAPN (N {.N.<->.N.}
  // ROTN ?
  
// object
  SHMACH_OP_ORETAIN,
  SHMACH_OP_ORELEASE,
  SHMACH_OP_OCALL,

// integer
  SHMACH_OP_LOAD,
  SHMACH_OP_LOAD0,
  SHMACH_OP_ILOAD1,
  SHMACH_OP_IADD,
  SHMACH_OP_ISUB,
  SHMACH_OP_ICMP_EQ,
  SHMACH_OP_ICMP_GT,
  SHMACH_OP_IDEC,
  
// float
  SHMACH_OP_FLOAD1,
  SHMACH_OP_FADD,
  SHMACH_OP_FSIN,
  SHMACH_OP_FPHASE,
  SHMACH_OP_FPH2RAD,
  SHMACH_OP_FMUL,
  SHMACH_OP_FCMP_GT,
  
// vec4f
};
  
#ifdef __cplusplus
}
#endif
