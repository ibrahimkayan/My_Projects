.section .bss
input_line_buffer:      .space 256
species_name_storage:   .space 64
song_token_input_area:  .space 8192
song_token_output_area: .space 8192
output_token_counter:   .quad 0
current_generation:     .quad 0
species_identifier_code:.quad 0

.section .data
newline_char_data:      .asciz "\n"
space_char_data:        .byte 32
sparrow_gen_prefix_text: .asciz "Sparrow Gen "
sparrow_gen_prefix_len = . - sparrow_gen_prefix_text
warbler_gen_prefix_text : .asciz "Warbler Gen "
warbler_gen_prefix_len = . - warbler_gen_prefix_text
nightingale_gen_prefix_text: .asciz "Nightingale Gen "
nightingale_gen_prefix_len = . - nightingale_gen_prefix_text
colon_space_separator:  .asciz ": "
colon_space_separator_len = . - colon_space_separator

.section .text
.global _start

_start:
    xor %rax, %rax
    movq %rax, current_generation(%rip)

    # Read input line
    xor %eax, %eax
    xor %edi, %edi
    lea input_line_buffer(%rip), %rsi
    mov $256, %rdx
    syscall

    # Setup for parsing
    lea input_line_buffer(%rip), %rsi
    lea species_name_storage(%rip), %r10
    lea song_token_input_area(%rip), %r11
    xor %rcx, %rcx # Species name index
    xor %r9, %r9   # Input buffer read index
    xor %r8, %r8   # Token write index / count

# Extract species name (until space)
extract_species_name_loop:
    movzbq (%rsi, %r9), %rax
    cmpb $0x20, %al
    je extraction_species_done
    movb %al, (%r10, %rcx)
    inc %rcx
    inc %r9
    jmp extract_species_name_loop

extraction_species_done:
    movb $0x00, (%r10, %rcx) # Null-terminate species
    inc %r9                 # Skip space

extract_token_loop:
    movzbq (%rsi, %r9), %rax
    cmpb $0x0a, %al         # Newline?
    je extraction_tokens_done
    cmpb $0x20, %al         # Space?
    je skip_token_whitespace
    movl %eax, (%r11, %r8, 4) # Store token (4 bytes)
    inc %r9
    jmp token_char_stored
skip_token_whitespace:
    inc %r9                 # Skip space
    jmp extract_token_loop
token_char_stored:
    inc %r8                 # Increment token count
    jmp extract_token_loop

extraction_tokens_done:
    mov %r8, output_token_counter(%rip) # Store initial token count

    # Identify Bird Type & Route
    lea species_name_storage(%rip), %r15
    movzbq (%r15), %rax     # First char of species
    cmpb $'S', %al
    je route_to_sparrow
    cmpb $'W', %al
    je route_to_warbler
    cmpb $'N', %al
    je route_to_nightingale
route_to_nightingale:
    movq $2, species_identifier_code(%rip) # Nightingale ID = 2
    jmp begin_nightingale_processing
route_to_sparrow:
    movq $0, species_identifier_code(%rip) # Sparrow ID = 0
    jmp begin_sparrow_processing
route_to_warbler:
    movq $1, species_identifier_code(%rip) # Warbler ID = 1
    jmp begin_warbler_processing

begin_nightingale_processing:
    lea song_token_input_area(%rip), %rsi # Input tokens base
    xor %r14, %r14                     # Input token index
    xor %r10, %r10                     # Output token index/count
    jmp nightingale_main_processing_loop

nightingale_main_processing_loop:
    cmp %r14, output_token_counter(%rip) # Processed all input?
    je nightingale_processing_finished
    lea song_token_output_area(%rip), %r15 # Output base
    lea (%rsi, %r14, 4), %r12          # Input token addr
    movzbq (%r12), %rax                # Get char
    cmpb $'+', %al
    je nightingale_execute_addition
    cmpb $'*', %al
    je nightingale_execute_duplication
    cmpb $'-', %al
    je nightingale_execute_subtraction
    cmpb $'H', %al
    je nightingale_execute_harmony

    lea (%r15, %r10, 4), %r11          # Output addr
    movl (%r12), %ebx                  # Read token
    movl %ebx, (%r11)                  # Write token
    inc %r14                           # Next input
    inc %r10                           # Next output
    jmp nightingale_main_processing_loop

