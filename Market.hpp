// Project Identifier: 0E04A31E0D60C01986ACB20081C9D8722A1899B6
#pragma once
#ifndef MARKET_HPP
#define MARKET_HPP
#include <vector>
#include <queue>
#include <iostream>
#include "Order.hpp"
#include "Trader.hpp"
#include "P2random.h"


// ----------------------------------------------------------------------- //
//               Structs with Comparators and containers!                 //
// --------------------------------------------------------------------- //

// Comparators for priority queues
struct BuyOrderComparator operator()(const Order& a, const Order& b) {
    if (a.price == b.price) {
        return a.arrivalOrder > b.arrivalOrder;
    }
    return a.price < b.price; // Higher price has higher priority
};

struct SellOrderComparator operator()(const Order& a, const Order& b) {
    if (a.price == b.price) {
        return a.arrivalOrder > b.arrivalOrder;
    }
    return a.price > b.price; // Lower price has higher priority
};

// helper for time traveler mode
// sell = true : sell order (can buy it)
struct time_traveler() { // index in vector will be stock#
    /*word_bank = { 
        'n': "No Trades"
        'b': "Can Buy"
        'c': "Complete"
        'p': "Potential"


        mode = 'c' || mode = 'p' in order to find a time_traveler for the stock
    } */
    char mode = 'n';

    size_t sell_time = 0;
    size_t buy_time = 0;
    size_t buy_price = 0; // first price will be lower - used for buy info
    size_t sell_price = 0; // first price should be > 0 - used for sell price
    size_t potential_buy_price = 0;
    size_t potential_buy_time = 0;
};

// no need for trader ID, because we can represent it as the index of our vector
struct Trader {
    uint32_t totalBought;
    uint32_t totalSold;
    long long netTransfer; // Use long long for larger sums

    // Constructor
    Trader() : totalBought(0), totalSold(0), netTransfer(0) {}

    // Functions to update trader's stats
    void bought(uint32_t quantity, uint32_t price) {
        totalBought += quantity;
        netTransfer -= static_cast<long long>(quantity * price);
    }

    void sold(uint32_t quantity, uint32_t price) {
        totalSold += quantity;
        netTransfer += static_cast<long long>(quantity) * price;
    }
};

struct Order {
    int timestamp = 0;
    int traderID = 0;
    int stockID = 0;
    bool isBuy = true; // true for buy, false for sell
    int price = 0;
    int quantity = 0;
    int orderNum = 0; // Used to break ties based on arrival order

    // Constructor
    Order(int ts, int tID, int sID, bool buy, int p, int q, int orderN) 
        : timestamp(ts), traderID(tID), stockID(sID), isBuy(buy),
            price(p), quantity(q), orderNum(orderN) { }

    // You may add additional member functions if needed
};


// ----------------------------------------------------------------------- //
//                     Market Class Definitions!!!                        //
// --------------------------------------------------------------------- //

class Market {
public:
    Market(int numStocks_in, int numTraders_in, bool v, bool m, bool tI, bool tT);

    void proccess_input (std::istringstream &ss);
    void printEndOfDaySummary();
    void printTraderInfo();

private:
    // read from input
    int numStocks;
    int numTraders;
    bool verbose = false;
    bool median = false;
    bool traderInfo = false;
    bool timeTravelers = false;

    // member values that WILL change throughout
    int currentTime;
    int tradesCompleted;
    int arrivalCounter; // For tie-breaking based on arrival order

    // Data structures
    std::vector<time_traveler> time_traveler_tracker;
    std::vector<Trader> traders;
    std::vector<std::priority_queue<Order, std::vector<Order>, BuyOrderComparator>> buyOrders;
    std::vector<std::priority_queue<Order, std::vector<Order>, SellOrderComparator>> sellOrders;

    // private member functions
    void matchOrders(int stockID);
    void outputMedianPrices();
    void updateTimeTravelers(const Order& order);
    void printTimeTravelerInfo();
    void verbose_helper(int trader, int shares, int stock, int seller, int price);

};
    
// ----------------------------------------------------------------------- //
//                 PUBLIC MEMBER FUCNTION IMPLEMENTATIONS                 //
// --------------------------------------------------------------------- //


Market::Market(int numStocks_in, int numTraders_in, bool v, bool m, bool tI, bool tT) : 
        numStocks(numStocks_in), numTraders(numTraders_in), verbose(v), median(m),
        traderInfo(tI), timeTravelers(tT), currentTime(0), tradesCompleted(0), 
        arrivalCounter(0), traders(numTraders, Trader(0)), 
        buyOrders(numStocks), sellOrders(numStocks) {
        

    // resize our traveler vector iff the mode is used, else its a waste...
    if (timeTravelers) time_traveler_tracker.resize(numStocks);


    // Initialize traders with correct IDs
    for (int i = 0; i < numTraders; ++i) {
        traders[i] = Trader(i);
    }

    // Initialize data structures for median and time travelers if needed
    // ...
} // Market ctor


