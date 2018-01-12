.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text

read:
	li $v0, 4
	la $a0, _prompt
	syscall
	li $v0, 5
	syscall
	jr $ra

write:
	li $v0, 1
	syscall
	li $v0, 4
	la $a0, _ret
	syscall
	move $v0, $0
	jr $ra

fact:
	subu $sp, $sp, 4
	sw $fp, 0($sp)
	move $fp, $sp
	subu $sp, $sp, 100
	sw $a0, -4($fp)
	lw $t0, -4($fp)
	bne $t0, 1, label2
	lw $t1, -4($fp)
	move $v0, $t1
	addi $sp, $sp, 100
	lw $fp, 0($sp)
	addi $sp, $sp, 4
	jr $ra
	j label3
label2:
	lw $t3, -4($fp)
	addi $t2, $t3, -1
	sw $t2, -8($fp)
