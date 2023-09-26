target extended-remote :3333

# print demangled symbols
set print asm-demangle on

# set backtrace limit to not have infinite backtrace loops
set backtrace limit 32

# detect unhandled exceptions, hard faults and panics
break DefaultHandler
break HardFault
break rust_begin_unwind

break main

monitor arm semihosting enable

load

# start the process but immediately halt the processor
#stepi

# run to next breakpoint (main)
continue
