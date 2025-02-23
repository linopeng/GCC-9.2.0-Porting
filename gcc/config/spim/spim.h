/* 	     Some of these macros are deprecated. They are marked by D 		 */
/* ------------------------------------------------------------------------------*
 * 			Data Sizes, Layout and Memory                            *
 * ------------------------------------------------------------------------------*/
#define BITS_BIG_ENDIAN \
  0

#define BYTES_BIG_ENDIAN \
  0

#define WORDS_BIG_ENDIAN \
  0

#define UNITS_PER_WORD \
  4

#define PARM_BOUNDARY \
  32

#define STACK_BOUNDARY \
  64

#define FUNCTION_BOUNDARY \
  32

#define BIGGEST_ALIGNMENT \
  64

#define STRICT_ALIGNMENT \
  0

#define MOVE_MAX /* Essentially size of the data bus */ \
  4

#define Pmode SImode

#define FUNCTION_MODE SImode

#define SLOW_BYTE_ACCESS 0

#define CASE_VECTOR_MODE SImode

/* ------------------------------------------------------------------------------*
 * 			Registers and their usage conventions 			 *
 * ------------------------------------------------------------------------------*/
/* We might need to add few internal registers like status register, HI,
 * LO registers,  FP registers. But  not at  this stage. Will  add those
 * only when  they are required.  Currently adding only  those registers
 * which are there  in SPIM. The registers for FPU  and co-proc0 will be
 * added, as per requirement.*/

#define FIRST_PSEUDO_REGISTER \
  32

#define FIXED_REGISTERS /* Even the registers which are not available */ \
                        /* are marked fixed so that they are not      */ \
                        /* considered for register allocation.        */ \
  {                                                                      \
    1, 1, 0, 0,                                                          \
    0, 0, 0, 0,                                                      \
    0, 0, 0, 0,                                                      \
    0, 0, 0, 0,                                                      \
    0, 0, 0, 0,                                                      \
    0, 0, 0, 0,                                                      \
    0, 0, 1, 1,                                                      \
    1, 1, 1, 1                                                       \
  }

#define CALL_USED_REGISTERS /* Caller-saved registers. This is irrelevant */ \
                            /* here because this version does not support */ \
                            /* function calls. We will make this more     */ \
                            /* precise later.                             */ \
  {                                                                          \
    1, 1, 1, 1,                                                              \
    1, 1, 1, 1,                                                          \
    1, 1, 1, 1,                                                          \
    1, 1, 1, 1,                                                          \
    0, 0, 0, 0,                                                          \
    0, 0, 0, 0,                                                          \
    1, 1, 1, 1,                                                          \
    1, 1, 1, 1                                                           \
  }

/* The  additional  classes   defined  here  are  CALLER_SAVED_REGISTER,
 * CALLEE_SAVED_REGISTER  and BASE_REGISTER.  Others are  pre-defined by
 * GCC. */
enum reg_class
{
  NO_REGS,
  ZERO_REGS,
  CALLER_SAVED_REGS,
  CALLEE_SAVED_REGS,
  BASE_REGS,
  GENERAL_REGS,
  ALL_REGS,
  LIM_REG_CLASSES
};

#define N_REG_CLASSES LIM_REG_CLASSES

#define REG_CLASS_NAMES      \
  {                          \
    "NO_REGS",               \
    "ZERO_REGS",         \
    "CALLER_SAVED_REGS", \
    "CALLEE_SAVED_REGS", \
    "BASE_REGS",         \
    "GEN_REGS",          \
    "ALL_REGS"           \
  }

#define REG_CLASS_CONTENTS /* We have included un-available registers also */          \
                           /* because in this level, it is irrelevant      */          \
  {                                                                                    \
    0x00000000, 0x00000001, 0x0200ff00, 0x00ff0000, 0xf2fffffc, 0xfffffffe, 0xffffffff \
  } /*some changes*/

#define REGNO_REG_CLASS(REGNO) \
  regno_reg_class(REGNO)

#define BASE_REG_CLASS \
  BASE_REGS

#define INDEX_REG_CLASS \
  NO_REGS

/* Currently we assume any register can be used as base register. But in
 * later levels,  we will define  the registers acording  to appropriate
 * register class.*/
