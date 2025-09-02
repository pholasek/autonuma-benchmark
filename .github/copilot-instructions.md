# AutoNUMA Benchmark
AutoNUMA benchmark is a C-based NUMA (Non-Uniform Memory Access) memory migration benchmarking tool suite that measures and visualizes NUMA-aware memory allocation and migration performance. It consists of memory-intensive benchmark programs, real-time monitoring tools, and automated plotting capabilities.

Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

## Working Effectively

### Prerequisites and Dependencies
- Install all required dependencies:
  ```bash
  sudo apt update
  sudo apt install -y libnuma-dev gnuplot gcc numactl
  ```
- **CRITICAL HARDWARE REQUIREMENT**: This benchmark suite requires NUMA hardware with at least 2 NUMA nodes to run. Single-node systems will abort with "This machine has less than 2 nodes."
- Check NUMA configuration: `numactl --hardware`

### Building the Benchmark Suite
- Bootstrap and build the repository:
  ```bash
  # NEVER CANCEL: Full build takes ~1 second. Set timeout to 30+ seconds.
  make clean
  numactl --hardware | gawk -v MoF=0 -v file="numa01.c" -f preproc.awk > numa01.prep.c
  numactl --hardware | gawk -v MoF=0 -v file="numa02.c" -f preproc.awk > numa02.prep.c
  make
  ```
- **CRITICAL**: The preprocessing step using AWK is REQUIRED before building. It analyzes the NUMA hardware topology and generates hardware-specific C source files.
- Build creates multiple benchmark variants: numa01, numa02, and versions with THREAD_ALLOC, HARD_BIND, INVERSE_BIND, and SMT configurations.

### Running Benchmarks
- **NEVER CANCEL BENCHMARKS**: Memory benchmarks may run for 10-60+ minutes depending on system and test type. Set timeout to 120+ minutes.
- Basic benchmark execution:
  ```bash
  # Requires multi-NUMA hardware, runs for extended periods
  ./start_bench.sh      # Basic numa01 and numa02 tests
  ./start_bench.sh -h   # Show all available options
  ./start_bench.sh -A   # Run ALL available tests (longest duration)
  ```
- Available test options:
  - `-s`: Run numa02_SMT test additionally
  - `-t`: Run numa01_THREAD_ALLOC test additionally  
  - `-b`: Run *_HARD_BIND tests additionally
  - `-i`: Run *_INVERSE_BIND tests additionally
  - `-A`: Run all available tests
  - `-m`: Use minimal number of threads per NUMA node

### Monitoring and Analysis Tools
- NUMA memory monitoring:
  ```bash
  ./nmstat -s           # System-wide NUMA memory usage
  ./nmstat -V           # Show version
  ./nmstat <PID>        # Monitor specific process
  ./nmstat -n <pattern> # Monitor processes matching pattern
  ```
- Generated output files:
  - `<testname>.txt`: Raw monitoring data during benchmark execution
  - `<testname>.pdf`: Generated plots via gnuplot (if gnuplot available)

## Validation
- Always verify NUMA hardware before running benchmarks: `numactl --hardware` should show multiple nodes.
- On single-node systems, you can build and test tools but cannot run actual benchmarks.
- **Memory Requirements**: Tests allocate 1-3GB per benchmark. Ensure sufficient system memory.
- Always run `make clean` before building to ensure clean state.

## Common Tasks
The following are outputs from frequently run commands to save time:

### Repository root structure
```
ls -la
Makefile          # Build configuration
README            # Basic usage documentation  
start_bench.sh    # Main benchmark runner script
numa01.c          # First benchmark program source
numa02.c          # Second benchmark program source
nmstat.c          # NUMA monitoring tool source
preproc.awk       # Hardware analysis preprocessor
genplot.awk       # Plot generation script
plot.sh           # Real-time plotting script
```

### System requirements check
```bash
numactl --hardware
# Must show: available: N nodes (where N >= 2)
# Single node systems cannot run benchmarks
```

### Build artifacts (after make)
```
numa01 numa02 nmstat                    # Main executables
numa01_THREAD_ALLOC numa01_HARD_BIND    # Benchmark variants
numa02_SMT numa02_HARD_BIND             # Additional variants
numa01.prep.c numa02.prep.c             # Generated source files
```

## Troubleshooting
- **"No rule to make target 'numa01.prep.c'"**: Run the preprocessing AWK commands before make.
- **"This machine has less than 2 nodes"**: NUMA benchmarks require multi-node hardware. Use for development/code changes only on single-node systems.
- **Benchmark exits quickly with no output**: This is normal behavior - benchmarks are memory-intensive and may run silently for extended periods.
- **"Permission denied" during benchmark**: Some tests may require root privileges for optimal NUMA control.
- **Missing libnuma**: Install with `sudo apt install libnuma-dev`.
- **Missing gnuplot**: Install with `sudo apt install gnuplot` for plot generation.
- **Build warnings about format strings**: Normal compiler warnings from nmstat.c - build still succeeds.

## Key Development Areas
- `numa01.c`: Memory sharing benchmark (3GB allocation, multiple processes/threads)
- `numa02.c`: Per-thread local memory benchmark (1GB allocation, single process)
- `nmstat.c`: Real-time NUMA memory statistics monitoring tool
- `start_bench.sh`: Orchestrates benchmark execution with various configurations
- `preproc.awk`: Critical hardware detection and source code generation
- `plot.sh`: Background plotting during benchmark execution
- Makefile: Builds multiple binary variants with different compile-time options

## Important Notes
- This is primarily a benchmarking and measurement tool, not a development framework.
- Changes should focus on measurement accuracy, hardware compatibility, or output formatting.
- Always test changes on actual multi-NUMA hardware when possible.
- Preserve the preprocessing workflow - it's essential for hardware-specific optimization.