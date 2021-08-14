b Execute
r
b *0x4016c0
b *0x4027a0
b *0x00000000004017e5
b *0x00000000004017e6
c
# _dl_aux_init
# b *0x442180
# c
# Just after call of _dl_aux_init
# b *0x402800
# SIGSEGV in __tunable_get_val
b *0x442110
c