#define REGNO_OK_FOR_BASE_P(REGNO) \
  IITB_regno_ok_for_base_p(REGNO)

/* Spim does not support indexed addressing mode. */
#define REGNO_OK_FOR_INDEX_P(REGNO) \
  0

#define PREFERRED_RELOAD_CLASS(X, CLASS) \
  CLASS

/* This is closely  related to the macro  HARD_REGNO_NREGS. It specifies
 * the maximum number  of consecutive registers of  class CLASS required
 * for  holding  a  value of  mode  MODE.  In  fact,  the value  of  the
 * macro CLASS_MAX_NREGS  (class, mode) should  be the maximum  value of
 * HARD_REGNO_NREGS  (regno, mode)  for all  regno values  in the  class
 * CLASS. */
#define CLASS_MAX_NREGS(CLASS, MODE) \
  ((GET_MODE_SIZE(MODE) + UNITS_PER_WORD - 1) / UNITS_PER_WORD) // All registers are word-sized.

/* ------------------------------------------------------------------------------*
 * 			Activation Record and Calling Conventions                *
 * ------------------------------------------------------------------------------*/

/* Stack layout is defined from the description from SPIM document. 
                      |                                 |
   direction of  |    |    CALLER'S ACTIVATION RECORD   |
     growth of   |    +---------------------------------+  <-- Previous SP
      stack      |    |   Parameters passed on stack    |
                 |    |  (Accessible from frame pointer.|
                \|/   |  Direction of growth is opposite|
                 V    |   to direction of stack growth) | <-- Current AP = HFP
                      +---------------------------------+ 
                      |        Return address           |          ,
                      +---------------------------------+         /|\
                      |      Dynamic chain address      |          |
                      |    (Pointer to caller's AR)     |          |
		      |       Caller's SPR, FPR	        |          |
                      +---------------------------------+  Register storage area.
                      |  Other caller saved registers   |          |
                      |.................................|          |
                      |  Callee saved registers being   |         \|/
                      |   used in the callee function   |          V
		      +---------------------------------+ <-- Current FP
                      |       Local variables           | 
                      |(Direction of growth of frame is |
                      | same as direction of growth of  |
                      |            stack)               |
                      +---------------------------------+
                      |                                 | <-- Current SP

  SP points to first empty slot.
  FP points to the location at which first local variable is stored.

  Since the number  of saved registers can only be  known after register
  allocation, the arguments frame is at a variable offset from the local
  variables frame. GCC tries to use  a single register for both argument
  and local variables frame. This requires defining a Hard Frame Pointer
  and  the  Frame  Pointer  is eliminated  by  computing  offsets  after
  register allocation.
*/
//ishkt changes
#define STACK_GROWS_DOWNWARD 1 // First sp goes down &second push element to stack then sp point to stack top element.

#define FRAME_GROWS_DOWNWARD 0

#define STACK_POINTER_OFFSET \
  0

#define FIRST_PARM_OFFSET(FNDECL) \
  0

#define STACK_POINTER_REGNUM \
  29

#define HARD_FRAME_POINTER_REGNUM \
  30

#define ARG_POINTER_REGNUM \
  HARD_FRAME_POINTER_REGNUM

/*Anyway this is dummy pointer, and is always eliminated to hard frame pointer.
  Use $at as frame_pointer to allocate dynamic var address and until gcc assign all reg frame_pointer=$at will eliminate by hard_frame_pointer*/
#define FRAME_POINTER_REGNUM \
  1

/* This macro was defined in level 0.0. But now that we have hard frame pointer, due
 * to variable sized fields between local variable frame and arguments, frame pointer
 * must be reduced to hard frame pointer register, which requires recalculation of
 * frame pointer offsets. Hence, we remove this macro.*/
/* #define INITIAL_FRAME_POINTER_OFFSET(DEPTH)\
DEPTH=initial_frame_pointer_offset (DEPTH) */

#define ELIMINABLE_REGS                                    \
  {                                                        \
    {FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM},          \
        {FRAME_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM}, \
        {ARG_POINTER_REGNUM, STACK_POINTER_REGNUM},        \
        {ARG_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM},   \
    {                                                      \
      HARD_FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM      \
    }                                                      \
  }

