#!/bin/bash

# After running generated/build/tcp_server (Not through gdb)
coredumpctl list

# coredumpctl gdb <PID>

###
# (gdb) info threads
# (gdb) bt
# (gdb) thread apply all bt
# (gdb) frame N
# (gdb) info locals
# (gdb) print <the pointer you dereferenced>
# (gdb) list
###