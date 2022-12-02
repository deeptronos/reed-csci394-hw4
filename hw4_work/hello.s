	.data
L_5:
	.asciiz "Hello, world!"
L_0:
	.asciiz "\n"
L_1:
	.asciiz "True"
L_4:
	.asciiz "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
L_2:
	.asciiz "False"
L_3:
	.asciiz "None"
	.text
	.globl main
main:
	sw $ra,-12($sp)
	sw $fp,-16($sp)
	move $fp, $sp
	addi $sp,$sp,-32
	la $t0,L_5
	sw $t0,-4($fp)
	li $v0,4
	lw $a0,-4($fp)
	syscall
	la $t0,L_0
	sw $t0,-8($fp)
	li $v0,4
	lw $a0,-8($fp)
	syscall
main_done:
	lw $ra,-12($fp)
	lw $fp,-16($fp)
	addi $sp,$sp,32
	jr $ra