/*Recomputes new offsets, after eliminating.*/
#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET) \
  (OFFSET) = initial_elimination_offset((FROM), (TO))

/* Function pops none of its arguments, so it is caller's responsibility 
 * to pop off the parameters. */
/*
#define RETURN_POPS_ARGS(FUN, TYPE, SIZE) \
0
*/

/* Allocate stack space for arguments at the beginning of each function.  */
/* Caller儲存arg到stack lw,sw */
#define ACCUMULATE_OUTGOING_ARGS \
  1
/* Define this if it is the responsibility of the caller to
   allocate the area reserved for arguments passed in registers.
   If `ACCUMULATE_OUTGOING_ARGS' is also defined, the only effect
   of this macro is to determine whether the space is included in
   `crtl->outgoing_args_size'.  */
/* 導致callee那邊 lw進來的reg變動 */
#define OUTGOING_REG_PARM_STACK_SPACE(FNTYPE) 1
//#define PUSH_ARGS  (!ACCUMULATE_OUTGOING_ARGS)

/* Copy form mips.h Check N is or not arg register */
#define FUNCTION_ARG_REGNO_P(N) \
  ((IN_RANGE((N), GP_ARG_FIRST, GP_ARG_LAST) || ((N) % 2 == 0)) && !fixed_regs[N])

typedef struct spim_args
{
  /* Always true for varargs functions.  Otherwise true if at least
     one argument has been passed in an integer register.  */
  int gp_reg_found;
  /* The number of arguments seen so far.  */
  unsigned int arg_number;
  /* The number of integer registers used so far.  For all ABIs except
     EABI, this is the number of words that have been added to the
     argument structure, limited to MAX_ARGS_IN_REGISTERS.  */
  unsigned int num_gprs;
  /* The number of words passed on the stack.  */
  unsigned int stack_words;
  /* True if the function has a prototype.  */
  int prototype;
} CUMULATIVE_ARGS;

/* 用来初始化CUMULATIVE_ARGS类型的变量CUM，其主要目的是FUNCTION_ARG宏定义可以通过访问变量CUM，能够确定当前参数的传递类型，
   如果使用寄存器传递参数，则FUNCTION_ARG返回传递参数的寄存器RTX，
   如果使用堆栈传递，则FUNCTION_ARG返回NULL_RTX */
#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, FNDECL, NAMED_ARGS) \
  spim_init_cumulative_args(&CUM, FNTYPE)

/* Check return value store in register $v0 */
#define FUNCTION_VALUE_REGNO_P(REGN) \
  ((REGN) == 2)

/* ------------------------------------------------------------------------------*
 * 			Addressing Mode information                              *
 * ------------------------------------------------------------------------------*/

/* To validate use of labels as symbolic references or numeric addresses */
#define CONSTANT_ADDRESS_P(X) (CONSTANT_P(X) && GET_CODE(X) != CONST_DOUBLE && GET_CODE(X) != CONST_VECTOR)

/* Since we don't have base indexed mode, we do not need more than one 
 * register for any address. */
#define MAX_REGS_PER_ADDRESS \
  1

/* This macro is  used to validate an  address used in an  insn by using
 * the constraints specified  in the template in the  md (describing the
 * addressing) mode and the constraints on the register classes. This is
 * implemented by transferring control  to appropriate code fragments in
 * source of generated compiler.*/
#ifdef REG_OK_STRICT
#define GO_IF_LEGITIMATE_ADDRESS(mode, x, label) \
  {                                              \
    if (legitimate_address1(mode, x))            \
      goto label;                                \
  }
#else
#define GO_IF_LEGITIMATE_ADDRESS(mode, x, label)          \
  {                                                       \
    if (legitimate_address2(mode, x))                     \
      goto label;                                         \
    fprintf(stderr, "-----------------------\n");         \
    fprintf(stderr, "mode : %s \n", GET_MODE_NAME(mode)); \
  }
#endif
#ifdef REG_OK_STRICT
#define REG_OK_FOR_BASE_P(x) \
  reg_ok_for_base_p1(x)
