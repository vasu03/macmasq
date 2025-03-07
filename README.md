# macmasq - MAC Randomizer Tool

A command-line utility that generates a random, locally administered MAC address and applies it to a specified network interface. This tool is written in C and leverages standard system calls available on Unix-like operating systems. This utility is intended to help users understand network interface manipulation and MAC address handling in C.

## Prerequisites

- **Operating Systems:** Linux based systems.
- **Compiler:** GCC or any compatible C compiler.
- **Privileges:** Administrative or root access is required to change network settings.

## Build Instructions

This project uses a Makefile for building the tool. So make sure you have Makefile utilities available and configured properly on your system.

The Makefile uses the following options for compilation:

```make
opt = -O3 -Wall -std=c2x
```

To build the tool, simply run:

```bash
make
```

This will compile the source code and generate the executable named `macmasq`.

To clean up the build artifacts, run:

```bash
make clean
```

## Usage

### Linux

1. **Build the Tool:**
   ```bash
   make
   ```
2. **Run the Tool:**
   ```bash
   sudo ./macmasq <INTERFACE>
   ```
   Replace `<INTERFACE>` with your network interface name (e.g., `eth0`, `wlan0`, `enp0s3`).

`NOTE`: 
- The tool currently supports MAC changing in DHCP configured networks and do not support changing the MAC in Static configured networks. Consider this while using this tool on your network.
- Support for other operating systems and Static configured network may be introduced in near future.

## Disclaimer

**Educational Use Only:**  
This tool is provided for educational and research purposes only. Ensure you have explicit permission to modify the MAC address on any network interface. The developers assume no liability for any misuse or damages resulting from the use of this tool.
