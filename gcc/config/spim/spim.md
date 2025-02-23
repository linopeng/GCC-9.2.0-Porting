;; The dummy instruction has been supported to ensure that no empty
;; arrays are generated in source files of the generated compiler in the
;; initial levels..

(include "constraints.md")
(include "predicates.md")

(define_insn "dummy_pattern"
	[(reg:SI 0)]
	"1"
	"This stmnt should not be emitted!"
)

;; Functions in genemit.c read .md file for desired target and generates
;; gen_<insn-name> function  for each standard named  pattern defined in
;; .md file.  The function gen_<insn-name>  is in turn used  to generate
;; RTLs at the  time of transforming input program into  RTL. The source
;; files  cfgrtl.c,  cse.c,expmed.c which  contribute  in  cc1, use  the
;; function gen_jump to generate insn corresponding to jump instruction.
;; If this pattern is not  defined in machine description, the compiler,
;; fails in the linking phase because gen_jump is not defined.


;; For compiling _any_ program, jumps are a must.

(define_insn "jump"
	[(set (pc) (label_ref (match_operand 0 "" "")))]
	""
	{
		 return "j \\t%l0";
	}
)

(define_insn "indirect_jump"
	[(set (pc) (match_operand:SI 0 "register_operand" ""))]
	""
	"jr \\t%0"
)

(define_expand "epilogue"
	[(clobber (const_int 0))]
	""
	{
		 spim_epilogue();
		 DONE;
	}
)

(define_insn "IITB_return"
	[(return)
	 (use (reg:SI 31))]
	""
	"jr \\t\\$ra"
)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; This is the basic standard named pattern, which is matched at the time of 
;; GIMPLE to RTL conversion. Hence to add assignment operation in our machine
;; description, we have to define this standard pattern. We can have various
;; variants of this pattern depending upon which target instruction to emit,
;; and move patterns supported in the architecture. These variants can be
;; defined using define_insn patterns and constraints handling specific 
;; pattern.

;;movmisalignm is to be used for unaligned memory boundaries.

(define_expand "movsi"
	[(set (match_operand:SI 0 "nonimmediate_operand" "")
	      (match_operand:SI 1 "general_operand" "")
	)]
	""
	{
	  if(GET_CODE(operands[1])==CONST_INT && INTVAL(operands[1])==0)
	  {
	    emit_insn(gen_IITB_move_zero(operands[0],gen_rtx_REG(SImode,0)));
	    DONE;
	  }
	  else
	  if(GET_CODE(operands[0])==MEM && GET_CODE(operands[1])!=REG)
	  {
	    
	    if(can_create_pseudo_p())
	    {
		operands[1]=force_reg(SImode,operands[1]);
	    }
	  }
 	}
)

(define_insn "IITB_move_from_mem"
	[(set (match_operand:SI 0 "register_operand" "=r")
	      (match_operand:SI 1 "memory_operand" "m")
	)]
	""
	"lw \\t%0, %m1"
)

(define_insn "IITB_move_to_mem"
	[(set (match_operand:SI 0 "memory_operand" "=m")
	      (match_operand:SI 1 "register_operand" "r")
	)]
	""
	"sw \\t%1, %m0"
)


;;Load patterns

(define_insn "*load_word"
	[(set (match_operand:SI 0 "register_operand" "=r")
              (mem:SI (match_operand:SI 1 "address_operand" "p")))]
	""
	"lw \\t%0, %a1"
)

;;Constant loads

(define_insn "*constant_load"
	[(set (match_operand:SI 0 "register_operand" "=r")
	      (match_operand:SI 1 "const_int_operand" "i"))]
	""
	"li \\t%0, %c1"
;; This can also be defined using the actual lui instruction along with shift insn, 
;; but that will be used once shift operation is included in md file.
)

(define_insn "*symbolic_address_load" 
	[(set (match_operand:SI 0 "register_operand" "=r")
	      (match_operand:SI 1 "symbolic_operand" ""))]
	""
	"la \\t%0, %s1"
)

;; Here z is the constraint character defined in REG_CLASS_FROM_LETTER_P
;; The register $zero is used here. 
(define_insn "IITB_move_zero"
	[(set (match_operand:SI 0 "nonimmediate_operand" "=r,m")
	      (match_operand:SI 1 "zero_register_operand" "z,z")
	)]
	""
	"@
	move \\t%0,%1
	sw \\t%1, %m0"
)

;;store patterns

(define_insn "*store_word"
        [(set (mem:SI (match_operand:SI 0 "address_operand" "p"))
              (match_operand:SI 1 "register_operand" "r"))]
        ""
        "sw \\t%1, %a0"
)

