# POK lab

This is the project for the Real-Time Systems and Scheduling course at SJTU SE.

# Prerequisites

Ensure you have the necessary prerequisites and dependencies installed by referring to the documentation at https://pok-kernel.github.io/usage/.

# Usage

## Normal run
To run the project in normal mode, use the following command:
```
make run
```

## Run in `-nographic` mode

For running QEMU in `-nographic` mode, use the following command.
```
make run-nographic
```

To stop qemu, use the following command.
```
pkill qemu
```
This may be rude if you have other QEMU processes running as well. If so, you can use the following command instead.
```
kill $(cat qemu.pid)
```
