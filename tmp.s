.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 208
  mov rax, rbp
  sub rax, 8
  push rax
  push 3
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8
  mov rax, rbp
  sub rax, 16
  push rax
  push 5
  push 6
  pop rdi
  pop rax
  imul rax, rdi
  push rax
  push 8
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8
  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  push 2
  pop rdi
  pop rax
  cqo
  idiv rdi
  push rax
  pop rdi
  pop rax
  add rax, rdi
  push rax
  pop rax
  jmp .Lreturn
.Lreturn:
  mov rsp, rbp
  pop rbp
  ret