#else
#define REG_OK_FOR_BASE_P(x) \
  reg_ok_for_base_p2(x)
#endif

#ifdef REG_OK_STRICT
#define REG_OK_FOR_INDEX_P(x) \
  reg_ok_for_index_p1(x)
#else
#define REG_OK_FOR_INDEX_P(x) \
  reg_ok_for_index_p2(x)
#endif

/* ------------------------------------------------------------------------------*
 * 			Assembly Output Format                                   *
 * ------------------------------------------------------------------------------*/

#define ASM_OUTPUT_ALIGN(STREAM, POWER) \
  asm_output_align(STREAM, POWER)

#define ASM_OUTPUT_SKIP(STREAM, NBYTES) \
  asm_output_skip(STREAM, NBYTES)

#define PRINT_OPERAND(STREAM, X, CODE) \
  print_operand(STREAM, X, CODE)

#define PRINT_OPERAND_ADDRESS(STREAM, X) \
  print_operand_address(STREAM, X)

#define ASM_GENERATE_INTERNAL_LABEL(STRING, PREFIX, NUM) \
  asm_generate_internal_label(STRING, PREFIX, NUM)

#define ASM_OUTPUT_LOCAL(STREAM, NAME, SIZE, ROUNDED) \
  asm_output_local(STREAM, NAME, SIZE, ROUNDED)

#define ASM_OUTPUT_COMMON(STREAM, NAME, SIZE, ROUNDED) \
  asm_output_common(STREAM, NAME, SIZE, ROUNDED)

#define ASM_OUTPUT_SYMBOL_REF(stream, sym) \
  asm_output_symbol_ref(stream, sym)

#define FUNCTION_PROFILER(file, lab) \
  function_profiler(file, lab)

/* This is how to output a string.  
#undef ASM_OUTPUT_ASCII
#define ASM_OUTPUT_ASCII(STREAM, STRING, LEN)				\
  spim_output_ascii (STREAM, STRING, LEN, "\t.asciiz\t")*/

#define ASM_APP_ON \
  "#APP"
#define ASM_APP_OFF \
  "#NO_APP"

/* Required for for producing assembly output. */
#define REGISTER_NAMES              \
  {                                 \
    "$zero", "$at", "$v0", "$v1",   \
        "$a0", "$a1", "$a2", "$a3", \
        "$t0", "$t1", "$t2", "$t3", \
        "$t4", "$t5", "$t6", "$t7", \
        "$s0", "$s1", "$s2", "$s3", \
        "$s4", "$s5", "$s6", "$s7", \
        "$t8", "$t9", "$k0", "$k1", \
        "$gp", "$sp", "$fp", "$ra", \
  }

#define TEXT_SECTION_ASM_OP \
  "\t.text"

#define DATA_SECTION_ASM_OP \
  "\t.data 0x10000000"

#define ASM_OUTPUT_LABELREF(stream, name) \
  fprintf(stream, "%s", name);

/* ------------------------------------------------------------------------------*
 * 			Misc 
 * ------------------------------------------------------------------------------*/
#define DEFAULT_SIGNED_CHAR \
  0

/* In  this level,  since  we  are not  supporting  any addressing  mode
 * involving  offsets (in  fact  we are  not  supporting ANY  addressing
 * mode), no  constant value is  defined. Later  on we will  define this
 * macro depending  upon constant  values permitted in  addressing modes
 * supported.*/

#define TARGET_CPU_CPP_BUILTINS() \
  do                              \
  {                               \
    builtin_define_std("spim");   \
    builtin_assert("cpu=spim");   \
  } while (0)

/* This macro has been defined to eliminate call to __main function from `main'. */
#define HAS_INIT_SECTION

#define TRAMPOLINE_SIZE 32

/************************************** Reference by mips.h **************************************/
/************************************** Used for function call macro **************************************/
#define GP_RETURN (GP_REG_FIRST + 2)
#define GP_REG_FIRST 0
#define GP_REG_LAST 31
#define GP_REG_NUM (GP_REG_LAST - GP_REG_FIRST + 1)
#define GP_ARG_FIRST (GP_REG_FIRST + 4)
#define GP_ARG_LAST 7