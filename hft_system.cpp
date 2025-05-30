#include <iostream>
#include <thread>
#include <atomic>
#include <array>
#include <string>
#include <vector>
#include <deque>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>

struct Config {
    std::string symbol = "BTCUSD";
    double initial_price = 100.0;
    size_t price_history_size = 100;
    size_t queue_size = 100;
    double update_interval_ms = 5.0; // ~200 Hz
    size_t strategy_window = 20;
    double strategy_threshold = 0.005;
    std::string log_file = "hft_log.txt";

    Config(int argc, char* argv[]) {
        if (argc > 1) {
            parse_args(argc, argv);
        }
    }

    void parse_args(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg.find("--symbol=") == 0) symbol = arg.substr(9);
            else if (arg.find("--initial-price=") == 0) initial_price = std::stod(arg.substr(16));
            else if (arg.find("--update-interval=") == 0) update_interval_ms = std::stod(arg.substr(17));
            else if (arg.find("--window=") == 0) strategy_window = std::stoul(arg.substr(9));
            else if (arg.find("--threshold=") == 0) strategy_threshold = std::stod(arg.substr(12));
            else if (arg.find("--log-file=") == 0) log_file = arg.substr(11);
        }
    }
};

class Logger {
private:
    std::ofstream file_;
    std::mutex mutex_;

public:
    Logger(const std::string& filename) {
        file_.open(filename, std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }

    ~Logger() {
        if (file_.is_open()) file_.close();
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / 1e6;
        file_ << std::fixed << std::setprecision(6) << timestamp << ": " << message << std::endl;
        std::cout << std::fixed << std::setprecision(6) << timestamp << ": " << message << std::endl;
    }
};

template<typename T, size_t Size>
class LockFreeQueue {
private:
    std::array<T, Size> buffer_;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};

public:
    bool push(const T& item) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % Size;
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false; // Queue full
        }
        buffer_[current_tail] = item;
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t current_head = head_.load(std::memory_order_relaxed);
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false; // Queue empty
        }
        item = buffer_[current_head];
        head_.store((current_head + 1) % Size, std::memory_order_release);
        return true;
    }
};

class PricesComponent {
private:
    Config config_;
    double current_price_;
    std::deque<double> price_history_;
    LockFreeQueue<std::tuple<std::string, double, double>, 100>& price_queue_;
    std::atomic<bool> running_;
    std::mt19937 rng_;
    std::normal_distribution<double> dist_;
    Logger& logger_;

public:
    PricesComponent(const Config& config, LockFreeQueue<std::tuple<std::string, double, double>, 100>& price_queue, Logger& logger)
        : config_(config), current_price_(config.initial_price), price_queue_(price_queue), running_(true),
          rng_(std::random_device{}()), dist_(0.0, 1.0), logger_(logger) {
        for (size_t i = 0; i < config_.price_history_size; ++i) {
            price_history_.push_back(config_.initial_price + dist_(rng_));
        }
    }

    void start_price_feed() {
        logger_.log("Price feed initialized for " + config_.symbol);
        simulate_price_feed();
    }

    void simulate_price_feed() {
        while (running_) {
            current_price_ += dist_(rng_);
            price_history_.push_back(current_price_);
            if (price_history_.size() > config_.price_history_size) {
                price_history_.pop_front();
            }
            double timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count() / 1e6;
            if (price_queue_.push({config_.symbol, current_price_, timestamp})) {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << "Price update - " << config_.symbol << ": " << current_price_;
                logger_.log(oss.str());
            }
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(config_.update_interval_ms * 1000)));
        }
    }

    void stop() {
        running_ = false;
    }
};

class StrategyComponent {
private:
    Config config_;
    LockFreeQueue<std::tuple<std::string, double, double>, 100>& price_queue_;
    std::deque<double> price_history_;
    int position_;
    std::atomic<bool> running_;
    Logger& logger_;

public:
    StrategyComponent(const Config& config, LockFreeQueue<std::tuple<std::string, double, double>, 100>& price_queue, Logger& logger)
        : config_(config), price_queue_(price_queue), position_(0), running_(true), logger_(logger) {
        std::mt19937 rng(std::random_device{}());
        std::normal_distribution<double> dist(0.0, 1.0);
        for (size_t i = 0; i < config_.strategy_window; ++i) {
            price_history_.push_back(config_.initial_price + dist(rng));
        }
    }

    void start_strategy() {
        logger_.log("Strategy initialized for " + config_.symbol);
        process_price_updates();
    }

    void process_price_updates() {
        while (running_) {
            std::tuple<std::string, double, double> item;
            if (price_queue_.pop(item)) {
                if (std::get<0>(item) == config_.symbol) {
                    on_price_update(std::get<1>(item), std::get<2>(item));
                }
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }
        }
    }

    void on_price_update(double price, double timestamp) {
        price_history_.push_back(price);
        if (price_history_.size() > config_.strategy_window) {
            price_history_.pop_front();
        }

        if (price_history_.size() == config_.strategy_window) {
            std::vector<double> prices(price_history_.begin(), price_history_.end());
            double momentum = (prices.back() - prices.front()) / prices.front();
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(4) << "Momentum for " << config_.symbol << ": " << momentum;
            logger_.log(oss.str());
            if (momentum > config_.strategy_threshold && position_ != 1) {
                execute_trade("BUY", price, timestamp);
                position_ = 1;
            } else if (momentum < -config_.strategy_threshold && position_ != -1) {
                execute_trade("SELL", price, timestamp);
                position_ = -1;
            }
        }
    }

    void execute_trade(const std::string& action, double price, double timestamp) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << timestamp << ": " << action << " " << config_.symbol << " at " << price;
        logger_.log(oss.str());
    }

    void stop() {
        running_ = false;
    }
};

class HFTSystem {
private:
    Config config_;
    Logger logger_;
    LockFreeQueue<std::tuple<std::string, double, double>, 100> price_queue_;
    PricesComponent prices_;
    StrategyComponent strategy_;
    std::thread price_thread_;
    std::thread strategy_thread_;

public:
    HFTSystem(int argc, char* argv[])
        : config_(argc, argv), logger_(config_.log_file), price_queue_(),
          prices_(config_, price_queue_, logger_), strategy_(config_, price_queue_, logger_) {}

    void run() {
        using namespace std::chrono;
        double start_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count() / 1e6;
        logger_.log("Starting HFT System...");

        price_thread_ = std::thread(&PricesComponent::start_price_feed, &prices_);
        strategy_thread_ = std::thread(&StrategyComponent::start_strategy, &strategy_);

        double end_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count() / 1e6;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << "Startup completed in " << (end_time - start_time) * 1000 << "ms";
        logger_.log(oss.str());

        // Run for 90 seconds
        auto start = system_clock::now();
        while (duration_cast<seconds>(system_clock::now() - start).count() < 90) {
            std::this_thread::sleep_for(milliseconds(100)); // Check every 100ms
        }

        // Stop the system after 90 seconds
        stop();
    }

    void stop() {
        prices_.stop();
        strategy_.stop();
        if (price_thread_.joinable()) price_thread_.join();
        if (strategy_thread_.joinable()) strategy_thread_.join();
        logger_.log("Shutting down HFT System after 90 seconds...");
    }
};

int main(int argc, char* argv[]) {
    try {
        HFTSystem hft(argc, argv);
        hft.run();
    } catch (const std::exception& e) {
        std::cerr << "Main error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}