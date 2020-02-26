  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.global hexasc, delay, time2string
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
hexasc:
	andi	$a0, $a0, 0xf		# save only 4 lsb
	slti	$t0, $a0, 10 		# om a0 är mindre än 10 (t0) -> t1 = 1
	beq	$t0, $zero, else 	# om bokstav (t1 = 1) -> jump to else
	nop				# delay slot filler (just in case)
	addi	$v0, $a0, 48 		# return ASCII värde
	j	L1			# return
	nop
else:
	addi	$v0, $a0, 55 		# return ASCII värde
L1:
	jr	$ra			# return
	nop
	
  # function delay
delay:
while:
	beq	$a0, $zero, done
	nop

	addi	$a0, $a0, -1
	
	addi	$t0, $zero, 100		#change value
	addi	$t1, $zero, 0
for:
	beq	$t0, $t1, while
	nop
	addi	$t1, $t1, 1
	j	for
	nop
done:
 	jr	$ra
 	nop
 	
  # function time2string
time2string:
	PUSH	$s0
 	PUSH	$ra
 	PUSH	$a0
 	
 	add	$s0, $a1, $zero
 	
 	srl	$a0, $s0, 0	# onödig
 	jal	hexasc
 	nop
 	POP	$a0
 	sb	$v0, 4($a0)
 	PUSH	$a0
 	
 	srl	$a0, $s0, 4
 	jal	hexasc
 	nop
 	POP	$a0
 	sb	$v0, 3($a0)
 	PUSH	$a0
 	
 	addi	$t0, $zero, 0x3a			# :
 	sb	$t0, 2($a0)
 	
 	srl	$a0, $s0, 8
 	jal	hexasc
 	nop
 	POP	$a0
 	sb	$v0, 1($a0)
 	PUSH	$a0
 	
 	srl	$a0, $s0, 12
 	jal	hexasc
 	nop
 	POP	$a0
 	sb	$v0, 0($a0)
 	PUSH	$a0
 	
 	POP	$a0				# lägg till X
 	lb	$t0, 4($a0)
 	sll	$t0, $t0, 8
 	lb	$t1, 3($a0)
 	add	$t0, $t0, $t1
 	addi	$t1, $zero, 0x3030
 	bne	$t0, $t1, noX
 	nop
 	addi	$t0, $zero, 0x58		
 	sb	$t0, 5($a0)
 	addi	$t0, $zero, 0x0			# blank space
 	sb	$t0, 6($a0)
 	j	Xdone
 	nop
 	
noX: 	
 	addi	$t0, $zero, 0x0			# blank space
 	sb	$t0, 5($a0)
 	
Xdone:
 	POP	$ra
 	POP	$s0
	
 	jr	$ra
 	nop


