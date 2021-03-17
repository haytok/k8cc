.intel_syntax noprefix
.data
x:
  .zero 32
.text
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 32
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
