#define FIRST_PSEUDO_REGISTER \
32
#define N_REG_CLASSES LIM_REG_CLASSES
#define CALL_USED_REGISTERS /* Caller-saved registers. This is irrelevant */ \
                            /* here because this version does not support */ \
			    /* function calls. We will make this more     */ \
			    /* precise later.                             */ \
{1,1,1,1, \
 1,1,1,1, \
 1,1,1,1, \
 1,1,1,1, \
 0,0,0,0, \
 0,0,0,0, \
 1,1,1,1, \
 1,1,1,1}

/* The  additional  classes   defined  here  are  CALLER_SAVED_REGISTER,
 * CALLEE_SAVED_REGISTER  and BASE_REGISTER.  Others are  pre-defined by
 * GCC. */
enum reg_class \
{\
	NO_REGS,\
	CALLER_SAVED_REGS,\
	CALLEE_SAVED_REGS,\
	BASE_REGS,\
	GENERAL_REGS,\
	ALL_REGS,\
	LIM_REG_CLASSES \
   };
#define UNITS_PER_WORD \
4
#define CUMULATIVE_ARGS \
int
#define MOVE_MAX  	/* Essentially size of the data bus */ \
4
#define STRICT_ALIGNMENT \
0
#define BYTES_BIG_ENDIAN \
0
#define FUNCTION_BOUNDARY \
32

#define BIGGEST_ALIGNMENT \
64
#define TRAMPOLINE_SIZE 32 
#define STACK_POINTER_REGNUM \
29
#define TARGET_CPU_CPP_BUILTINS()\
do                                            \
  {                                           \
      builtin_assert ("machine=spim");        \
      builtin_define_std ("spim");           \
      builtin_assert ("cpu=spim");	      \
      builtin_define ("__spim__");     					\
      builtin_define ("_spim");				\
  }                                           \
  while (0)
#define WORDS_BIG_ENDIAN \
0
#define STACK_BOUNDARY \
64
#define MAX_REGS_PER_ADDRESS \
1
#define Pmode SImode

#define FUNCTION_ARG_REGNO_P(r) /* Irrelevant in this level */ \
0
#define ARG_POINTER_REGNUM \
HARD_FRAME_POINTER_REGNUM 
#define FRAME_POINTER_REGNUM \
1
#define ELIMINABLE_REGS \
{{FRAME_POINTER_REGNUM,      STACK_POINTER_REGNUM}, \
 {FRAME_POINTER_REGNUM,      HARD_FRAME_POINTER_REGNUM}, \
 {ARG_POINTER_REGNUM,        STACK_POINTER_REGNUM}, \
 {HARD_FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM} \
}
#define PARM_BOUNDARY \
32
#define FUNCTION_MODE SImode
#define BASE_REG_CLASS \
BASE_REGS
/* Currently we assume any register can be used as base register. But in
 * later levels,  we will define  the registers acording  to appropriate
 * register class.*/
#define REGNO_OK_FOR_BASE_P(REGNO)\
1
#define REGNO_REG_CLASS(REGNO) \
regno_reg_class(REGNO)
#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, FNDECL, NAMED_ARGS)	\
{\
CUM = 0;\
}
#define BITS_BIG_ENDIAN \
0
#define ASM_GENERATE_INTERNAL_LABEL(STRING, PREFIX, NUM)                  \
        asm_generate_internal_label(STRING, PREFIX, NUM)
#define SLOW_BYTE_ACCESS 0
#define FIRST_PARM_OFFSET(FUN)\
0
#define ASM_OUTPUT_ALIGN(STREAM, POWER)                                   \
         asm_output_align(STREAM, POWER)
#define CASE_VECTOR_MODE SImode
#define ASM_APP_ON                                                        \
	"#APP"
#define ASM_APP_OFF                                                       \
	"#NO_APP"
#define FUNCTION_PROFILER(file,lab) \
function_profiler(file,lab)
/* Spim does not support indexed addressing mode. */
#define REGNO_OK_FOR_INDEX_P(REGNO)\
0
#define INDEX_REG_CLASS \
NO_REGS
/*Recomputes new offsets, after eliminating.*/
#define INITIAL_ELIMINATION_OFFSET(FROM, TO, VAR) \
(VAR) = initial_elimination_offset(FROM, TO)

#define FIXED_REGISTERS /* Even the registers which are not available */ \
			/* are marked fixed so that they are not      */ \
			/* considered for register allocation.        */ \
{1,1,0,0, \
 0,0,0,0, \
 0,0,0,0, \
 0,0,0,0, \
 0,0,0,0, \
 0,0,0,0, \
 0,0,1,1, \
 1,1,1,1}

#define REG_CLASS_CONTENTS  /* We have included un-available registers also */ \
                            /* because in this level, it is irrelevant      */ \
{0x00000000,0x0200ff00,0x00ff0000,0xf2fffffc,0xffffffff,0xffffffff}

#define REG_CLASS_NAMES \
{\
	"NO_REGS",\
	"CALLER_SAVED_REGS",\
	"CALLEE_SAVED_REGS",\
	"BASE_REGS",\
	"GEN_REGS",\
	"ALL_REGS"\
}

/* Required for for producing assembly output. */
#define REGISTER_NAMES \
{"$zero","$at","$v0","$v1", \
 "$a0","$a1","$a2","$a3", \
 "$t0","$t1","$t2","$t3", \
 "$t4","$t5","$t6","$t7", \
 "$s0","$s1","$s2","$s3", \
 "$s4","$s5","$s6","$s7", \
 "$t8","$t9","$k0","$k1", \
 "$gp","$sp","$fp","$ra", \
}

#define ASM_OUTPUT_COMMON(STREAM, NAME, SIZE, ROUNDED)                    \
         asm_output_common(STREAM, NAME, SIZE, ROUNDED)

#define ASM_OUTPUT_LOCAL(STREAM, NAME, SIZE, ROUNDED)                     \
         asm_output_local(STREAM, NAME, SIZE, ROUNDED)

#define ASM_OUTPUT_SKIP(STREAM, NBYTES)                                   \
         asm_output_skip(STREAM, NBYTES)

#undef TARGET_ASM_CONSTRUCTOR
#define TARGET_ASM_CONSTRUCTOR NULL

#undef TARGET_ASM_DESTRUCTOR
#define TARGET_ASM_DESTRUCTOR NULL

#define DEFAULT_SIGNED_CHAR \
0

#define TEXT_SECTION_ASM_OP	"\t.text"	/* instructions */
#define DATA_SECTION_ASM_OP	"\t.data"	/* large data */

#define PREFERRED_RELOAD_CLASS(X, CLASS) \
CLASS



