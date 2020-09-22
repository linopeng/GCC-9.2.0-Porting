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

;; Integer constraints
(define_constraint "I"
  "An integer in the range 0 to 254."
  (and (match_code "const_int")
       (match_test "abs(ival) < 0xff")))

(define_constraint "J"
  "An integer in the range 1 to 32."
  (and (match_code "const_int")
       (match_test "abs(ival) < 0x01f")))

(define_constraint "K"
  "A shift operand, an integer in the range 0 to 31."
  (and (match_code "const_int")
       (match_test "abs(ival) < 0xffff")))
