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

test:
	subu $sp, $sp, 4
	sw $fp, 0($sp)
	move $fp, $sp
	subu $sp, $sp, 100
	sw $a0, -4($fp)
	sw $a1, -8($fp)
	sw $a2, -12($fp)
	sw $a3, -16($fp)
	lw $a0, 8($fp)
	sw $a0, -20($fp)
	lw $t0, -20($fp)
	lw $t1, -12($fp)
	lw $t2, -16($fp)
	add $t0, $t1, $t2
	sw $t0, -20($fp)
	lw $t3, -20($fp)
	move $v0, $t3
	addi $sp, $sp, 100
	lw $fp, 0($sp)
	addi $sp, $sp, 4
	jr $ra
main:
	subu $sp, $sp, 4
	sw $fp, 0($sp)
	move $fp, $sp
	subu $sp, $sp, 100
	li $t4, 0
	sw $t4, -4($fp)
	li $t5, 1
	sw $t5, -8($fp)
	li $t6, 2
	sw $t6, -12($fp)
	li $t7, 3
	sw $t7, -16($fp)
	li $t0, 4
	sw $t0, -20($fp)
	lw $a0, -20($fp)
	lw $a1, -16($fp)
	lw $a2, -12($fp)
	lw $a3, -8($fp)
	lw $s0, -4($fp)
	subu $sp, $sp, 4
	lw $s0, 0($sp)
	subu $sp, $sp, 4
	sw $ra, 0($sp)
	jal test
	move $t1, $v0
	sw $t1, -24($fp)
	lw $ra, 0($sp)
	addi $sp, $sp, 4
	subu $sp, $sp, 4
	sw $ra, 0($sp)
	lw $t2, -24($fp)
	move $a0, $t2
	jal write
	sw $t2, -24($fp)
	lw $ra, 0($sp)
	addi $sp, $sp, 4
	move $v0, $0
	addi $sp, $sp, 100
	lw $fp, 0($sp)
	addi $sp, $sp, 4
	jr $ra
