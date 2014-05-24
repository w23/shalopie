#include <stdio.h>
#include "shmach.h"

static int test_basic_ret() {
  static shmach_op_t text[] = {
    SHMACH_OP_LOAD_0,
    SHMACH_OP_RET
  };
  shmach_core_t core;
  shmach_core_init(&core, text, 0);
  shmach_core_return_t ret = shmach_core_run(&core, 128);
  if (ret.result != shmach_core_return_result_return) return 1;
  if (ret.count != 0) return 2;
  return 0;
}

static int test_basic_yield() {
  static shmach_op_t text[] = {
    SHMACH_OP_LOAD_1, // 1 31337
    SHMACH_OP_YIELD,  //
    SHMACH_OP_LOAD_0, // 0
    SHMACH_OP_RET
  };
  shmach_core_t core;
  shmach_value_t *val = shmach_core_init(&core, text, 1);
  val[0].v.i = 31337;
  shmach_core_return_t ret = shmach_core_run(&core, 128);
  if (ret.result != shmach_core_return_result_yield) return 1;
  if (ret.count != 1) return 2;
  if (ret.values[0].v.i != 31337) return 3;
  ret = shmach_core_run(&core, 128);
  if (ret.result != shmach_core_return_result_return) return 4;
  if (ret.count != 0) return 5;
  return 0;
}

static int test_basic_data_manip() {
  static shmach_op_t text[] = {
    SHMACH_OP_DUP,    // 4 4 1 T
    SHMACH_OP_LOAD_1, // 1 4 4 1 T
    SHMACH_OP_ADD,    // 5 4 1 T
    SHMACH_OP_SUB,    // 1 1 T
    SHMACH_OP_LOAD_0, // 0 1 1 T
    SHMACH_OP_ADD,    // 1 1 T
    SHMACH_OP_DUP,    // 1 1 1 T
    SHMACH_OP_ADD,    // 2 1 T
    SHMACH_OP_RET
  };
  shmach_core_t core;
  shmach_object_t tmp;
  shmach_value_t *val = shmach_core_init(&core, text, 3);
  val[0].v.i = 4;
  val[1].v.i = 1;
  val[2].v.o = &tmp;
  shmach_core_return_t ret = shmach_core_run(&core, 128);
  if (ret.result != shmach_core_return_result_return) return 1;
  if (ret.count != 2) return 2;
  if (ret.values[0].v.i != 1) return 3;
  if (ret.values[1].v.o != &tmp) return 4;
  return 0;
}


static int test_object_call_val;
static void test_object_call_func(shmach_object_t *obj, shmach_core_t *core, uint32_t index)
{
  if (index == 1)
    test_object_call_val = core->sp[0].v.i;
  core->sp += 1;
}

static int test_object_call() {
  static shmach_op_t text[] = {
    SHMACH_OP_DUP,    // o o
    SHMACH_OP_LOAD_1, // 1 o o
    SHMACH_OP_DUP,    // 1 1 o o
    SHMACH_OP_ADD,    // 2 o o
    SHMACH_OP_SWAP,   // o 2 o
    SHMACH_OP_LOAD_1, // 1 o 2 o
    SHMACH_OP_SWAP,   // o 1 2 o
    SHMACH_OP_OCALL,  // o
    SHMACH_OP_POP,
    SHMACH_OP_LOAD_0,
    SHMACH_OP_RET
  };
  test_object_call_val = 0;
  shmach_core_t core;
  shmach_object_t tmp;
  tmp.func = test_object_call_func;
  shmach_value_t *val = shmach_core_init(&core, text, 1);
  val[0].v.o = &tmp;
  shmach_core_return_t ret = shmach_core_run(&core, 128);
  if (ret.result != shmach_core_return_result_return) return 1;
  if (ret.count != 0) return 2;
  if (test_object_call_val != 2) return 3;
  return 0;
}

// \todo:
// - jmp
// - flow ctl
// - objects

int main(int argc, char *argv[]) {
  int ret = 0;
#define RUN_TEST(name) { \
  int r = test_##name(); \
  if (r != 0) \
    printf("test " #name ": FAIL (%d)\n", r); \
  else \
    printf("test " #name ": OK\n"); \
  ret += r; \
}
  RUN_TEST(basic_ret)
  RUN_TEST(basic_yield)
  RUN_TEST(basic_data_manip)
  RUN_TEST(object_call)
  return ret;
}
