#define IN_TARGET_CODE 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tm.h" 
#include "rtl.h"
#include "tree.h"
#include "memmodel.h"
#include "df.h"
#include "tm_p.h"
#include "stringpool.h"
#include "attribs.h"
#include "regs.h"
#include "emit-rtl.h"
#include "hard-reg-set.h"
#include "real.h"
#include "insn-config.h"
#include "conditions.h"
#include "output.h"
#include "insn-codes.h"//
#include "insn-attr.h"
#include "flags.h"
#include "except.h"//
#include "function.h"
#include "recog.h"
#include "calls.h"
#include "expr.h"
#include "optabs.h"
#include "toplev.h"
#include "basic-block.h"//
#include "ggc.h"
#include "target.h"
#include "langhooks.h"
#include "cgraph.h"//
#include "gimple.h"
#include "builtins.h"
#include "explow.h"

#define IITB_YES 1
#define IITB_NO 0

/* This file should be included last.  */
#include "target-def.h"										 
		
#define return_addr_rtx gen_rtx_REG(SImode,31)			   

int i =0;
rtx compare_op0;
rtx compare_op1;

int
is_index_reg(int REGN)
{
	return IITB_NO;
}

int 
is_base_reg(int REGN)
{
	if(is_caller_saved_reg(REGN) 
			|| is_callee_saved_reg(REGN) 
			|| is_arg_reg(REGN) 
			|| is_return_val_reg(REGN)
			|| (REGN>=28 && REGN<=31)
			|| (REGN == 1)
	)
		return IITB_YES;
	return IITB_NO;
}

