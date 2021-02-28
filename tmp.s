.intel_syntax noprefix
.global main
main:
  push 1
  pop rax
  push 2
  pop rax
  push 3
  pop rax
  ret
  pop rax
  ret