(define_insn "*move_regs"
        [(set (match_operand:SI 0 "register_operand" "=r")
              (match_operand:SI 1 "register_operand" "r")
        )]
        ""
        "move \\t%0,%1"
)

(define_insn "addsi3"
	[(set (match_operand:SI 0 "register_operand" "=r,r")
              (plus:SI (match_operand:SI 1 "register_operand" "r,r")
                       (match_operand:SI 2 "nonmemory_operand" "r,K"))
         )]
        ""
        "@
         add \\t%0, %1, %2
         addi \\t%0, %1, %c2"

)

(define_expand "prologue"
	[(clobber (const_int 0))]
	""
	{
		spim_prologue();
		DONE;
	}
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Arithmatic and logical operations
;;===================================

(define_insn "abssi2"
	[(set (match_operand:SI 0 "register_operand" "=r")
	      (abs:SI (match_operand:SI 1 "register_operand" "r")))]
	""
	"abs \\t%0, %1"
)

(define_insn "andsi3"
        [(set (match_operand:SI 0 "register_operand" "=r,r")
              (and:SI (match_operand:SI 1 "register_operand" "r,r")
                      (match_operand:SI 2 "nonmemory_operand" "r,K"))
         )]
        ""
        "@
         and \\t%0, %1, %2
         andi \\t%0, %1, %c2"
)

(define_insn "divsi3"
        [(set (match_operand:SI 0 "register_operand" "=r,r")
              (div:SI (match_operand:SI 1 "register_operand" "r,r")
                       (match_operand:SI 2 "general_operand" "r,im"))
         )]
        ""
        "@
	div \\t%1, %2\\n\\tmflo \\t%0
	div \\t%0, %1, %2"
)

(define_insn "udivsi3"
        [(set (match_operand:SI 0 "register_operand" "=r,r")
              (udiv:SI (match_operand:SI 1 "register_operand" "r,r")
                       (match_operand:SI 2 "general_operand" "r,im"))
         )]
        ""
        "@
	divu \\t%1, %2\\n\\tmflo \\t%0
	divu \\t%0, %1, %2"
)

(define_insn "modsi3"
        [(set (match_operand:SI 0 "register_operand" "=r")
              (mod:SI (match_operand:SI 1 "register_operand" "r")
                       (match_operand:SI 2 "register_operand" "r"))
         )]
        ""
        "rem \\t%0, %1, %2"
)
                                                                                                    
(define_insn "umodsi3"
        [(set (match_operand:SI 0 "register_operand" "=r")
              (umod:SI (match_operand:SI 1 "register_operand" "r")
                       (match_operand:SI 2 "register_operand" "r"))
         )]
        ""
        "remu \\t%0, %1, %2"
)

(define_insn "mulsi3"
        [(set (match_operand:SI 0 "register_operand" "=r")
              (mult:SI (match_operand:SI 1 "register_operand" "r")
                       (match_operand:SI 2 "register_operand" "r"))
         )]
        ""
        "mul \\t%0, %1, %2"
)
(define_insn "umulsi3"
        [(set (match_operand:SI 0 "register_operand" "=r")
              (mult:SI (zero_extend:SI (match_operand:SI 1 "register_operand" "r"))
                       (zero_extend:SI (match_operand:SI 2 "general_operand" "rmi")))
         )]
        ""
        "mulou \\t%0, %1, %2"
)
(define_insn "negsi2"
        [(set (match_operand:SI 0 "register_operand" "=r")
              (neg:SI (match_operand:SI 1 "register_operand" "r"))
         )]
        ""
        "neg \\t%0, %1"
)

;;There is no standard pattern for NOR instruction, so currently omitting the pattern.

(define_insn "one_cmplsi2"
	[(set (match_operand:SI 0 "register_operand" "=r")
	      (not:SI (match_operand:SI 1 "register_operand" "r")))]
	""
	"not \\t%0, %1"
)

(define_insn "iorsi3"
        [(set (match_operand:SI 0 "register_operand" "=r,r")
              (ior:SI (match_operand:SI 1 "register_operand" "r,r")
                       (match_operand:SI 2 "nonmemory_operand" "r,K"))
         )]
        ""
        "@
	  or \\t%0, %1, %2
	  ori \\t%0, %1, %c2"
)

(define_insn "xorsi3"
        [(set (match_operand:SI 0 "register_operand" "=r,r")
              (xor:SI (match_operand:SI 1 "register_operand" "r,r")
                       (match_operand:SI 2 "nonmemory_operand" "r,K"))
         )]
        ""
        "@
          xor \\t%0, %1, %2
          xori \\t%0, %1, %c2"
)

(define_insn "subsi3"
        [(set (match_operand:SI 0 "register_operand" "=r")
              (minus:SI (match_operand:SI 1 "register_operand" "r")
                       (match_operand:SI 2 "register_operand" "r"))
         )]
        ""
        "sub \\t%0, %1, %2"
)

(define_insn "ashlsi3"
        [(set (match_operand:SI 0 "register_operand" "=r,r")
              (ashift:SI (match_operand:SI 1 "register_operand" "r,r")
                       (match_operand:SI 2 "nonmemory_operand" "r,J"))
         )]
        ""
        "@
	 sllv \\t%0, %1, %2
	 sll \\t%0, %1, %c2"
)

(define_insn "ashrsi3"
        [(set (match_operand:SI 0 "register_operand" "=r,r")
              (ashiftrt:SI (match_operand:SI 1 "register_operand" "r,r")
                       (match_operand:SI 2 "nonmemory_operand" "r,J"))
         )]
        ""
        "@
         srav \\t%0, %1, %2
         sra \\t%0, %1, %c2"
)

(define_insn "lshrsi3"
        [(set (match_operand:SI 0 "register_operand" "=r,r")
              (lshiftrt:SI (match_operand:SI 1 "register_operand" "r,r")
                       (match_operand:SI 2 "nonmemory_operand" "r,J"))
         )]
        ""
        "@
         srlv \\t%0, %1, %2
         srl \\t%0, %1, %c2"
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function calls
;;===============

(define_insn "call"
	[(call (match_operand:SI 0 "memory_operand" "m")
	       (match_operand:SI 1 "immediate_operand" "i"))
	 (clobber (reg:SI 31))
	]
	""
	"*
		return emit_asm_call(operands,0);
	"
)

(define_insn "call_value"
	[(set (match_operand:SI 0 "register_operand" "=r")
	      (call (match_operand:SI 1 "memory_operand" "m")
		    (match_operand:SI 2 "immediate_operand" "i")))
	 (clobber (reg:SI 31))
	]
	""
	"*
		return emit_asm_call(operands,1);
        "
)

(define_insn "call_value_internal"
  [(set (match_operand 0 "register_operand" "")
        (call (mem:SI (match_operand 1 "call_insn_operand" "S"))
              (match_operand 2 "" "")))
   (clobber (reg:SI 31))]
  ""
  "*
    return spim_output_jump (operands);
  "
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;Conditional code and branch instructions
;;========================================
(define_code_iterator any_cond [lt ltu eq ge geu gt gtu le leu ne])
;; (define_code_iterator fcond [lt ltu eq le leu ne])
;; (define_code_iterator swapped_fcond [ge geu gt gtu])

(define_expand "cmpsi"
	[(set (cc0) (compare (match_operand:SI 0 "register_operand" "")
                             (match_operand:SI 1 "nonmemory_operand" "")))]
	""
	{
		compare_op0=operands[0];
		compare_op1=operands[1];
		DONE;
	}
)

(define_expand "b<code>"
	[(set (pc) (if_then_else (any_cond:SI (match_dup 1) 
					       (match_dup 2)) 
				 (label_ref (match_operand 0 "" "")) 
				 (pc)))]
	""
	{
		operands[1]=compare_op0;
		operands[2]=compare_op1;
		if(immediate_operand(operands[2],SImode))
		{
			operands[2]=force_reg(SImode,operands[2]);
		}
		else if(immediate_operand(operands[2],SFmode))
		{
			operands[2]=force_reg(SFmode,operands[2]);
		}
		else if(immediate_operand(operands[2],DFmode))
                {
                        operands[2]=force_reg(DFmode,operands[2]);
                }

	}
)

(define_insn "*insn_b<code>"
        [(set (pc) (if_then_else (any_cond:SI (match_operand:SI 1 "register_operand" "r")
                                               (match_operand:SI 2 "register_operand" "r"))
                                 (label_ref (match_operand 0 "" ""))
                                 (pc)))]
        ""
	"*
		return conditional_insn(<CODE>,operands,0);
	"
)

(define_insn "cbranchsi4"
	[(set (pc)
		(if_then_else
			(match_operator 0 "comparison_operator"
			[(match_operand:SI 1 "register_operand" "")
			(match_operand:SI 2 "register_operand" "")])
		(label_ref (match_operand 3 "" ""))
		(pc)))]
	""
	"* 
		return conditional_insn(GET_CODE(operands[0]),operands,0); 
	"
)

;;isha added
(define_insn "nop"
  [(const_int 0)]
  ""
  "#nop"
)