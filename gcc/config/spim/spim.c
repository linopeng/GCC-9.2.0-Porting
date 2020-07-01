#define IN_TARGET_CODE 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "rtl.h"
#include "tree.h"
#include "df.h"
#include "memmodel.h"
#include "tm_p.h"
#include "stringpool.h"
#include "attribs.h"
#include "emit-rtl.h"
#include "stor-layout.h"
#include "varasm.h"
#include "calls.h"
#include "output.h"
#include "explow.h"
#include "expr.h"
#include "cfgrtl.h"
#include "builtins.h"
#include "regs.h"

#define IITB_YES 1
#define IITB_NO 0

/* This file should be included last.  */
#include "target-def.h"

void
asm_output_align(FILE *STREAM, int POWER)
{
        int num,i;
        num=1;
        for(i=0;i<POWER;i++)
        {
                num=num*2;
        }
        fprintf (STREAM, "\t.align %d\n",POWER);
}

void
function_profiler(FILE*asm_file,int labelno)
{
}

int
regno_ok_for_index_p (int REGN)
{
        return IITB_NO;
}

void
asm_output_common(FILE *STREAM,char *NAME,int SIZE,int ROUNDED)
{
}

void
asm_output_skip(FILE  *STREAM,int NBYTES)
{
}

void
asm_output_local(FILE *STREAM,char *NAME,int SIZE,int ROUNDED)
{
}

enum reg_class
regno_reg_class(int REGN)
{ 
        return ALL_REGS;
}

void
asm_generate_internal_label(char *STRING,char *PREFIX,int NUM)
{
        sprintf(STRING,"%s%d", PREFIX,NUM);
}

int
IITB_regno_ok_for_base_p (int REGN)
{
        return IITB_YES;
}

/* Functions related to activation records could be empty in this level, but we have
 * chosen to define activation records completely and hence, functions have been 
 * defined fully.*/
int
initial_elimination_offset(int from, int to)
{
	if(from == FRAME_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
	{
		return (get_frame_size());
	}
	else if(from == FRAME_POINTER_REGNUM && to == HARD_FRAME_POINTER_REGNUM)
	{
		return -(3+registers_to_be_saved())*4;
	}
	else if(from == ARG_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
	{
		return ((3+registers_to_be_saved())*4+get_frame_size());
	}
	else if(from == HARD_FRAME_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
	{
		return ((3+registers_to_be_saved())*4+get_frame_size());
	}
	else 
		printf("\nIt should not come here... Trying to eliminate non-eliminable register!\n");
		return 0;
}

int registers_to_be_saved()
{
        int i,num;
        for(i=0,num=0;i<FIRST_PSEUDO_REGISTER;i++)
        {
                if(df_regs_ever_live_p(i) && !call_used_regs[i] && !fixed_regs[i])
                        num++;
        }
        return num;
}

void
spim_epilogue()
{
        emit_jump_insn(gen_IITB_return());
}

/* Initialize the GCC target structure. 
 * All macros taged as target_hook are defined here, instead of defining
 * in .h file. */
#undef TARGET_ASM_GLOBALIZE_LABEL
#define TARGET_ASM_GLOBALIZE_LABEL \
spim_asm_globalize_label

#undef TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P spim_legitimate_address_p

void
spim_asm_globalize_label(FILE *stream, const char *name)
{
	fprintf(stream,"\t.globl %s\n",name);
	return 0;
}

static bool
spim_legitimate_address_p (machine_mode mode, rtx x, bool strict_p)
{
	return x;
}




struct gcc_target targetm = TARGET_INITIALIZER;
