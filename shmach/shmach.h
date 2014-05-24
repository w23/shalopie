#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
  
enum {
  SHMACH_MAX_STACK = 4096,
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

typedef struct {
  shmach_op_t *text;
  uint32_t size;
} shmach_section_t;

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
  shmach_op_t *text;
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
  SHMACH_OP_RET,
  SHMACH_OP_YIELD,
  SHMACH_OP_JMP,
  SHMACH_OP_JNZ,
  SHMACH_OP_CALL,
  
// data manip
  SHMACH_OP_DUP,
  SHMACH_OP_SWAP,
  SHMACH_OP_POP,
  // GET (N .<-. v, PUT (N v .->. v)
  // DUPN (N {.N.})
  // SWAPN (N {.N.<->.N.}
  // ROTN ?
  
// object
  SHMACH_OP_ORETAIN,
  SHMACH_OP_ORELEASE,
  SHMACH_OP_OCALL,

// integer
  SHMACH_OP_LOAD_0,
  SHMACH_OP_LOAD_1,
  SHMACH_OP_ADD,
  SHMACH_OP_SUB,
  SHMACH_OP_CMP_EQ,
  SHMACH_OP_CMP_GT,
  
// float
  SHMACH_OP_FLOAD_1
  
// vec4f
};
  
#ifdef __cplusplus
}
#endif
