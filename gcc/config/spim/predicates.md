;; Predicate definitions for Spim.


(define_predicate "symbolic_operand"
  (match_code "symbol_ref,label_ref")
{
	if(GET_CODE(op) == SYMBOL_REF || GET_CODE(op) == LABEL_REF)
        {
                return 1;
        }
        return 0;
})

(define_predicate "zero_register_operand"
  (match_code "reg")
{
        if(GET_CODE(op)!=REG)
                return 0;
        if(REGNO(op) == 0)
        {
                return 1;
        }
        return 0;
})

(define_predicate "constant_operand"
  (match_code "const_int")
{
	if(GET_CODE(op)!=CONST_INT)
		return 0;
	if(INTVAL(op) <=0xff)
	{
		return 1;
	}
	return 0;
})
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                Mips                                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define_predicate "move_operand"
  (match_operand 0 "general_operand")

{
        return 1;
})

(define_predicate "const_call_insn_operand"
  (match_code "const,symbol_ref,label_ref")
)

(define_predicate "call_insn_operand"
  (ior (match_operand 0 "const_call_insn_operand")
       (match_operand 0 "register_operand")))