int
is_arg_reg(int REGN)
{
        if((REGN>=4 && REGN<=7))
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

int
non_strict_index_reg(int REGN)
{
	return IITB_NO;
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
is_general_reg(int REGN)
{
        if(REGN<FIRST_PSEUDO_REGISTER)
                return IITB_YES;
        return IITB_NO;
}

int 
modes_tieable_p(enum machine_mode MODE1, enum machine_mode MODE2)
{
        if((MODE1 == MODE2)
      		|| (GET_MODE_SIZE(MODE1) <= GET_MODE_SIZE(MODE2)
                  && GET_MODE_CLASS(MODE1) == GET_MODE_CLASS(MODE2)))
                return IITB_YES;
        return IITB_NO;
}

enum reg_class
regno_reg_class(int REGN)
{ 
        if(REGN==0)
                return ZERO_REGS;
        if(is_callee_saved_reg(REGN))
                return CALLEE_SAVED_REGS;
	if(is_caller_saved_reg(REGN))
                return CALLER_SAVED_REGS;
        if(is_base_reg(REGN))
                return BASE_REGS;
        if(is_general_reg(REGN))
                return GENERAL_REGS;
        return NO_REGS;
}

int
IITB_regno_ok_for_base_p (int REGN)
{
        if(is_base_reg(REGN) 
                || (REGN >= FIRST_PSEUDO_REGISTER 
                && is_base_reg(reg_renumber[REGN])))
                return IITB_YES;
        return IITB_NO;
}

int
regno_ok_for_index_p (int REGN)
{
        if(is_index_reg(REGN) 
                || (REGN >= FIRST_PSEUDO_REGISTER 
			&& is_index_reg(reg_renumber[REGN])))
                return IITB_YES;
        return IITB_NO;
}

int registers_to_be_saved(void)
{
        int i,num;
        for(i=0,num=0;i<FIRST_PSEUDO_REGISTER;i++)
        {
                if(df_regs_ever_live_p(i) && !call_used_regs[i] && !fixed_regs[i])
                        num++;
        }
        return num;
}

/* Functions related to activation records could be empty in this level, but we have
 * chosen to define activation records completely and hence, functions have been 
 * defined fully.*/
int
initial_elimination_offset(int from, int to)
{
	if(from == FRAME_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
	{
		//fprintf(stderr,"\n FRAME_POINTER_REGNUM to STACK_POINTER_REGNUM\n");
		return (get_frame_size());
	}
	else if(from == FRAME_POINTER_REGNUM && to == HARD_FRAME_POINTER_REGNUM)
	{
		return (2+registers_to_be_saved())*4;
	}
	else if(from == ARG_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
	{
		//fprintf(stderr,"ARG_POINTER_REGNUM to STACK_POINTER_REGNUM\n");
		return ((2+registers_to_be_saved())*4+get_frame_size());
	}
	else if(from == ARG_POINTER_REGNUM && to == HARD_FRAME_POINTER_REGNUM)
        {
				//fprintf(stderr,"ARG_POINTER_REGNUM to HARD_FRAME_POINTER_REGNUM\n");
                return ((2+4+registers_to_be_saved())*4+get_frame_size());
        }             
	else if(from == HARD_FRAME_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
	{
		//fprintf(stderr,"HARD_FRAME_POINTER_REGNUM to STACK_POINTER_REGNUM\n");
		return ((2+registers_to_be_saved())*4+get_frame_size());
	}
	else 
		printf("\nIt should not come here... Trying to eliminate non-eliminable register!\n");
		return 0;
}

int
constant_address_p (rtx X)
{
	fprintf(stderr,"--------This is constant_address_p--------\n");
	return (CONSTANT_P(X) && 
		GET_CODE(X)!=CONST_DOUBLE
		&& GET_CODE(X)!=CONST_VECTOR);
}

/*This function corresponds to the macro GO_IF_LEGITIMATE_ADDRESS. There are
 * two varients of this macro: one when the registers used may or may not be 
 * hard registers, and second when the registers must be LEGITIMATE HARD 
 * REGISTERS. This function checks if the address is legitimate or not.*/
int
legitimate_address1(enum machine_mode MODE,rtx X)
{
	fprintf(stderr,"--------This is legitimate_address1--------\n");
	rtx op1,op2;
	if(CONSTANT_ADDRESS_P(X))
		return 1;
	if(GET_CODE(X)==REG && is_base_reg(REGNO(X)))
		return 1;
	if(GET_CODE(X)==PLUS)
	{
		op1=XEXP(X,0);
		op2=XEXP(X,1);
		if(GET_CODE(op1)==REG && CONSTANT_ADDRESS_P(op2) && is_base_reg(REGNO(op1)))
			return 1;
		if(GET_CODE(op2)==REG && CONSTANT_ADDRESS_P(op1) && is_base_reg(REGNO(op2)))
			return 1;
	}
	return 0;

}
/*Non-strict definition*/
int
legitimate_address2(enum machine_mode MODE,rtx X)
{
	//fprintf(stderr,"--------This is legitimate_address2--------\n");
	rtx op1,op2;
        if(CONSTANT_ADDRESS_P(X))
                return 1;
		
        if(GET_CODE(X)==REG && non_strict_base_reg(REGNO(X)))
				//fprintf(stderr,"This is library check%d \n",i++);
                return 1;
        if(GET_CODE(X)==PLUS)
        {
                op1=XEXP(X,0);
                op2=XEXP(X,1);
                if(GET_CODE(op1)==REG && CONSTANT_ADDRESS_P(op2) && non_strict_base_reg(REGNO(op1)))
                        return 1;
                if(GET_CODE(op2)==REG && CONSTANT_ADDRESS_P(op1) && non_strict_base_reg(REGNO(op2)))
                        return 1;
        }
        return 0;
}

/*Here also, strict and non-strict varients are needed.*/
int 
reg_ok_for_base_p1(rtx x)
{
	if(is_base_reg(REGNO(x)))
		return IITB_YES;
	return IITB_NO;
}
int
reg_ok_for_base_p2(rtx x)
{
	if(non_strict_base_reg(REGNO(x)))
		return IITB_YES;
	return IITB_NO;
}

/*Here also, strict and non-strict varients are needed.*/
int 
reg_ok_for_index_p1(rtx x)
{
	if(is_index_reg(REGNO(x)))
		return IITB_YES;
	return IITB_NO;
}

int
reg_ok_for_index_p2(rtx x)
{
	if(non_strict_index_reg(REGNO(x)))
		return IITB_YES;
	return IITB_NO;
}

rtx 
legitimize_address(rtx X,rtx OLDX, enum machine_mode MODE)
{
	fprintf(stderr,"--------This is legitimize_address--------\n");
        rtx op1,op2,op;
	op=NULL;
        if(memory_address_p(MODE,X))
               return X;
	if(GET_CODE(X)==MEM && can_create_pseudo_p())
		op = force_reg(MODE,X);
	else if(GET_CODE(X)==PLUS && can_create_pseudo_p())
	{
		op1=XEXP(X,0);
		op2=XEXP(X,1);
		if(GET_CODE(op1)==REG && !CONSTANT_ADDRESS_P(op2))
		{
			op=force_reg(MODE,X);
		}
		else if(GET_CODE(op2)==REG && !CONSTANT_ADDRESS_P(op1))
                {
                        op=force_reg(MODE,X);
                }
	}
	if(op!=NULL && memory_address_p(MODE,op))
		return op;
	return X;
}

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
asm_output_skip(FILE  *STREAM,int NBYTES)
{
	fprintf(STREAM,"\t.skip %u\n", NBYTES);
}

void
print_operand(FILE *STREAM,rtx X,char CODE)
{
	rtx op;
	switch(CODE)
	{
		case 0:/* Generally, if there is no code after % character, then
			  it is considered as register operand. But, for safe case,
			  at this place also, I am taking care of all other operands.*/
			if(GET_CODE(X) == REG)
			{
				fprintf(STREAM,"%s",reg_names[XINT(X,0)]);
				break;
			}
			else if(GET_CODE(X) == MEM)
			{
			        op=XEXP(X,0);
		                PRINT_OPERAND_ADDRESS(STREAM,op);
			}
			else if(GET_CODE(X)==CONST_INT)
	                {
	                        fprintf(STREAM,"%d",XINT(X,0));
	                }
			else if(CONSTANT_ADDRESS_P(X))
			{
				PRINT_OPERAND_ADDRESS(STREAM,X);
			}
			else if((GET_CODE(X) == CONST_DOUBLE || GET_CODE(X) == CONST_INT))
			{
				if(GET_CODE(X)==LABEL_REF)
				{
					ASM_OUTPUT_LABELREF(STREAM,XSTR(X,0));
				}
				else
				{
					op=XEXP(X,0);
					PRINT_OPERAND(STREAM,op,0);
					printf("#Legitimate address");
				}
			}
	                else
	                {
				  printf("other case"
					  " in CODE = 0");
	                }
		break;
		case 'm':
			if(GET_CODE(X) != MEM)
			{
				printf("Invalid operand : Not a memory operand");
				return;
			}
			op=XEXP(X,0);
			PRINT_OPERAND_ADDRESS(STREAM,op);
			break;
		case 'D':
			if(GET_CODE(X)==SYMBOL_REF)
			{
				output_addr_const(STREAM,X);
			}
			else
			{
				printf("Other than symbol ref not allowed");
			}
			break;
		default:
			printf("In print operand default");
	}
}

void
print_operand_address(FILE *STREAM,rtx X)
{
	//fprintf(stderr,"--------This is print_operand_address--------\n");
	rtx op1,op2,temp;
	int num;
	switch(GET_CODE(X))
	{
		case SUBREG:
			/*As in case of register indirect mode, where address 
			  of operand is present in subreg.*/
			fprintf(STREAM,"0(%s)",reg_names[REGNO(XEXP(X,0))]);
			break;
		case REG:
			/*As in case of register indirect mode, address of operand
			  in memory is persent in register REGNO(X).*/
			fprintf(STREAM,"0(%s)",	reg_names[REGNO(X)]);
			break;
		case PLUS:
			/*The address can be in base displacement or base - index
			  form of addressing.*/
			op1 = XEXP(X,0);
			op2 = XEXP(X,1);
			if(GET_CODE(op1) == CONST_INT 
				&& (GET_CODE(op2) == REG 
					|| GET_CODE(op2) == SUBREG))
				/*base displacement*/
			{
				fprintf(STREAM,"%d(%s)", INTVAL(op1),
						((GET_CODE(op2)==REG)
						?reg_names[REGNO(op2)]
						:reg_names[REGNO(XEXP(op2,0))]));
			}
			else if (GET_CODE(op2) == CONST_INT 
					&& (GET_CODE(op1) == REG 
						|| GET_CODE(op1) == SUBREG))
				/*base displacement*/
			{
				fprintf(STREAM,"%d(%s)", INTVAL(op2),
						((GET_CODE(op1) == REG)
						?reg_names[REGNO(op1)]
						:reg_names[REGNO(XEXP(op1,0))]));
			}
			break;
		default:
			if(CONSTANT_ADDRESS_P(X))
			{
				output_addr_const(STREAM,X);
			}
			else
				fprintf(STREAM,"Coming in default part of" 
						" print_operand_address");
			break;
	}
}

void
asm_generate_internal_label(char *STRING,char *PREFIX,int NUM)
{
        sprintf(STRING,"%s%d", PREFIX,NUM);
}
void
asm_output_local(FILE *STREAM,char *NAME,int SIZE,int ROUNDED)
{
	fprintf(STREAM,"\t.reserve ");
        assemble_name (STREAM, NAME);
        fprintf (STREAM, ",%u,\"bss\"\n", SIZE);
}

void
asm_output_common(FILE *STREAM,char *NAME,int SIZE,int ROUNDED)
{
	fprintf(stderr,"--------This is asm_output_common--------\n");
	int i;
	fprintf(STREAM, "\t.data\n");
        assemble_name(STREAM,NAME);
        fprintf(STREAM, ":\t.word \t");
          for(i=0;i<SIZE/UNITS_PER_WORD;i++)
          {
                  fprintf(STREAM, "0 \t");
          }
          fprintf(STREAM,"\n");
}

int
asm_output_symbol_ref(FILE *stream, rtx sym)
{
        //fprintf(stream,"_");
        assemble_name(stream, XSTR((sym),0));
} 

void
function_profiler(FILE*asm_file,int labelno)
{
}

void
initialize_trampoline(void)
{
        return;
}

void
spim_prologue(void)
{
        int i,j;
	
        emit_move_insn(gen_rtx_MEM(SImode,plus_constant(SImode,stack_pointer_rtx,-4)),return_addr_rtx);	// sw      $ra, -4($sp)
        //emit_move_insn(gen_rtx_MEM(SImode,plus_constant(SImode,stack_pointer_rtx,-4)),stack_pointer_rtx);
        emit_move_insn(gen_rtx_MEM(SImode,plus_constant(SImode,stack_pointer_rtx,-8)),hard_frame_pointer_rtx); // sw      $fp, -8($sp)
        for(i=0,j=4;i<FIRST_PSEUDO_REGISTER;i++)
        {
                if(df_regs_ever_live_p(i) && !call_used_regs[i] && !fixed_regs[i])
                {
						fprintf(stderr,"----------------------------------------------------------------------------prologue\n");
                        emit_move_insn(gen_rtx_MEM(SImode,plus_constant(SImode,hard_frame_pointer_rtx,-4*j,0)), gen_rtx_REG(SImode,i)); //definde plus_constant arg 3 offset
						j++;																												// Similar sw  $r[0..31], -4*j($fp)
                }
        }
		// j = 4
		emit_insn(gen_rtx_SET(stack_pointer_rtx, plus_constant(SImode,stack_pointer_rtx,-((2+j)*4+get_frame_size()))));	// addi    $sp, $sp, -((3+j)*4+get_frame_size())
		emit_move_insn(hard_frame_pointer_rtx, plus_constant(SImode,stack_pointer_rtx,0)); // move    $fp,	$sp
}

void
spim_epilogue(void)
{
        int i,j;
       
        for(i=0,j=3;i<FIRST_PSEUDO_REGISTER;i++) /*Restore all the callee-registers from stack frame*/
        {
                if(df_regs_ever_live_p(i) && !call_used_regs[i] && !fixed_regs[i])
                {
						fprintf(stderr,"----------------------------------------------------------------------------epilogue\n");
                        emit_move_insn(gen_rtx_REG(SImode,i), gen_rtx_MEM(SImode,plus_constant(SImode,hard_frame_pointer_rtx,-4*j))); //lw reg Mem
                        j++;
                }
        }
	/*Restore stack pointer*/
	emit_insn(gen_rtx_SET(stack_pointer_rtx, plus_constant(SImode,stack_pointer_rtx,((3+j)*4+get_frame_size()))));
	//emit_move_insn(stack_pointer_rtx, gen_rtx_MEM(SImode,plus_constant(SImode,stack_pointer_rtx,-4)));
	/*Restore frame pointer*/
	emit_move_insn(hard_frame_pointer_rtx, gen_rtx_MEM(SImode,plus_constant(SImode,stack_pointer_rtx,-8)));
	/*Restore return address*/
	emit_move_insn(return_addr_rtx, gen_rtx_MEM(SImode,plus_constant(SImode,stack_pointer_rtx,-4)));
	/*Jump instruction*/
	emit_jump_insn(gen_IITB_return());
}

char*
emit_asm_call(rtx operands[],int type)
															 
{
	if(type == 0) /*call*/
	{
	        if(GET_CODE(XEXP(operands[0],0))==REG)
        	        return "jalr %0, \\$ra";
	        if(memory_address_p(SImode,XEXP(operands[0],0)))
        	        return "jal %0";
	}
	else /*call-value*/
	{
		if(GET_CODE(XEXP(operands[1],0))==REG)
                        return "jalr %1, \\$ra";
        if(memory_address_p(SImode,XEXP(operands[1],0)))
						return "jal \t%1";
	}				   
}

const char *
spim_output_jump (rtx *operands)
{
	// if add below statement will output error in final_scan_insn_1, at final.c:3055
	//if(memory_address_p(SImode,XEXP(operands[1],0)))
  			return "jal \t%1";
}

void
spim_init_cumulative_args (CUMULATIVE_ARGS *cum, tree fntype)
{
  memset (cum, 0, sizeof (*cum));
  cum->prototype = (fntype && prototype_p (fntype));
  cum->gp_reg_found = (cum->prototype && stdarg_p (fntype));
}

/* Initialize the GCC target structure. 
 * All macros taged as target_hook are defined here, instead of defining
 * in .h file. */
#undef TARGET_STRUCT_VALUE_RTX
#define TARGET_STRUCT_VALUE_RTX \
spim_struct_value_rtx

#undef TARGET_ASM_INTERNAL_LABEL 
#define TARGET_ASM_INTERNAL_LABEL \
spim_asm_internal_label

#undef TARGET_ASM_ALIGNED_SI_OP 
#define TARGET_ASM_ALIGNED_SI_OP "\t.word\t"

#undef TARGET_ASM_ALIGNED_DI_OP 
#define TARGET_ASM_ALIGNED_DI_OP "\t.word\t"

#undef TARGET_ASM_GLOBALIZE_LABEL
#define TARGET_ASM_GLOBALIZE_LABEL \
spim_asm_globalize_label

#undef TARGET_HARD_REGNO_NREGS
#define TARGET_HARD_REGNO_NREGS spim_hard_regno_nregs

#undef TARGET_HARD_REGNO_MODE_OK
#define TARGET_HARD_REGNO_MODE_OK spim_hard_regno_mode_ok

#undef TARGET_TRULY_NOOP_TRUNCATION
#define TARGET_TRULY_NOOP_TRUNCATION spim_truly_noop_truncation

#undef TARGET_LEGITIMATE_CONSTANT_P
#define TARGET_LEGITIMATE_CONSTANT_P spim_legitimate_constant_p

#undef TARGET_MODES_TIEABLE_P
#define TARGET_MODES_TIEABLE_P spim_modes_tieable_p

#undef TARGET_STARTING_FRAME_OFFSET
#define TARGET_STARTING_FRAME_OFFSET spim_starting_frame_offset

#undef TARGET_FUNCTION_ARG
#define TARGET_FUNCTION_ARG spim_function_arg
# undef TARGET_FUNCTION_ARG_ADVANCE
# define TARGET_FUNCTION_ARG_ADVANCE spim_function_arg_advance
#undef TARGET_FUNCTION_VALUE
#define TARGET_FUNCTION_VALUE spim_function_value
#undef TARGET_LIBCALL_VALUE
#define TARGET_LIBCALL_VALUE spim_libcall_value

#undef TARGET_MODE_DEPENDENT_ADDRESS_P
#define TARGET_MODE_DEPENDENT_ADDRESS_P spim_mode_dependent_address_p

#undef TARGET_ASM_FUNCTION_PROLOGUE
#define TARGET_ASM_FUNCTION_PROLOGUE spim_output_function_prologue
#undef TARGET_ASM_FUNCTION_END_PROLOGUE
#define TARGET_ASM_FUNCTION_END_PROLOGUE spim_output_function_end_prologue
#undef TARGET_ASM_FUNCTION_EPILOGUE
#define TARGET_ASM_FUNCTION_EPILOGUE spim_output_function_epilogue
#undef TARGET_ASM_FUNCTION_END_EPILOGUE
#define TARGET_ASM_FUNCTION_END_EPILOGUE spim_output_function_end_epilogue

// #undef TARGET_RETURN_POPS_ARGS
// #define TARGET_RETURN_POPS_ARGS 	spim_return_pops_args

rtx
spim_struct_value_rtx(tree fndecl, int incoming)
{
	return gen_rtx_REG(Pmode, 2);
}

void
spim_asm_internal_label(FILE *stream, const char *prefix, unsigned int labelno)
{
	fprintf(stream,"%s%d:\n",prefix,labelno);
	return;
}

void
spim_asm_globalize_label(FILE *stream, const char *name)
{
	fprintf(stream,"\t.globl %s\n",name);
	return;
}

/* Number of consecutive hard registers required to hold 
 * value of given mode, starting from register REGNO. */
static unsigned int
spim_hard_regno_nregs (unsigned int regno, machine_mode mode)
{
  	return (GET_MODE_SIZE (mode) + UNITS_PER_WORD - 1) / UNITS_PER_WORD; //All registers are word-sized.
}

/* The following macro returns 1 if a  value of mode MODE can be held in
 * register REGNO. If the mode is  double, it checks for register number
 * and allows only if register has  even number, else returns 0. This is
 * because for double  values, register with even number  is paired with
 * the succeeding  odd numbered  register. For  single integer  mode, it
 * allows all registers.*/

static bool
spim_hard_regno_mode_ok (unsigned int regno, machine_mode mode)
{ 
        if(GET_MODE_CLASS(mode) == MODE_INT)
		{
            if(GET_MODE_SIZE(mode) >= UNITS_PER_WORD)  /*Double Integer value.*/
            {
				 //fprintf(stderr,"MODE_INT size = %d\t",GET_MODE_SIZE(mode));
				 if(regno >= 0 && regno <= FIRST_PSEUDO_REGISTER && (regno % 2) == 0)
				 {
					  //fprintf(stderr,"regno >= 0 && regno <= FIRST_PSEUDO_REGISTER && (regno % 2) == 0 =============true\n");
                      return IITB_YES;
				 }else
				 {
					 //fprintf(stderr,"regno >= 0 && regno <= FIRST_PSEUDO_REGISTER && (regno % 2) == 0 =============False\n");
					 return IITB_NO;
				 }
				 
            }
            else
            {
                if(regno >= 0 && regno <= FIRST_PSEUDO_REGISTER)
                    return IITB_YES;
				fprintf(stderr,"GET_MODE_SIZE(mode) <= UNITS_PER_WORD \n");
                return IITB_NO;
            }
        }
		 if(GET_MODE_CLASS(mode) == MODE_FLOAT)
		 {
		 	//fprintf(stderr,"MODE_FLOAT size = %d\n",GET_MODE_SIZE(mode));
             if(GET_MODE_SIZE(mode) >= UNITS_PER_WORD)  /*Double Integer value.*/
             {
                  if(regno >= 0 && regno <= FIRST_PSEUDO_REGISTER&& (regno % 2) == 0)
                       return IITB_YES;
                 return IITB_NO;
             }
		 	else
             {
                 if(regno >= 0 && regno <= FIRST_PSEUDO_REGISTER)
                     return IITB_YES;
                 return IITB_NO;
             }
         }
	return IITB_NO;
}

/* Implement TARGET_TRULY_NOOP_TRUNCATION.  */
static bool
spim_truly_noop_truncation (poly_uint64 outprec, poly_uint64 inprec)
{
  return 1;
}

/* Implement TARGET_LEGITIMATE_CONSTANT_P.  */
static bool
spim_legitimate_constant_p (machine_mode mode ATTRIBUTE_UNUSED, rtx x)
{
  return (GET_CODE(x) == CONST_DOUBLE || GET_CODE(x) == CONST_INT);
}

/* This  macro defines  if object  of mode2  can be  moved in  object of
 * mode1. If the modes are same or they belong to same class (eg. int or
 * float) and mode2 has size less than mode1, then we allow the move.*/
static bool
spim_modes_tieable_p (machine_mode mode1, machine_mode mode2)
{
  	if((mode1 == mode2)
      		|| (GET_MODE_SIZE(mode1) <= GET_MODE_SIZE(mode2)
                  && GET_MODE_CLASS(mode1) == GET_MODE_CLASS(mode2)))
                return IITB_YES;
        return IITB_NO;
}

int
spim_starting_frame_offset (void)
{
	return 0;
}

/* Information about a single argument.  */
struct spim_arg_info {
  /* The number of words passed in registers, rounded up.  */
  unsigned int reg_words;
  /* For EABI, the offset of the first register from GP_ARG_FIRST or
     FP_ARG_FIRST.  For other ABIs, the offset of the first register from
     the start of the ABI's argument structure (see the CUMULATIVE_ARGS
     comment for details).
     The value is MAX_ARGS_IN_REGISTERS if the argument is passed entirely
     on the stack.  */
  unsigned int reg_offset;
  /* The number of words that must be passed on the stack, rounded up.  */
  unsigned int stack_words;
  /* The offset from the start of the stack overflow area of the argument's
     first stack word.  Only meaningful when STACK_WORDS is nonzero.  */
  unsigned int stack_offset;
};

static unsigned int
spim_function_arg_boundary (machine_mode mode, const_tree type)
{
  unsigned int alignment;

  alignment = type ? TYPE_ALIGN (type) : GET_MODE_ALIGNMENT (mode);
  if (alignment < PARM_BOUNDARY)
    alignment = PARM_BOUNDARY;
  if (alignment > STACK_BOUNDARY)
    alignment = STACK_BOUNDARY;
  return alignment;
}

static void
spim_get_arg_info (struct spim_arg_info *info, const CUMULATIVE_ARGS *cum,
		   machine_mode mode, const_tree type, bool named)
{
  bool doubleword_aligned_p;
  unsigned int num_bytes, num_words, max_regs;

  /* Work out the size of the argument.  */
  num_bytes = type ? int_size_in_bytes (type) : GET_MODE_SIZE (mode);
  num_words = (num_bytes + UNITS_PER_WORD - 1) / UNITS_PER_WORD;

  /* See whether the argument has doubleword alignment.  */
  doubleword_aligned_p = (spim_function_arg_boundary (mode, type)
			  > BITS_PER_WORD);

  /* Set REG_OFFSET to the register count we're interested in.
     The EABI allocates the floating-point registers separately,
     but the other ABIs allocate them like integer registers.  */
  //fprintf(stderr,"Before reg_offset = %d\n",info->reg_offset);
  info->reg_offset = cum->num_gprs;
  //fprintf(stderr,"After reg_offset = %d\n",info->reg_offset);

  /* Advance to an even register if the argument is doubleword-aligned.  */
  if (doubleword_aligned_p){
	fprintf(stderr,"reg_offset will change\n");
    info->reg_offset += info->reg_offset & 1;	  
  }


  /* Work out the offset of a stack argument.  */
  info->stack_offset = cum->stack_words;
  if (doubleword_aligned_p){
	fprintf(stderr,"stack_offset will change\n");
	info->stack_offset += info->stack_offset & 1;
  }


  max_regs = 4 - info->reg_offset;

  /* Partition the argument between registers and stack.  */
  info->reg_words = MIN (num_words, max_regs);
  info->stack_words = num_words - info->reg_words;
}

static unsigned int
spim_arg_regno (const struct spim_arg_info *info)
{
	return GP_ARG_FIRST + info->reg_offset; //init_info->reg_offset為0
}

/* 判断是否可以使用参数寄存器传递某个参数 */
static rtx
spim_function_arg (cumulative_args_t cum_v, machine_mode mode,
		   const_tree type, bool named)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);
  struct spim_arg_info info;

  /* We will be called with a mode of VOIDmode after the last argument
     has been seen.  Whatever we return will be passed to the call expander.
     If we need a MIPS16 fp_code, return a REG with the code stored as
     the mode.  */
  if (mode == VOIDmode)
    {
	return NULL;
    }

  spim_get_arg_info (&info, cum, mode, type, named);

  /* Return straight away if the whole argument is passed on the stack.  */
  if (info.reg_offset == 4){
	//fprintf(stderr,"info.reg_offset == 4 is true\n");
    return NULL;
  }
  return gen_rtx_REG (mode, spim_arg_regno (&info));
}

/* Implement TARGET_FUNCTION_ARG_ADVANCE.  */
/* 也可以在当前参数传递方式确定后更新该变量CUM，从而为确定下一个参数的传递方式提供依据。*/
static void
spim_function_arg_advance (cumulative_args_t cum_v, machine_mode mode,
			   const_tree type, bool named)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);
  struct spim_arg_info info;
  spim_get_arg_info (&info, cum, mode, type, named);

  cum->gp_reg_found = true;
  /* Advance the register count.  This has the effect of setting
     num_gprs to MAX_ARGS_IN_REGISTERS if a doubleword-aligned
     argument required us to skip the final GPR and pass the whole
     argument on the stack.  */
  cum->num_gprs = info.reg_offset + info.reg_words;

  /* Advance the stack word count.  */
  if (info.stack_words > 0)
    cum->stack_words = info.stack_offset + info.stack_words;

  cum->arg_number++;
}

static rtx
spim_function_value (const_tree valtype, const_tree fn_decl_or_type,
		     bool outgoing ATTRIBUTE_UNUSED)
{
	//Return register is register 2 when value is of type SImode.
	return gen_rtx_REG(TYPE_MODE(valtype), 2);
}
/* Create an RTX representing the place where a
   library function returns a value of mode MODE.  */
/* GP_RETURN can not be zero if it output error reload pass wrong. */
static rtx
spim_function_value_1 (const_tree valtype, const_tree fn_decl_or_type,
		       machine_mode mode)
{
  return gen_rtx_REG (mode, GP_RETURN);
}
static rtx
spim_libcall_value (machine_mode mode, const_rtx fun ATTRIBUTE_UNUSED)
{
  return spim_function_value_1 (NULL_TREE, NULL_TREE, mode);
}

static bool 
spim_mode_dependent_address_p (const_rtx addr,
				addr_space_t as ATTRIBUTE_UNUSED)
{
	fprintf(stderr,"-------this is spim_mode_dependent_address_p-------\n");
	    if (GET_CODE (addr) == PLUS
      && symbolic_operand (XEXP (addr, 1), VOIDmode))
    return true;

  return false;
};

static void
spim_output_function_prologue (FILE *file)
{
	fprintf(file,"\t#Function prologue\n");
}
static void
spim_output_function_end_prologue (FILE *file)
{
	fprintf(file,"\t#Function End Prologue\n");
}
static void
spim_output_function_epilogue (FILE *file)
{
	fprintf(file,"\t#Function Epilogue\n");
}
static void
spim_output_function_end_epilogue (FILE *file)
{
	fprintf(file,"\t#Function End Epilogue\n");
}

/* Note, we also need to provide a struct for machine functions
	the compilation breaks without it */
struct GTY(()) machine_function
{
	int reg_val;
};

struct gcc_target targetm = TARGET_INITIALIZER;

#include "gt-spim.h"