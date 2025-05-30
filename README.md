
# High-Frequency Trading System
=============================

## Overview
This project implements a high-frequency trading (HFT) system in C++ for the project "Integration of Strategy and Prices Component in a High-Frequency Trading System." It integrates a simulated price feed and a momentum-based trading strategy using multithreading and a lock-free queue for low-latency performance. The system runs for exactly 90 seconds, supports runtime configuration via command-line arguments, and logs events to both the console and a file.

## Components

- **Config**: Parses command-line arguments for system parameters (e.g., symbol, update interval).
- **Logger**: Thread-safe logging to a file (default: `hft_log.txt`) and console.
- **LockFreeQueue**: A circular buffer using `std::atomic` for thread-safe price updates.
- **PricesComponent**: Simulates a price feed (default: BTCUSD, ~200 Hz) with Gaussian noise.
- **StrategyComponent**: Executes a momentum strategy (20-tick window, 0.005 threshold).
- **HFTSystem**: Manages threads, coordinates components, and enforces a 90-second runtime.

## Prerequisites

- **Compiler**: C++17-compliant (e.g., g++ 7+, clang, MSVC).
- **Operating System**: Windows, Linux, or macOS.
- **Dependencies**: C++ Standard Library (no external libraries required).
- **Build Tools**: Make or CMake (optional, for build automation).

## Installation

### Clone the Repository:
```bash
git clone https://github.com/Pradhyumn1/hft_system_simulation.git
cd your-repo
```

> Replace `your-username` and `your-repo` with your GitHub username and repository name.

### Build with Make (Unix-like systems):
```bash
make
```
Outputs `hft_system` executable.

To clean build files:
```bash
make clean
```

### Build with CMake (cross-platform):
```bash
mkdir build && cd build
cmake ..
cmake --build .
```
Outputs `hft_system` (or `hft_system.exe` on Windows) in `build/`.

### Manual Compilation:
```bash
g++ -std=c++17 -Wall -Wextra -g3 -pthread hft_system.cpp -o hft_system
```

## Usage

### Run the Program:

Default configuration (runs for 90 seconds):
```bash
./hft_system
```

Custom configuration:
```bash
./hft_system --symbol=ETHUSD --initial-price=2000.0 --update-interval=2.5 --window=30 --threshold=0.01 --log-file=eth_log.txt
```

### Output:

Logs to console and `hft_log.txt` (or specified file) with timestamps, price updates, momentum calculations, and trade signals.

Example output:
```
1625234567.123456: Starting HFT System...
1625234567.123789: Price feed initialized for BTCUSD
1625234567.124012: Strategy initialized for BTCUSD
1625234567.124567: Price update - BTCUSD: 100.45
1625234567.124890: Momentum for BTCUSD: 0.0067
1625234567.125123: BUY BTCUSD at 100.45
1625234657.123456: Shutting down HFT System after 90 seconds...
```

### Shutdown:

- The system automatically terminates after 90 seconds with a clean thread shutdown.
- Press Ctrl+C for early termination.

## Configuration

Command-Line Arguments:
- `--symbol=<symbol>`: Trading symbol (default: BTCUSD).
- `--initial-price=<price>`: Starting price (default: 100.0).
- `--update-interval=<ms>`: Price update interval in milliseconds (default: 5.0, ~200 Hz).
- `--window=<ticks>`: Strategy lookback window (default: 20).
- `--threshold=<value>`: Momentum threshold (default: 0.005).
- `--log-file=<file>`: Log file name (default: `hft_log.txt`).

Example:
```bash
./hft_system --symbol=ETHUSD --update-interval=10.0
```

## Code Structure

- **hft_system.cpp**: Main source file containing all components.
  - Config: Parses command-line arguments for system configuration.
  - Logger: Handles thread-safe logging to file and console.
  - LockFreeQueue: Lock-free circular buffer for price updates.
  - PricesComponent: Generates simulated price updates.
  - StrategyComponent: Processes prices and executes momentum-based trades.
  - HFTSystem: Manages threads, enforces 90-second runtime, and coordinates components.
  - main: Entry point to initialize and run the system.

## Performance Notes

- **Latency**: Utilizes a lock-free queue (`std::atomic`) for minimal contention, optimized for HFT.
- **Threading**: Two threads (price feed and strategy) ensure concurrent processing.
- **Runtime**: Fixed at 90 seconds for controlled execution, with clean thread shutdown.
- **Scalability**: Designed for a single symbol; extendable for multiple symbols.

## Limitations

- **Simulation**: Uses random price data (Gaussian noise). For production, integrate with a real market data API (e.g., Binance, Alpaca).
- **Trading**: Simulates trades via logging. Add an exchange API for live trading.
- **Single Symbol**: Currently supports one symbol. Future versions can handle multiple symbols.

## Building and Running

With Make:
```bash
make run
```

With CMake:
```bash
cd build
cmake --build . --target hft_system
./hft_system
```

## Troubleshooting

**Compilation Errors**:
- Ensure C++17 support with `-std=c++17`.
- Include `-pthread` for threading support.
- Verify `<iomanip>` is included for `std::setprecision` (already included in the code).

Example error: no member named 'setprecision' indicates missing `<iomanip>`; this is resolved in the current code.

**Runtime Issues**:
- Check `hft_log.txt` for error messages if the program fails.
- Ensure sufficient disk space for logging.
- If the system doesn’t terminate after 90 seconds, verify the timer logic in `HFTSystem::run`.

**Early Termination**:
- Use Ctrl+C to stop before 90 seconds; the system will shut down cleanly.

## Future Improvements

- Integrate real-time market data APIs (e.g., FIX, WebSocket).
- Support multiple symbols with dynamic thread pools.
- Add configuration file support for easier setup.
- Include performance metrics (e.g., latency, trade frequency).
- Enhance error handling with detailed diagnostics.

## Contributing
Contributions are welcome! Please submit pull requests or open issues for bugs, features, or optimizations on GitHub.

## License
This project is provided as-is for educational purposes. No specific license is applied; use and modify at your own risk.

## Acknowledgments

- Based on the project "Integration of Strategy and Prices Component in a High-Frequency Trading System."
- Inspired by high-frequency trading requirements for low-latency price processing and strategy execution.
- Optimized with C++17, lock-free data structures, and a 90-second runtime limit.
- Reference video: YouTube Explanation.

## Contact
For questions or support, open an issue on the GitHub repository.
