#include <string.h>
#include "shmach.h"

#ifdef __MACH__
#include <libkern/OSAtomic.h>
#define ATOMIC32_INC(v) OSAtomicIncrement32((int32_t*)&(v))
#define ATOMIC32_DEC_AND_FETCH(v) OSAtomicDecrement32((int32_t*)&(v))
#else
#error define ATOMIC32_ ops
#endif

void shmach_object_retain(shmach_object_t *obj) {
  ATOMIC32_INC(obj->ref);
}

void shmach_object_release(shmach_object_t *obj) {
  if (ATOMIC32_DEC_AND_FETCH(obj->ref) == 0)
    obj->dtor(obj);
}

shmach_value_t *shmach_core_init(
  shmach_core_t *core,
  shmach_op_t *text,
  uint32_t arguments)
{
  core->text = text;
  core->pc = 0;
  core->sp = core->stack + SHMACH_MAX_STACK - arguments;
  return core->sp;
}

shmach_core_return_t shmach_core_run(shmach_core_t *core, uint32_t max_cycles) {
  shmach_core_return_t ret;
  ret.result = shmach_core_return_result_hang;
  while (max_cycles-- > 0) {
    switch (core->text[core->pc++]) {
    case SHMACH_OP_NOP:
      break;
    case SHMACH_OP_RET: { // expects [N {args x N} RETJMP]
        uint32_t args = core->sp[0].v.i;
        if (core->sp + args + 1 == core->stack + SHMACH_MAX_STACK) {
          ret.result = shmach_core_return_result_return;
          ret.count = args;
          ret.values = core->sp + 1;
          return ret;
        }
        core->pc = core->sp[args].v.i;
        for (; args > 0; --args)
          core->sp[args] = core->sp[args-1];
      }
      break;
    case SHMACH_OP_YIELD: // expects [N {args x N}
      ret.values = core->sp + 1;
      ret.count = core->sp->v.i;
      ret.result = shmach_core_return_result_yield;
      core->sp += ret.count + 1;
      return ret;
    case SHMACH_OP_JMP: // expects [ADDR]
      core->pc = (core->sp++)->v.i;
      break;
    case SHMACH_OP_JNZ: // expects [ADDR ZCHECK]
      if (core->sp[1].v.i != 0)
        core->pc = core->sp->v.i;
      core->sp += 2;
      break;
    case SHMACH_OP_CALL:
      // TODO
      break;
      
    case SHMACH_OP_DUP:
      core->sp--;
      core->sp[0] = core->sp[1];
      break;
    case SHMACH_OP_SWAP: {
        shmach_value_t tmp = core->sp[0];
        core->sp[0] = core->sp[1];
        core->sp[1] = tmp;
      }
      break;
    case SHMACH_OP_POP:
      core->sp++;
      break;

    case SHMACH_OP_ORETAIN:
      shmach_object_retain(core->sp->v.o);
      break;
    case SHMACH_OP_ORELEASE:
      shmach_object_release((core->sp++)->v.o);
      break;
    case SHMACH_OP_OCALL: // expects [OBJ FNUM Fargs...]
      // func should clear all parameters
      core->sp[0].v.o->functbl[core->sp[1].v.i](core->sp[0].v.o, core);
      break;

    case SHMACH_OP_LOAD_0:
      (--core->sp)->v.i = 0;
      break;
      
    case SHMACH_OP_LOAD_1:
      (--core->sp)->v.i = 1;
      break;
    
    case SHMACH_OP_ADD:
      core->sp[1].v.i += core->sp[0].v.i;
      ++core->sp;
      break;
      
    case SHMACH_OP_SUB:
      core->sp[1].v.i = core->sp[0].v.i - core->sp[1].v.i;
      ++core->sp;
      break;
      
    case SHMACH_OP_CMP_EQ:
      core->sp[1].v.i = core->sp[0].v.i == core->sp[1].v.i;
      ++core->sp;
      break;

    case SHMACH_OP_CMP_GT:
      core->sp[1].v.i = core->sp[0].v.i > core->sp[1].v.i;
      ++core->sp;
      break;
      
    default:
      break;
      //assert(false);
    }
  }
  return ret;
}