nightingale_processing_finished:
    mov %r10, output_token_counter(%rip)
    jmp exit # Changed from 'exit' to avoid conflict with syscall name

nightingale_execute_addition: # '+' : Duplicate last 2 (like Warbler '*')
    cmp $2, %r10
    jl nightingale_skip_addition_logic
    lea -4(%r15, %r10, 4), %r12 # last token addr
    movl (%r12), %ebx          # ebx = last
    lea -8(%r15, %r10, 4), %r12 # 2nd last addr
    movl (%r12), %ecx          # ecx = 2nd last
    lea (%r15, %r10, 4), %r12  # 1st new slot addr
    movl %ecx, (%r12)          # write 2nd last
    lea 4(%r12), %r12          # 2nd new slot addr
    movl %ebx, (%r12)          # write last
    add $2, %r10               # +2 tokens
nightingale_skip_addition_logic:
    inc %r14                   # Consume '+'
    call _common_post_operation_tasks
    jmp nightingale_main_processing_loop

nightingale_execute_duplication: # '*' : Duplicate entire song
    mov %r10, %r12             # r12 = elements to copy
    test %r12, %r12            # Count == 0?
    jz nightingale_duplication_complete # Skip if empty
    xor %r9, %r9               # r9 = source index
nightingale_copy_loop:
    lea (%r15, %r9, 4), %r13   # Source addr
    movl (%r13), %eax          # Read token
    lea (%r15, %r10, 4), %r13  # Dest addr (end of song)
    movl %eax, (%r13)          # Write token
    inc %r9                    # Next source
    inc %r10                   # Next dest (extend song)
    cmp %r9, %r12              # Copied all?
    jne nightingale_copy_loop
nightingale_duplication_complete:
    inc %r14                   # Consume '*'
    call _common_post_operation_tasks
    jmp nightingale_main_processing_loop

nightingale_execute_subtraction: # '-' : If last 2 identical, remove one
    cmp $2, %r10
    jl nightingale_skip_subtraction
    lea -4(%r15, %r10, 4), %r12
    movl (%r12), %ebx          # Last token
    lea -8(%r15, %r10, 4), %r12
    movl (%r12), %ecx          # Second last token
    cmpl %ebx, %ecx            # Compare
    jne nightingale_skip_subtraction # Skip if different
    dec %r10                   # Same, remove one
    lea (%r15, %r10, 4), %r12
    movl $0, (%r12)            # Zero slot (optional)
nightingale_skip_subtraction:
    inc %r14                   # Consume '-'
    call _common_post_operation_tasks
    jmp nightingale_main_processing_loop

nightingale_execute_harmony: # 'H' : a,b,c -> a-c, b-a
    cmp $3, %r10
    jl nightingale_skip_harmony
    lea -4(%r15, %r10, 4), %r12 # Addr of c
    movzbq (%r12), %rcx        # char c
    lea -8(%r15, %r10, 4), %r12 # Addr of b
    movzbq (%r12), %rbx        # char b
    lea -12(%r15, %r10, 4), %r12 # Addr of a
    movzbq (%r12), %rax        # char a
    # Overwrite 'a' slot with a-c
    movb %al, (%r12)
    movb $'-', 1(%r12)
    movb %cl, 2(%r12)
    movb $0x00, 3(%r12)
    # Overwrite 'b' slot with b-a
    add $4, %r12               # Addr where 'b' was
    movb %bl, (%r12)
    movb $'-', 1(%r12)
    movb %al, 2(%r12)
    movb $0x00, 3(%r12)
    dec %r10                   # Net -1 token
nightingale_skip_harmony:
    inc %r14                   # Consume 'H'
    call _common_post_operation_tasks
    jmp nightingale_main_processing_loop


