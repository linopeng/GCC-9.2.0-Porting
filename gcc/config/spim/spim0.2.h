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
      builtin_define_std ("spim");           \
      builtin_assert ("cpu=spim");	      \
  }                                           \
  while (0)
#define WORDS_BIG_ENDIAN \
0
#define STACK_BOUNDARY \
64
#define MAX_REGS_PER_ADDRESS \
1

