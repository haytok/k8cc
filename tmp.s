.intel_syntax noprefix
.global main
main:
  push 1
  add rsp, 8
  push 2
  pop rax
  ret
  push 3
  pop rax
  ret
  ret