void Market::proccess_input (std::istringstream &ss) {
    int timestamp = 0;
    std::string buySell = "";
    char tChar = '\0';
    char sChar = '\0';
    char aux = '\0';
    int traderID = 0;
    int stockID = 0;
    int price = 0;
    int quantity = 0;

    while (ss >> timestamp) { // time stamp auto read by while loop

        ss >> buySell >> aux >> traderID >> aux 
            >> stockID >> aux >> price >> aux >> quantity; // check that read works!!

        // for debug read in test
        cout << "read in... Time: " << timestamp << ", T" << traderID << ", S" 
             << stockID << ", $" << price << ", #" << quantity << "\n";


        // Check for input errors
        if (timestamp < currentTime) { // most likely error first :)
            std::cerr << "Error: Timestamps not non-decreasing.\n";
            exit(1);
        } if (timestamp < 0) {
            std::cerr << "Error: Negative timestamp.\n";
               exit(1);
        } if (traderID < 0 || traderID >= numTraders) {
            std::cerr << "Error: Invalid trader ID.\n";
            exit(1);
        } if (stockID < 0 || stockID >= numStocks) {
            std::cerr << "Error: Invalid stock ID.\n";
            exit(1);
        } if (price <= 0 || quantity <= 0) {
            std::cerr << "Error: Non-positive price or quantity.\n";
            exit(1);
        }

        // Handle timestamp change
        if (timestamp != currentTime) {
            if (median) outputMedianPrices(); // call Median at each time change
            currentTime = timestamp;
        }


        // CHECK REST
        bool isBuy = (buySell == "BUY");
        Order newOrder(timestamp, traderID, stockID, buySell, price, quantity, arrivalCounter++);
    
        // Update time traveler data
        if (timeTravelers) {
            updateTimeTravelers(newOrder);
        }

        // Add order to market and attempt matching
        if (isBuy) {
            buyOrders[stockID].push(newOrder);
            matchOrders(stockID);
        } else {
            sellOrders[stockID].push(newOrder);
            matchOrders(stockID);
        }
    } // while
} // process_input

// End of day summary
void Market::printEndOfDaySummary() {
    std::cout << "---End of Day---\n";
    std::cout << "Trades Completed: " << tradesCompleted << "\n";
}

// Trader info output
void Market::printTraderInfo() {
    std::cout << "---Trader Info---\n";
    for (const auto& trader : traders) {
        std::cout << "Trader " << trader.traderID << " bought "
                  << trader.totalBought << " and sold " << trader.totalSold
                  << " for a net transfer of $" << trader.netTransfer << "\n";
    }
}







// ----------------------------------------------------------------------- //
//                 PRIVATE MEMBER FUCNTION IMPLEMENTATIONS                //
// --------------------------------------------------------------------- //


// Matching logic
void Market::matchOrders(int stockID) {
    auto& buyPQ = buyOrders[stockID];
    auto& sellPQ = sellOrders[stockID];

    while (!buyPQ.empty() && !sellPQ.empty()) {
        Order buyOrder = buyPQ.top();
        Order sellOrder = sellPQ.top();

        if (sellOrder.price > buyOrder.price) {
            // No match possible
            break;
        }

        // Determine trade price
        int tradePrice;
        if (sellOrder.timestamp < buyOrder.timestamp ||
            (sellOrder.timestamp == buyOrder.timestamp &&
             sellOrder.arrivalOrder < buyOrder.arrivalOrder)) {
            tradePrice = sellOrder.price;
        } else {
            tradePrice = buyOrder.price;
        }

        // Determine trade quantity
        int tradeQuantity = std::min(buyOrder.quantity, sellOrder.quantity);

        // Update trader statistics
        traders[buyOrder.traderID].bought(tradeQuantity, tradePrice);
        traders[sellOrder.traderID].sold(tradeQuantity, tradePrice);

        // Update quantities
        buyOrder.quantity -= tradeQuantity;
        sellOrder.quantity -= tradeQuantity;

        tradesCompleted++;

        // Update median data
        if (median) {
            // Add tradePrice to median data structures
            // ...
        }

        // Verbose output
        if (verbose) {
            std::cout << "Trader " << buyOrder.traderID << " purchased "
                      << tradeQuantity << " shares of Stock " << stockID
                      << " from Trader " << sellOrder.traderID << " for $"
                      << tradePrice << "/share\n";
        }

        // Remove or update orders in priority queues
        buyPQ.pop();
        sellPQ.pop();

        if (buyOrder.quantity > 0) {
            buyPQ.push(buyOrder);
        }
        if (sellOrder.quantity > 0) {
            sellPQ.push(sellOrder);
        }
    }
}

// Output median prices
void Market::outputMedianPrices() {
    // For each stock, calculate and output the median if trades have occurred
    // Implement efficient median calculation using two heaps per stock
    // ...
}