begin_warbler_processing:
    lea song_token_input_area(%rip), %rsi
    xor %r14, %r14
    xor %r10, %r10
    jmp warbler_main_processing_loop

warbler_main_processing_loop:
    cmp %r14, output_token_counter(%rip) # Processed all input?
    je warbler_processing_finished
    lea song_token_output_area(%rip), %r15 # Output base
    lea (%rsi, %r14, 4), %r12          # Input token addr
    movb (%r12), %al                   # Get char
    cmpb $'+', %al
    je warbler_execute_addition
    cmpb $'*', %al
    je warbler_execute_duplication
    cmpb $'-', %al
    je warbler_execute_subtraction
    cmpb $'H', %al
    je warbler_execute_harmony
    lea (%r15, %r10, 4), %r11          # Output addr
    movl (%r12), %eax                  # Read token
    movl %eax, (%r11)                  # Write token
    inc %r14                           # Next input
    inc %r10                           # Next output
    jmp warbler_main_processing_loop

warbler_processing_finished:
    mov %r10, output_token_counter(%rip)
    jmp exit

warbler_execute_addition: # '+' : Replace last 2 with T-C
    cmp $2, %r10
    jl warbler_skip_addition_logic
    sub $2, %r10               # Overwrite 2nd last
    lea (%r15, %r10, 4), %r12  # Target addr
    movl $0x00432D54, (%r12)   # T-C-null
    inc %r10                   # Net +1 token (was -2, now +1 of the new combined)
warbler_skip_addition_logic:
    inc %r14                   # Consume '+'
    call _common_post_operation_tasks
    jmp warbler_main_processing_loop

warbler_execute_duplication: # '*' : Duplicate last 2 (a,b -> a,b,a,b)
    cmp $2, %r10
    jl warbler_skip_duplication_logic
    lea -4(%r15, %r10, 4), %r12 # Addr of last (b)
    movl (%r12), %ebx          # ebx = b
    lea -8(%r15, %r10, 4), %r12 # Addr of 2nd last (a)
    movl (%r12), %ecx          # ecx = a
    lea (%r15, %r10, 4), %r12  # Addr for first duplicate (a)
    movl %ecx, (%r12)          # Write a
    lea 4(%r12), %r12          # Addr for second duplicate (b)
    movl %ebx, (%r12)          # Write b
    add $2, %r10               # +2 tokens
warbler_skip_duplication_logic:
    inc %r14                   # Consume '*'
    call _common_post_operation_tasks
    jmp warbler_main_processing_loop

warbler_execute_subtraction: # '-' : Remove last token
    cmp $1, %r10
    jl warbler_skip_subtraction_logic
    dec %r10                   # Decrease count
    lea (%r15, %r10, 4), %r12  # Addr of removed slot
    movl $0, (%r12)            # Zero out (optional)
warbler_skip_subtraction_logic:
    inc %r14                   # Consume '-'
    call _common_post_operation_tasks
    jmp warbler_main_processing_loop

warbler_execute_harmony: # 'H' : Append 'T'
    lea (%r15, %r10, 4), %r12  # Addr for new token
    movl $0x54, (%r12)         # 'T' + nulls
    inc %r10                   # Increase count
    inc %r14                   # Consume 'H'
    call _common_post_operation_tasks
    jmp warbler_main_processing_loop


begin_sparrow_processing:
    lea song_token_input_area(%rip), %rsi # Input tokens base
    xor %r14, %r14                     # Input token index
    xor %r10, %r10                     # Output token index/count
    jmp sparrow_main_processing_loop

sparrow_main_processing_loop:
    cmp %r14, output_token_counter(%rip) # Processed all input?
    je sparrow_processing_finished

    lea (%rsi, %r14, 4), %r12          # Current input token addr
    lea song_token_output_area(%rip), %r15 # Output tokens base

    movzbq (%r12), %rax                # Get token char
    cmpb $'+', %al
    je sparrow_execute_addition
    cmpb $'*', %al
    je sparrow_execute_duplication
    cmpb $'-', %al
    je sparrow_execute_subtraction
    cmpb $'H', %al
    je sparrow_execute_harmony

    lea (%r15, %r10, 4), %r11          # Output slot addr
    movl (%r12), %ebx                  # Read input token
    movl %ebx, (%r11)                  # Write to output
    inc %r10                           # Inc output count
    inc %r14                           # Inc input index
    jmp sparrow_main_processing_loop

