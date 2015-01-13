
 ; where_was_i assembly helper code
;
; This code is called by the Address Error Trap ISR.  It will save the address
; of the instruction that caused an illegal address reference trap.  Place
; _errAddress in your Watch window to see what happend.
;
; GLOBAL: _errAddress
;
  .section *,bss,near
  .global __errAddress
__errAddress:   .space 4
  .text
  .global _where_was_i
_where_was_i:
  push.d w0
  sub w15,#12,w1           ; twelve bytes pushed since last trap!
  mov [w1++], w0
  mov w0, __errAddress
  mov [w1], w0
  mov.b WREG, __errAddress+2
  pop.d w0
  return
