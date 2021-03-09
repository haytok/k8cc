.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 4
  push 3
  pop rsi
  pop rdi
  mov rax, rsp
  and rax, 15
  jnz .Lcall0
  mov rax, 0
  call add2
  jmp .Lend0
  .Lcall0:
  sub rsp, 8
  mov rax, 0
  call add2
  add rsp, 8
  .Lend0:
  push rax
  push 4
  push 3
  pop rsi
  pop rdi
  mov rax, rsp
  and rax, 15
  jnz .Lcall1
  mov rax, 0
  call sub2
  jmp .Lend1
  .Lcall1:
  sub rsp, 8
  mov rax, 0
  call sub2
  add rsp, 8
  .Lend1:
  push rax
  pop rsi
  pop rdi
  mov rax, rsp
  and rax, 15
  jnz .Lcall2
  mov rax, 0
  call add2
  jmp .Lend2
  .Lcall2:
  sub rsp, 8
  mov rax, 0
  call add2
  add rsp, 8
  .Lend2:
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
.global add2
add2:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov [rbp-16], rdi
  mov [rbp-8], rsi
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rdi
  pop rax
  add rax, rdi
  push rax
  pop rax
  jmp .Lreturn.add2
.Lreturn.add2:
  mov rsp, rbp
  pop rbp
  ret
.global sub2
sub2:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov [rbp-16], rdi
  mov [rbp-8], rsi
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  pop rax
  jmp .Lreturn.sub2
.Lreturn.sub2:
  mov rsp, rbp
  pop rbp
  ret
