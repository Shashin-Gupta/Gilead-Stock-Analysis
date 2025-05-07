// monte_carlo_risk_analysis.cpp
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <random>
#include <ctime>
#include <curl/curl.h>
#include "include/json.hpp"

using json = nlohmann::json;
using namespace std;

struct StockData {
    string date;
    double close;
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

string to_date_string(time_t epoch) {
    tm* timeinfo = gmtime(&epoch);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return string(buffer);
}

vector<StockData> fetch_stock_data(const string& symbol) {
    // Calculate period1 (6 months ago) and period2 (now)
    time_t now = time(nullptr);
    time_t period1 = now - 60 * 60 * 24 * 180;
    time_t period2 = now;

    string url = "https://apidojo-yahoo-finance-v1.p.rapidapi.com/stock/v2/get-timeseries?symbol=" + symbol +
                 "&period1=" + to_string(period1) + "&period2=" + to_string(period2) + "&region=US";

    CURL* curl = curl_easy_init();
    string readBuffer;
    vector<StockData> data;

    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "X-RapidAPI-Key: 497f5dd499msh47f32f252553702p116afajsn1c145341c541");
        headers = curl_slist_append(headers, "X-RapidAPI-Host: apidojo-yahoo-finance-v1.p.rapidapi.com");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            try {
                json j = json::parse(readBuffer);
                if (j.contains("timestamp") && j.contains("indicators") && j["indicators"].contains("quote")) {
                    auto timestamps = j["timestamp"];
                    auto closes = j["indicators"]["quote"][0]["close"];

                    for (size_t i = 0; i < timestamps.size(); ++i) {
                        if (i < closes.size() && !closes[i].is_null()) {
                            time_t raw_time = timestamps[i];
                            string date = to_date_string(raw_time);
                            data.push_back({date, closes[i]});
                        }
                    }
                    sort(data.begin(), data.end(), [](auto& a, auto& b) { return a.date < b.date; });
                }
            } catch (const std::exception& e) {
                cerr << "JSON parse error: " << e.what() << endl;
            }
        } else {
            cerr << "cURL error: " << curl_easy_strerror(res) << endl;
        }
    }
    return data;
}

vector<StockData> generate_future_data(const vector<StockData>& past_data, const string& end_date_str) {
    vector<StockData> future;
    if (past_data.size() < 2) return future;

    struct tm last_tm = {};
    strptime(past_data.back().date.c_str(), "%Y-%m-%d", &last_tm);
    time_t last_time = timegm(&last_tm);

    struct tm tm_end = {};
    strptime(end_date_str.c_str(), "%Y-%m-%d", &tm_end);
    time_t end_time = timegm(&tm_end);

    double mean_daily_return = 0.0, stdev = 0.0;
    vector<double> returns;
    for (size_t i = 1; i < past_data.size(); ++i) {
        double change = (past_data[i].close - past_data[i - 1].close) / past_data[i - 1].close;
        returns.push_back(change);
    }
    for (double r : returns) mean_daily_return += r;
    mean_daily_return /= returns.size();
    for (double r : returns) stdev += pow(r - mean_daily_return, 2);
    stdev = sqrt(stdev / returns.size());

    mt19937 gen(random_device{}());
    normal_distribution<> dist(mean_daily_return, stdev);

    double price = past_data.back().close;
    for (time_t t = last_time + 86400; t <= end_time; t += 86400) {
        double change = dist(gen);
        price *= (1 + change);
        future.push_back({to_date_string(t), price});
    }
    return future;
}

double simulate_investment(const vector<StockData>& data, int trials, const string& kill_date) {
    mt19937 gen(random_device{}());
    int success_count = 0;

    for (int t = 0; t < trials; ++t) {
        double value = 1.0;
        for (size_t i = 1; i < data.size(); ++i) {
            if (data[i].date > kill_date) break;
            double change = (data[i].close - data[i - 1].close) / data[i - 1].close;
            value *= (1 + change);
        }
        if (value > 1.0) success_count++;
    }
    return static_cast<double>(success_count) / trials;
}

void risk_summary(const vector<StockData>& data) {
    double max_drop = 0, max_rise = 0;
    for (size_t i = 1; i < data.size(); ++i) {
        double change = (data[i].close - data[i - 1].close) / data[i - 1].close;
        max_drop = min(max_drop, change);
        max_rise = max(max_rise, change);
    }
    cout << "Max Daily Rise: " << max_rise * 100 << "%\n";
    cout << "Max Daily Drop: " << max_drop * 100 << "%\n";
}

int main() {
    string symbol = "GILD";
    string kill_date = "2025-06-19";
    int trials = 10000;

    cout << "Fetching stock data for " << symbol << "...\n";
    vector<StockData> past_data = fetch_stock_data(symbol);

    if (past_data.empty()) {
        cerr << "Failed to fetch data." << endl;
        return 1;
    }

    vector<StockData> future_data = generate_future_data(past_data, kill_date);

    cout << "Running Monte Carlo Simulation on projected data...\n";
    double prob_win = simulate_investment(future_data, trials, kill_date);
    cout << "Probability of Profit by " << kill_date << ": " << prob_win * 100 << "%\n";

    risk_summary(future_data);

    return 0;
}