sparrow_processing_finished:
    mov %r10, output_token_counter(%rip) # Update final count
    jmp exit

sparrow_execute_addition: # '+' : Merge last two (note2, note1 -> note2-note1)
    cmp $2, %r10
    jl sparrow_skip_addition_logic
    lea -4(%r15, %r10, 4), %r12 # note1 addr (originally last token)
    movzbq (%r12), %rbx        # note1 char
    sub $4, %r12               # note2 addr (originally second to last)
    movzbq (%r12), %rcx        # note2 char
    movb %cl, (%r12)           # Write note2
    movb $'-', 1(%r12)         # Write '-'
    movb %bl, 2(%r12)          # Write note1
    movb $0x00, 3(%r12)        # Null term
    sub $1, %r10               # One less token
sparrow_skip_addition_logic:
    inc %r14                   # Consume '+'
    call _common_post_operation_tasks
    jmp sparrow_main_processing_loop

sparrow_execute_duplication: # '*' : Duplicate last token
    cmp $1, %r10
    jl sparrow_skip_duplication_logic
    lea -4(%r15, %r10, 4), %r12 # Last token addr
    movl (%r12), %ebx          # Read last token
    lea (%r15, %r10, 4), %r12  # Next output slot addr
    movl %ebx, (%r12)          # Write duplicate
    inc %r10                   # One more token
sparrow_skip_duplication_logic:
    inc %r14                   # Consume '*'
    call _common_post_operation_tasks
    jmp sparrow_main_processing_loop

sparrow_execute_harmony: # 'H' : Apply C->T, T->C, D->D-T
    xor %r13, %r13             # Loop index
sparrow_harmony_apply_loop:
    cmp %r13, %r10             # Done?
    je sparrow_harmony_done
    lea (%r15, %r13, 4), %r12  # Current token addr
    movzbq (%r12), %rbx        # Get char
    cmpb $'C', %bl
    je sparrow_harmony_change_c
    cmpb $'T', %bl
    je sparrow_harmony_change_t
    cmpb $'D', %bl
    je sparrow_harmony_change_d
    inc %r13                   # No match, next token
    jmp sparrow_harmony_apply_loop
sparrow_harmony_change_c:      # C -> T
    movl $0x54, (%r12)         # 'T' + nulls
    inc %r13
    jmp sparrow_harmony_apply_loop
sparrow_harmony_change_t:      # T -> C
    movl $0x43, (%r12)         # 'C' + nulls
    inc %r13
    jmp sparrow_harmony_apply_loop
sparrow_harmony_change_d:      # D -> D-T
    movl $0x00542D44, (%r12)   # D-T-null
    inc %r13
    jmp sparrow_harmony_apply_loop
sparrow_harmony_done:
    inc %r14                   # Consume 'H'
    call _common_post_operation_tasks
    jmp sparrow_main_processing_loop

sparrow_execute_subtraction: # '-' : Remove first C, then T, then D
    cmp $1, %r10
    jl sparrow_finish_subtraction_op
    lea song_token_output_area(%rip), %rdi # Output base
    xor %r13, %r13             # Search index
sparrow_sub_search_c_entry:          # Find C
    cmp %r13, %r10
    je sparrow_sub_search_t_entry
    lea (%rdi, %r13, 4), %r12
    cmpb $'C', (%r12)
    je sparrow_sub_target_identified
    inc %r13
    jmp sparrow_sub_search_c_entry
sparrow_sub_search_t_entry:     # Find T
    xor %r13, %r13
