.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  mov rax, rsp
  and rax, 15
  jnz .Lcall1
  mov rax, 0
  call ret32
  jmp .Lend1
  .Lcall1:
  sub rsp, 8
  mov rax, 0
  call ret32
  add rsp, 8
  .Lend1:
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
.global ret32
ret32:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 32
  pop rax
  jmp .Lreturn.ret32
.Lreturn.ret32:
  mov rsp, rbp
  pop rbp
  ret
