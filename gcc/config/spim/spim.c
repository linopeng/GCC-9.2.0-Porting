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
#include "diagnostic-core.h"
#include "stor-layout.h"
#include "varasm.h"
#include "calls.h"
#include "output.h"
#include "explow.h"	  					 					  
#include "expr.h"
#include "cfgrtl.h"
#include "builtins.h"
#include "regs.h"
#include "langhooks.h"
#include "cgraph.h"//
#include "gimple.h"//
#include "basic-block.h"//
#include "ggc.h"
#include "insn-attr.h"
#include "flags.h"
#include "except.h"//
#include "function.h"


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

void
print_operand(FILE *STREAM,rtx X,char CODE)
{
}


void
print_operand_address(FILE *STREAM,rtx X)
{
}

int
legitimate_address1(enum machine_mode MODE,rtx X)
{
	return 0;

}
/*Non-strict definition*/
int
legitimate_address2(enum machine_mode MODE,rtx X)
{
        
rtx op1,op2;
        if(CONSTANT_ADDRESS_P(X))
                return 1;
        
	if(GET_CODE(X)==REG && non_strict_base_reg(REGNO(X)))
                return 1;
        
	   
	return 0;
}

rtx
function_value ()
{
	//Return register is register 2 when value is of type SImode.
	return (gen_rtx_REG(SImode,2));
}

int
non_strict_base_reg(int regn)
{
	if(is_base_reg(regn))
		return IITB_YES;
	if(regn>=FIRST_PSEUDO_REGISTER)
		return IITB_YES;
	return IITB_NO;
}

int 
is_base_reg(int REGN)
{
	if(is_caller_saved_reg(REGN) 
			|| is_callee_saved_reg(REGN) 
			|| is_arg_reg(REGN) 
			|| is_return_val_reg(REGN)
			|| (REGN>=28 && REGN<=31))
		return IITB_YES;
	return IITB_NO;
}

int
is_caller_saved_reg(int REGN)
{
        if((REGN>=8 && REGN<=15)||(REGN==24)||(REGN==25))
                return IITB_YES;
        return IITB_NO;
}

int
is_callee_saved_reg(int REGN)
{
        if((REGN>=16 && REGN<=23))
                return IITB_YES;
        return IITB_NO;
}

int
is_arg_reg(int REGN)
{
        if( (REGN>=4 && REGN<=7))
                return IITB_YES;
        return IITB_NO;
}

int
is_return_val_reg(int REGN)
{
        if((REGN==2) || (REGN==3))
                return IITB_YES;
        return IITB_NO;
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


#undef TARGET_ASM_DESTRUCTOR
#define TARGET_ASM_DESTRUCTOR NULL
#undef TARGET_ASM_CONSTRUCTOR
#define TARGET_ASM_CONSTRUCTOR NULL

void
spim_asm_globalize_label(FILE *stream, const char *name)
{
	fprintf(stream,"\t.globl %s\n",name);
	return 0;
}


void
initialize_trampoline()
{
        return;
}


struct gcc_target targetm = TARGET_INITIALIZER;
