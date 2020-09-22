;; Predicate definitions for Spim.


(define_predicate "symbolic_operand"
  (match_code "const,symbol_ref,label_ref")
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