sparrow_sub_search_t_loop:
    cmp %r13, %r10
    je sparrow_sub_search_d_entry
    lea (%rdi, %r13, 4), %r12
    cmpb $'T', (%r12)
    je sparrow_sub_target_identified
    inc %r13
    jmp sparrow_sub_search_t_loop
sparrow_sub_search_d_entry:     # Find D
    xor %r13, %r13
sparrow_sub_search_d_loop:
    cmp %r13, %r10
    je sparrow_finish_subtraction_op # Not found
    lea (%rdi, %r13, 4), %r12
    cmpb $'D', (%r12)
    je sparrow_sub_target_identified
    inc %r13
    jmp sparrow_sub_search_d_loop
sparrow_sub_target_identified: # Found target at index r13
    mov %r13, %r9              # r9 = index to remove
sparrow_sub_shift_loop:        # Shift elements left
    mov %r9, %rax              # current index
    inc %rax                   # source index (index + 1)
    cmp %rax, %r10             # source valid? (is (index+1) < count?)
    jge sparrow_sub_shifting_complete # if current index is last, shifting is done
    lea (%rdi, %r9, 4), %r12   # Dest addr
    lea 4(%r12), %r11          # Source addr
    movl (%r11), %eax          # Read source
    movl %eax, (%r12)          # Write dest
    inc %r9                    # Next position
    jmp sparrow_sub_shift_loop
sparrow_sub_shifting_complete:
    dec %r10                   # One less token
sparrow_finish_subtraction_op:    # End '-' op
    inc %r14                   # Consume '-'
    call _common_post_operation_tasks
    jmp sparrow_main_processing_loop



_common_post_operation_tasks:
    pushq %rax # Save a few just in case, though print_current_generation_state saves many.
    pushq %rbx
    call print_current_generation_state
    incq current_generation(%rip)
    popq %rbx
    popq %rax
    ret

print_current_generation_state:
    pushq %rax; pushq %rbx; pushq %rcx; pushq %rdx # Save volatile regs
    pushq %rsi; pushq %rdi; pushq %r9;  pushq %r8
    pushq %r11; pushq %r12; pushq %r13; pushq %r14; pushq %r15
    pushq %r10 # Save R10 (count)

    # Print species prefix
    mov $1, %rax; mov $1, %rdi # write, stdout
    movq species_identifier_code(%rip), %rbx
    cmpq $0, %rbx              # Sparrow?
    je .print_state_sparrow
    cmpq $1, %rbx              # Warbler?
    je .print_state_warbler
    lea nightingale_gen_prefix_text(%rip), %rsi # Default Nightingale
    mov $nightingale_gen_prefix_len, %rdx
    jmp .print_state_prefix_written
.print_state_sparrow:
    lea sparrow_gen_prefix_text(%rip), %rsi
    mov $sparrow_gen_prefix_len, %rdx
    jmp .print_state_prefix_written
.print_state_warbler:
    lea warbler_gen_prefix_text(%rip), %rsi
    mov $warbler_gen_prefix_len, %rdx
.print_state_prefix_written:
    syscall                    # Print prefix

    # Print generation number
    movq current_generation(%rip), %rsi
    call integer_to_string_and_print

    # Print separator ": "
    mov $1, %rax; mov $1, %rdi
    lea colon_space_separator(%rip), %rsi
    mov $colon_space_separator_len, %rdx
    syscall

    # Print song elements
    xor %r8, %r8               # Loop counter
    mov (%rsp), %r10           # Get saved R10 (count) from stack frame for comparison
.print_state_elements_loop:
    cmp %r8, %r10              # Done printing?
    je .print_state_elements_done
    lea song_token_output_area(%rip), %rsi
    lea (%rsi, %r8, 4), %r12   # Token addr
    pushq %r8; pushq %r10      # Save loop vars before call
    mov %r12, %rdi             # Arg for length func
    call calculate_token_string_length
    mov %rax, %rdx             # RDX = length for syscall
    popq %r10; popq %r8         # Restore loop vars
    mov %r12, %rsi             # RSI = token string addr
    mov $1, %rax; mov $1, %rdi # write, stdout
    syscall                    # Print token

    # Print space if not last
    mov %r8, %rax
    inc %rax
    cmp %rax, %r10
    je .print_state_space_skip # If current is (count-1), skip space
