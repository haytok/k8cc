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
  sub rax, 8
  push rax
  push 1
  pop rdi
  pop rax
  imul rdi, 8
  sub rax, rdi
  push rax
  push 7
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
