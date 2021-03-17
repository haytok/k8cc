.intel_syntax noprefix
.data
.text
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 7
  push 3
  push 3
  pop rdx
  pop rsi
  pop rdi
  mov rax, rsp
  and rax, 15
  jnz .Lcall0
  mov rax, 0
  call sub_char
  jmp .Lend0
  .Lcall0:
  sub rsp, 8
  mov rax, 0
  call sub_char
  add rsp, 8
  .Lend0:
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
.global sub_char
sub_char:
  push rbp
  mov rbp, rsp
  sub rsp, 8
  mov [rbp-3], dil
  mov [rbp-2], sil
  mov [rbp-1], dl
  mov rax, rbp
  sub rax, 3
  push rax
  pop rax
  movsx eax, byte ptr [rax]
  push rax
  mov rax, rbp
  sub rax, 2
  push rax
  pop rax
  movsx eax, byte ptr [rax]
  push rax
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  mov rax, rbp
  sub rax, 1
  push rax
  pop rax
  movsx eax, byte ptr [rax]
  push rax
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  pop rax
  jmp .Lreturn.sub_char
.Lreturn.sub_char:
  mov rsp, rbp
  pop rbp
  ret
