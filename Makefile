.PHONY: debugprobe-upload
debugprobe-upload:
	# ELF_PATH should be the path to the .elf file
	# this is run like:
	# 	make debugprobe-upload ELF_PATH="path/to/bin.elf"
	sudo openocd -f interface/cmsis-dap.cfg \
             -f target/rp2040.cfg \
             -c "transport select swd" \
             -c "adapter speed 5000" \
             -c "program ${ELF_PATH} verify reset exit"

# For using with gdb
.PHONY: debugprobe-server
debugprobe-server:
	sudo openocd -f interface/cmsis-dap.cfg \
             -f target/rp2040.cfg \
             -c "transport select swd" \
             -c "adapter speed 5000"

.PHONY: open-gdb
open-gdb:
	# consider using gdb-multiarch
	gdb -ex "target extended-remote :3333" ${ELF_PATH}