.print_state_elements_done_check_space: # Fallthrough for printing space
    mov $1, %rax; mov $1, %rdi # write, stdout
    lea space_char_data(%rip), %rsi
    mov $1, %rdx               # Space length 1
    syscall
.print_state_space_skip:
    inc %r8                    # Next token
    jmp .print_state_elements_loop

.print_state_elements_done:
    # Print final newline
    mov $1, %rax; mov $1, %rdi
    lea newline_char_data(%rip), %rsi
    mov $1, %rdx
    syscall

    # Restore registers
    popq %r10
    popq %r15; popq %r14; popq %r13; popq %r12; popq %r11
    popq %r8;  popq %r9; popq %rdi; popq %rsi
    popq %rdx; popq %rcx; popq %rbx; popq %rax
    ret

calculate_token_string_length:
    pushq %rdi; pushq %rcx
    xor %rax, %rax             # length = 0
    mov $4, %rcx               # max = 4
.calc_length_loop:
    cmpb $0, (%rdi, %rax)      # Check byte at addr+len
    je .calc_length_finished
    inc %rax                   # Increment length
    loopnz .calc_length_loop   # Loop max 4 times or until null (or rcx becomes 0)
.calc_length_finished:
    popq %rcx; popq %rdi
    ret

integer_to_string_and_print:
    pushq %rax; pushq %rbx; pushq %rdx; pushq %rdi; pushq %rcx
    pushq %rsi # Save original rsi as well since it's used for input
    movq %rsi, %rax            # Integer to convert (original was passed in RSI)
    subq $32, %rsp             # Stack buffer (local buffer for string)
    movq %rsp, %rdi            # Buffer pointer
    xorq %rcx, %rcx            # Digit count/index in buffer
    testq %rax, %rax           # Handle 0 case
    jnz .convert_loop_entry
    movb $'0', (%rdi)
    incq %rcx
    jmp .print_converted_digits
.convert_loop_entry:           # Convert non-zero int
    movq $10, %rbx             # Base 10
.convert_next_digit_loop:
    xorq %rdx, %rdx
    divq %rbx                  # RDX:RAX / 10 -> RAX=quot, RDX=rem
    addb $'0', %dl             # ASCII digit
    movb %dl, (%rdi, %rcx)     # Store backwards in buffer
    incq %rcx
    testq %rax, %rax           # Quotient zero?
    jnz .convert_next_digit_loop
.print_converted_digits:       # Digits are reversed in buffer (e.g., 123 is "321")
    xorq %rax, %rax            # Start index for reversal (e.g. 0)
    movq %rcx, %rbx            # Count of digits
    decq %rbx                  # End index for reversal (e.g. count-1)
    cmpq $1, %rcx              # Skip if 1 digit or less (no reversal needed)
    jle .skip_reversal_logic
.reverse_in_place_loop:        # Reverse string in buffer
    cmpq %rbx, %rax            # Pointers crossed or met? (start_idx >= end_idx)
    jge .skip_reversal_logic
    movb (%rdi, %rax), %dl     # Start char
    movb (%rdi, %rbx), %sil    # End char (using sil for byte from rsi)
    movb %sil, (%rdi, %rax)    # Swap
    movb %dl, (%rdi, %rbx)     # Swap
    incq %rax
    decq %rbx
    jmp .reverse_in_place_loop
.skip_reversal_logic:          # Print the number
    movq $1, %rax              # write syscall
    movq $1, %rdi              # stdout
    movq %rsp, %rsi            # String addr (buffer on stack)
    movq %rcx, %rdx            # String length (number of digits)
    syscall
    addq $32, %rsp             # Clean stack buffer
    popq %rsi # Restore original rsi
    popq %rcx; popq %rdi; popq %rdx; popq %rbx; popq %rax # Restore other regs
    ret

exit:
    mov $60, %rax
    xor %rdi, %rdi
    syscall
