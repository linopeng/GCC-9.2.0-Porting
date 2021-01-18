;; Constraint definitions for the Spim

;; Register constraints.
(define_register_constraint "z" "ZERO_REGS"
  )

(define_register_constraint "b" "BASE_REGS"
  )

(define_register_constraint "y" "CALLER_SAVED_REGS"
  )

(define_register_constraint "x" "CALLEE_SAVED_REGS"
  )
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                          Mips constraints                                                     ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;Register constraints


;; Floating-point constraints

(define_constraint "G"
  "Floating-point zero."
  (and (match_code "const_double")
       (match_test "op == CONST0_RTX (mode)")))

(define_constraint "S"
  "@internal
   A constant call address."
  (and (match_operand 0 "call_insn_operand")
       (match_test "CONSTANT_P (op)")))   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Integer constraints
(define_constraint "I"
  "An integer in the range 0 to 254."
  (and (match_code "const_int")
       (match_test "abs(ival) < 0xff")))

(define_constraint "J"
  "An integer in the range 0 to 31."
  (and (match_code "const_int")
       (match_test "abs(ival) < 0x01f")))

(define_constraint "K"
  "A shift operand, an integer in the range 0 to 65534."
  (and (match_code "const_int")
       (match_test "abs(ival) < 0xffff")))

(define_constraint "D"
  "For pattern symbolic_address_load symbolic_operand use."
  (match_code "symbol_ref"))
