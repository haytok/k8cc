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
  mov rax, rsp
  and rax, 15
  jnz .Lcall1
  mov rax, 0
  call add
  jmp .Lend1
  .Lcall1:
  sub rsp, 8
  mov rax, 0
  call add
  add rsp, 8
  .Lend1:
  push rax
  push 5
  pop rsi
  pop rdi
  mov rax, rsp
  and rax, 15
  jnz .Lcall2
  mov rax, 0
  call add
  jmp .Lend2
  .Lcall2:
  sub rsp, 8
  mov rax, 0
  call add
  add rsp, 8
  .Lend2:
  push rax
  pop rax
  jmp .Lreturn
.Lreturn:
  mov rsp, rbp
  pop rbp
  ret
