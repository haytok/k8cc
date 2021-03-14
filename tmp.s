.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov rax, rbp
  sub rax, 16
  push rax
  push 3
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8
  mov rax, rbp
  sub rax, 8
  push rax
  push 5
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8
  mov rax, rbp
  sub rax, 16
  push rax
  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rsi
  pop rdi
  mov rax, rsp
  and rax, 15
  jnz .Lcall0
  mov rax, 0
  call foo
  jmp .Lend0
  .Lcall0:
  sub rsp, 8
  mov rax, 0
  call foo
  add rsp, 8
  .Lend0:
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
.global foo
foo:
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
  jmp .Lreturn.foo
.Lreturn.foo:
  mov rsp, rbp
  pop rbp
  ret