// Update time traveler data
void Market::updateTimeTravelers(const Order& order) {

/*
1. Start in "No trades", where you can buy a stock

2. If you find a sell order, switch to "Can Buy"
    a.  if you get another stock you can buy, buy it if it's lower than we bought ours for

3. If you can sell (fill a buy order) for higher than you bought it for, mark as Complete" 
    a.  if you get another stock you can SELL, SELL it if it's higher than we already sold for

4. If you find another sell order (you can buy) and its less than we bought ours for, switch to "potential" mode
    a.  repeat 2 - 3
    b.  once done with step three, compare the amount gained
    c.  if amount gained is MORE than we previously had, change our potential traveler to our traveler and set potential's mode to "No trades"
    d.  if amount gained is NOT MORE than we had, remain in "Can buy mode until we can turn a bigger profit"
    e.  whenever we get a new buy order...compare it to our sell_price and potential sell price!!!
    f.  stay in potential.mode = 'b' until I can make more than before

        'n': "No Trades"        
        'b': "Can Buy"            
        'c': "Complete"           
        'p': "Potential"        

    char mode = 'n';
    size_t sell_time = 0;
    size_t buy_time = 0;
    size_t buy_price = 0; // first price will be lower - used for buy info
    size_t sell_price = 0; // first price should be > 0 - used for sell price
    time_traveler potential;
*/ 
    int id = order.stockID;
    time_traveler curr_stock = time_traveler_tracker[id];

    // !.isBuy means its a sell order (TT can buy it)
    if (!order.isBuy) {
        // if haven't bought yet OR this price is cheaper than what we bought for...
        if (curr_stock.mode == 'n' || (curr_stock.mode == 'b' && curr_stock.buy_price > order.price)) {
            curr_stock.buy_price = order.price;
            curr_stock.buy_time = order.timestamp;
            curr_stock.mode = 'b';
            return;
        }

        // if in c mode and price is cheaper than what we bought for... 
        if ( (curr_stock.mode) == 'c' && (order.price < curr_stock.buy_price) ) {
            curr_stock.potential_buy_price = order.price;
            curr_stock.potential_buy_time = order.timestamp;
            curr_stock.mode = 'p';
            return;
        }
    } // if we can buy

     // .isBuy (TT can sell to this fool)
    if (order.isBuy) {

        // if haven't bought anything yet
        if (curr_stock.mode = 'n') return;

        // if haven't sold yet and is selling for more than we bought for OR if we're in 'c' and its selling for more than we sold ours for
        if ( (curr_stock.mode == 'b' && curr_stock.buy_price < order.price) || (curr_stock.mode == 'c' && curr_stock.sell_price < order.price) ) {
            curr_stock.sell_price = order.price;
            curr_stock.sell_time = order.timestamp;
            curr_stock.mode = 'c';
            return;
        }

        // if in potential mode and an order comes along that can earn more than what we already have, change it to our main and put into 'c'
        // !!!might cause issues with unsigned ints!!!
        if ( curr_stock.mode == 'p' && (order.price - curr_stock.potential_buy_price) > (curr_stock.sell_price - curr_stock.buy_price) ) {
            curr_stock.buy_price = curr_stock.potential_buy_price;
            curr_stock.buy_time = curr_stock.potential_buy_time;
            curr_stock.sell_price = order.price;
            curr_stock.sell_time = order.timestamp;
            curr_stock.mode = 'c';
            return;
        }

        // if in potential mode and a lesser bid value comes through
        if (curr_stock.mode == 'p' && curr_stock.potential_buy_price > order.price) {
            curr_stock.potential_buy_price = order.price;
            curr_stock.potential_buy_time = order.timestamp;
            return;
        }

    } // if we can sell
} 

// Time traveler info output
void Market::printTimeTravelerInfo() {
    std::cout << "---Time Travelers---\n";
    for (int stockID = 0; stockID < numStocks; ++stockID) {

        // get our current stock in "curr"
        auto curr = time_traveler_tracker[stockID];

        // check that both times have been initialized and that we didn't buy after our sell
        if (curr.buy_time != 9999 || curr.sell_time != 9999 || curr.buy_time > curr.sell_time) {
            std::cout << "A time traveler would buy Stock " << stockID << " at time " << curr.buy_time << " for " 
                      << curr.buy_price << " and sell it at time " <<  curr.sell_time << " for " << curr.sell_price << "\n";
                      continue;
        }
        // else, we have no TT info avaliable for the stock we're at
        std::cout << "A time traveler could not make a profit on Stock " << stockID << "\n";
    } // for
}

void Market::verbose_helper(int trader, int shares, int stock, int seller, int price) {
    std::cout << "Trader " << trader << " purchased " << sNum << " shares of Stock " 
              << stock << " from Trader " << seller << " for $" << price << "/share\n";
}

#endif // MARKET_HPP