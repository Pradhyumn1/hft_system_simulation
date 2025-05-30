*High-Frequency Trading System*

Overview
This project implements a high-frequency trading (HFT) system in C++ for the project "Integration of Strategy and Prices Component in a High-Frequency Trading System." It uses a lock-free queue for low-latency communication between a price feed and a momentum-based trading strategy, with multithreading for concurrent processing. The system supports runtime configuration and logs events to a file and console.
Components

Config: Parses command-line arguments for system parameters (e.g., symbol, update interval).
Logger: Thread-safe logging to file and console.
LockFreeQueue: A circular buffer using std::atomic for thread-safe price updates.
PricesComponent: Simulates a price feed (default: BTCUSD, ~200 Hz) with Gaussian noise.
StrategyComponent: Executes a momentum strategy (20-tick window, 0.005 threshold).
HFTSystem: Manages threads and coordinates components.

Prerequisites

Compiler: C++17-compliant (e.g., g++ 7+, clang, MSVC).
OS: Windows, Linux, or macOS.
Dependencies: C++ Standard Library (no external libraries).
Build Tools: Make or CMake (optional).

Installation

Clone or Download:
Download hft_system.cpp, Makefile, and CMakeLists.txt or clone the repository.


Build with Make (Unix-like systems):make


Outputs hft_system.
Clean: make clean.


Build with CMake (cross-platform):mkdir build && cd build
cmake ..
cmake --build .


Outputs hft_system (or hft_system.exe on Windows) in build/.


Manual Compilation:g++ -std=c++17 -Wall -Wextra -g3 -pthread hft_system.cpp -o hft_system



Usage

Run the Program:
Default configuration:./hft_system


Custom configuration:./hft_system --symbol=ETHUSD --initial-price=2000.0 --update-interval=2.5 --window=30 --threshold=0.01 --log-file=eth_log.txt




Output:
Logs to console and hft_log.txt (or specified file) with timestamps, price updates, momentum, and trades.
Example:1625234567.123456: Starting HFT System...
1625234567.123789: Price feed initialized for BTCUSD
1625234567.124012: Strategy initialized for BTCUSD
1625234567.124567: Price update - BTCUSD: 100.45
1625234567.124890: Momentum for BTCUSD: 0.0067
1625234567.125123: 1625234567.125123: BUY BTCUSD at 100.45




Shutdown:
Press Ctrl+C to stop, triggering clean thread shutdown.



Configuration

Command-Line Arguments:
--symbol=<symbol>: Trading symbol (default: BTCUSD).
--initial-price=<price>: Starting price (default: 100.0).
--update-interval=<ms>: Price update interval (default: 5.0).
--window=<ticks>: Strategy lookback window (default: 20).
--threshold=<value>: Momentum threshold (default: 0.005).
--log-file=<file>: Log file (default: hft_log.txt).


Example:./hft_system --symbol=ETHUSD --update-interval=10.0



Code Structure

hft_system.cpp: Contains all components.
Config: Command-line argument parsing.
Logger: Thread-safe logging.
LockFreeQueue: Lock-free price update queue.
PricesComponent: Simulated price feed.
StrategyComponent: Momentum-based trading strategy.
HFTSystem: Thread and component management.
main: System entry point.



Performance Notes

Latency: Lock-free queue (std::atomic) minimizes contention, improving over mutex-based queues.
Threading: Two threads (price feed, strategy) for parallel processing.
Scalability: Supports single symbol; extendable for multiple symbols.

Limitations

Simulation: Uses random price data. Integrate with real APIs (e.g., Binance) for production.
Trading: Simulates trades via logging. Add exchange API for live trading.
Single Symbol: Extend for multiple symbols via configuration.

Building and Running

With Make:make run


With CMake:cd build
cmake --build . --target hft_system
./hft_system



Troubleshooting

Compilation Errors:
Ensure C++17 support (-std=c++17).
Include -pthread for threading support.
Verify <iomanip> is included for std::setprecision.


Warnings:
Fixed sign comparison issues by using size_t for sizes.
Included timestamp in trade logs to resolve unused parameter warning.



Future Improvements

Add real market data API integration.
Support multiple symbols with thread pools.
Implement configuration file support.
Add performance metrics (e.g., latency, trade frequency).

Contributing
Submit pull requests or issues for bugs, features, or optimizations.
License
Provided as-is for educational purposes. No specific license; use at your own risk.
Acknowledgments

Based on the project "Integration of Strategy and Prices Component in a High-Frequency Trading System."
Optimized for low-latency HFT with C++17 and lock-free data structures.

