#define IN_TARGET_CODE 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h" //
#include "rtl.h"
#include "tree.h"
#include "tm_p.h"
#include "regs.h"
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
#include "expr.h"
#include "optabs.h"
#include "toplev.h"
#include "basic-block.h"//
#include "ggc.h"
#include "target.h"
#include "langhooks.h"
#include "cgraph.h"//
#include "gimple.h"//
#include "memmodel.h"

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

#undef TARGET_MERGE_DECL_ATTRIBUTES
#define TARGET_MERGE_DECL_ATTRIBUTES NULL
#undef TARGET_MERGE_TYPE_ATTRIBUTES
#define TARGET_MERGE_TYPE_ATTRIBUTES NULL

#undef TARGET_EXPAND_BUILTIN
#define TARGET_EXPAND_BUILTIN NULL

#undef TARGET_BUILD_BUILTIN_VA_LIST
#define TARGET_BUILD_BUILTIN_VA_LIST NULL

#undef TARGET_FN_ABI_VA_LIST
#define TARGET_FN_ABI_VA_LIST NULL

#undef TARGET_CANONICAL_VA_LIST_TYPE
#define TARGET_CANONICAL_VA_LIST_TYPE NULL

#undef TARGET_MUST_PASS_IN_STACK
#define TARGET_MUST_PASS_IN_STACK NULL

struct gcc_target targetm = TARGET_INITIALIZER;
