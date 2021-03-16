.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 56
  mov rax, rbp
  sub rax, 8
  push rax
  mov rax, rbp
  sub rax, 56
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
  push 6
  pop rdi
  pop rax
  imul rdi, 8
  add rax, rdi
  push rax
  push 6
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8
  mov rax, rbp
  sub rax, 56
  push rax
  push 2
  pop rdi
  pop rax
  imul rdi, 24
  add rax, rdi
  push rax
  push 0
  pop rdi
  pop rax
  imul rdi, 8
  add rax, rdi
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
