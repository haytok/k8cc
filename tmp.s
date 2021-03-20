.intel_syntax noprefix
.data
.text
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 2
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
