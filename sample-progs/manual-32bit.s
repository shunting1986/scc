.global main
main:
  pushl %ebp
  movl %esp, %ebp

  subl $12, %esp
  pushal

  leal -8(%ebp), %ebx
  pushl %ebx
  leal -4(%ebp), %ebx
  pushl %ebx
  pushl $fmt1
  call scanf
  addl $12, %esp

  movl -4(%ebp), %ebx
  movl -8(%ebp), %esi
  addl %ebx, %esi
  movl %esi, -12(%ebp)

  movl -12(%ebp), %ebx
  pushl %ebx
  pushl $fmt2
  call printf
  addl $8, %esp

  popal
  movl -12(%ebp), %eax
  movl %ebp, %esp
  popl %ebp
  ret

fmt1:
  .string "%d%d"
fmt2:
  .string "%d\n"
