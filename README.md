# Gilead Stock Monte Carlo Risk Analysis

This C++ project simulates the potential profitability of investing in Gilead Sciences (GILD) stock ahead of a major clinical trial readout scheduled for **June 19, 2025**. Using historical data and Monte Carlo simulations, the program estimates the likelihood that a daily-investment strategy yields a positive return by the "kill date".

## Clinical Trial Context

Gilead is conducting an important **Phase 3 trial** related to [lenacapavir](https://www.gilead.com/news-and-press/company-statements/gilead-announces-positive-phase-3-results-for-lenacapavir) — a long-acting HIV treatment. The results are expected around mid-2025, making this an ideal case for **event-driven trading simulation**.

- Trial data expected: **June 19, 2025**
- Drug: Lenacapavir
- Ticker: `GILD`

Related Links:
- [Gilead's Lenacapavir Pipeline](https://www.gilead.com/science-and-medicine/therapeutic-areas/hiv/lenacapavir)
- [Press Release: Positive Phase 3 Results](https://www.gilead.com/news-and-press/press-room/press-releases/2023/12/gileads-lenacapavir-demonstrates-positive-results)

---

## How It Works

The program does the following:

1. Downloads 6 months of historical price data for GILD using Yahoo Finance via RapidAPI.
2. Estimates daily return mean and volatility.
3. Projects future prices using a **geometric Brownian motion** model.
4. Simulates thousands of potential outcomes until the kill date.
5. Computes the probability of ending up with a profit.

---

## How to Use

### 1. Requirements

- macOS or Linux
- g++
- libcurl
- Internet connection

Install required packages (on macOS):

```bash
brew install curl
```

### 2. Build and Run

```bash
mkdir -p include
curl -L https://github.com/nlohmann/json/releases/latest/download/json.hpp -o include/json.hpp
```

### 3. Output

The program will display:
- Estimated probability of profit by June 19, 2025
- Max daily rise/drop simulated

## Notes
- The Monte Carlo model uses historical volatility and drift — it does not incorporate options, sentiment, or insider movement.
- You can modify the symbol or kill_date in the main() function for other events.

Created by Shashin Gupta, 2025
#### This project is for educational purposes only. Not investment advice.
