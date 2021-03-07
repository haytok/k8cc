.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 3
  push 5
  pop rsi
  pop rdi
  call add
  push rax
  push 5
  pop rsi
  pop rdi
  call add
  push rax
  pop rax
  jmp .Lreturn
.Lreturn:
  mov rsp, rbp
  pop rbp
  ret
