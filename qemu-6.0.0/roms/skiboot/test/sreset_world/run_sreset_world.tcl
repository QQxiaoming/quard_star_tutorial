source ../../external/mambo/skiboot.tcl
source ../../external/mambo/mambo_utils.tcl

mysim go
mysim memory fwrite 0x30000000 0x300000 skiboot-sreset_world.dump
exit
