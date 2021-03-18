.intel_syntax noprefix
.data
.LC0:
  .byte 108
  .byte 0
.text
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push offset .LC0
  push 0
  pop rdi
  pop rax
  imul rdi, 1
  add rax, rdi
  push rax
  pop rax
  movsx eax, byte ptr [rax]
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
