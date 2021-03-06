.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 1
  pop rax
  cmp rax, 0
  je .Lelse0
  push 2
  pop rax
  jmp .Lreturn
  jmp .Lend0
  .Lelse0:
  push 3
  pop rax
  jmp .Lreturn
  .Lend0:
  push 4
  pop rax
  jmp .Lreturn
.Lreturn:
  mov rsp, rbp
  pop rbp
  ret
