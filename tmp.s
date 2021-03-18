.intel_syntax noprefix
.data
.LC0:
  .byte 97
  .byte 98
  .byte 99
  .byte 0
.text
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 4
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
