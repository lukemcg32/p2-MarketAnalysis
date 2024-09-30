// Project Identifier: 0E04A31E0D60C01986ACB20081C9D8722A1899B6
#pragma once
#ifndef MARKET_HPP
#define MARKET_HPP
#include <vector>
#include <queue>
#include <iostream>
#include <sstream>


// ----------------------------------------------------------------------- //
//               Structs with Comparators and containers!                 //
// --------------------------------------------------------------------- //


struct BuyOrderComparator {
    bool operator()(const Order& a, const Order& b) {
        if (a.price == b.price) return a.orderNum > b.orderNum;
        return a.price < b.price; // Higher price has higher priority
    }
};

struct SellOrderComparator {
    bool operator()(const Order& a, const Order& b) {
        if (a.price == b.price) return a.orderNum > b.orderNum;
        return a.price > b.price; // Lower price has higher priority
    }
};

// helper for time traveler mode
// sell = true : sell order (can buy it)
struct time_traveler { // index in vector will be stock#
    /*  'n': "No Trades"
        'b': "Can Buy"
        'c': "Complete"
        'p': "Potential"

        mode = 'c' || mode = 'p' in order to find a time_traveler for the stock */
    char mode = 'n';

    size_t sell_time = 0;
    size_t buy_time = 0;
    size_t buy_price = 0; // first price will be lower - used for buy info
    size_t sell_price = 0; // first price should be > 0 - used for sell price
    size_t potential_buy_price = 0;
    size_t potential_buy_time = 0;
};

// no need for trader ID, because we can represent it as the index of our vector
// we'll keep it for now
struct Trader {
    uint32_t traderID;
    uint32_t totalBought;
    uint32_t totalSold;
    long long netTransfer; // Use long long for larger sums

    // Constructor
    Trader(uint32_t tID) : traderID(tID), totalBought(0), totalSold(0), netTransfer(0) {}

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
    uint32_t timestamp = 0;
    uint32_t traderID = 0;
    uint32_t stockID = 0;
    bool isBuy = true; // true for buy, false for sell
    uint32_t price = 0;
    uint32_t quantity = 0;
    uint32_t orderNum = 0; // Used to break ties based on arrival order

    // Constructor
    Order(uint32_t ts, uint32_t tID, uint32_t sID, bool buy, uint32_t p, uint32_t q, const uint32_t orderN) 
        : timestamp(ts), traderID(tID), stockID(sID), isBuy(buy),
            price(p), quantity(q), orderNum(orderN) { }

    // You may add additional member functions if needed
};


// ----------------------------------------------------------------------- //
//                     Market Class Definitions!!!                        //
// --------------------------------------------------------------------- //

class Market {
public:
    Market(uint32_t numStocks_in, uint32_t numTraders_in, bool v, bool m, bool tI, bool tT); // x
    void proccess_input(std::istringstream &ss); // x
    void printEndOfDaySummary(); // x
    void printTraderInfo(); // x
    void printTimeTravelerInfo(); // x

private:
    // read from input
    uint32_t numStocks;
    uint32_t numTraders;
    bool verbose = false;
    bool median = false;
    bool traderInfo = false;
    bool timeTravelers = false;

    // member values that WILL change throughout
    uint32_t currentTime;
    uint32_t tradesCompleted;
    uint32_t arrivalCounter; // For tie-breaking based on arrival order

    // Data structures
    std::vector<time_traveler> time_traveler_tracker;
    std::vector<Trader> traders;
    std::vector<std::priority_queue<Order, std::vector<Order>, BuyOrderComparator>> buyOrders;
    std::vector<std::priority_queue<Order, std::vector<Order>, SellOrderComparator>> sellOrders;
    // fast median
    class MedianPriorityQueue {
        private:
            std::priority_queue<int> maxHeap; // smaller half - no comp makes it an auto MaxPQ
            std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap; // larger half
            void balanceHeaps();
        public:
            void insert(int num);
            double getMedian();
    };

    // private member functions
    void matchOrders(uint32_t stockID);
    void outputMedianPrices();
    
    void updateTimeTravelers(const Order& order); // x
    void verbose_helper(uint32_t trader, uint32_t shares, uint32_t stock, uint32_t seller, uint32_t price); // x

};
    
// ----------------------------------------------------------------------- //
//                 PUBLIC MEMBER FUCNTION IMPLEMENTATIONS                 //
// --------------------------------------------------------------------- //


Market::Market(uint32_t numStocks_in, uint32_t numTraders_in, bool v, bool m, bool tI, bool tT) : 
        numStocks(numStocks_in), numTraders(numTraders_in), verbose(v), median(m),
        traderInfo(tI), timeTravelers(tT), currentTime(0), tradesCompleted(0), 
        arrivalCounter(0), traders(numTraders, Trader(0)), 
        buyOrders(numStocks), sellOrders(numStocks) {
        

    // resize our traveler vector IF AND ONLY IF the mode is used, else its a waste...
    if (timeTravelers) time_traveler_tracker.resize(numStocks);


    // Initialize traders with correct IDs
    for (int i = 0; i < numTraders; ++i) {
        traders[i] = Trader(i);
    }

    // Initialize data structures for median and time travelers if needed
    // ...
} // Market ctor


void Market::proccess_input (std::istringstream &ss) {
    uint32_t timestamp = 0;
    std::string buySell = "";
    char tChar = '\0';
    char sChar = '\0';
    char aux = '\0';
    uint32_t traderID = 0;
    uint32_t stockID = 0;
    uint32_t price = 0;
    uint32_t quantity = 0;
    bool isBuy = false;

    while (ss >> timestamp) { // time stamp auto read by while loop

        ss >> buySell >> aux >> traderID >> aux 
            >> stockID >> aux >> price >> aux >> quantity; // check that read works!!

        // for debug read in test
        std::cout << "read in... Time: " << timestamp << ", T" << traderID << ", S" 
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
        isBuy = (buySell == "BUY");
        Order newOrder(timestamp, traderID, stockID, isBuy, price, quantity, arrivalCounter++);
    
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
void Market::matchOrders(uint32_t stockID) {
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
        uint32_t tradePrice;
        if (sellOrder.timestamp < buyOrder.timestamp ||
            (sellOrder.timestamp == buyOrder.timestamp &&
             sellOrder.orderNum < buyOrder.orderNum)) {
            tradePrice = sellOrder.price;
        } else {
            tradePrice = buyOrder.price;
        }

        // Determine trade quantity
        uint32_t tradeQuantity = std::min(buyOrder.quantity, sellOrder.quantity);

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
    uint32_t id = order.stockID;
    time_traveler curr_stock = time_traveler_tracker[id];

    // !.isBuy means its a sell order (TT can buy it)
    if (!order.isBuy) {
        // if haven't bought yet OR this price is cheaper than what we bought for...
        if (curr_stock.mode == 'n' || (curr_stock.mode == 'b' && curr_stock.buy_price < order.price)) {
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

        // if we're in potential mode and we get a stock that is less than our potential mode holder
        if (curr_stock.mode == 'p' && curr_stock.potential_buy_price > order.price) {
            curr_stock.potential_buy_price = order.price;
            curr_stock.potential_buy_time = order.timestamp;
            // already in potential mode
            return;
        }
        
    } // if <we can buy>

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
} // updateTT

// Time traveler info output
void Market::printTimeTravelerInfo() {
    std::cout << "---Time Travelers---\n";
    for (uint32_t stockID = 0; stockID < numStocks; ++stockID) {
        auto curr = time_traveler_tracker[stockID];
        // check if we have something complete... (in p/c mode)
        if (curr.mode == 'p' || curr.mode == 'c') {
            std::cout << "A time traveler would buy Stock " << stockID << " at time " << curr.buy_time << " for " 
                      << curr.buy_price << " and sell it at time " <<  curr.sell_time << " for " << curr.sell_price << "\n";
        } 
        else { // else, we have no TT info avaliable for the stock we're at
            std::cout << "A time traveler could not make a profit on Stock " << stockID << "\n";
        }
    } // for
}

void Market::verbose_helper(uint32_t trader, uint32_t shares, uint32_t stock, uint32_t seller, uint32_t price) {
    std::cout << "Trader " << trader << " purchased " << shares << " shares of Stock " 
              << stock << " from Trader " << seller << " for $" << price << "/share\n";
}



// ----------------------------------------------------------------------- //
//                 MedianPriorityQueue Implementations                    //
// --------------------------------------------------------------------- //


void Market::MedianPriorityQueue::balanceHeaps() {
    if (maxHeap.size() > minHeap.size() + 1) {
        minHeap.push(maxHeap.top());
        maxHeap.pop();
    } else if (minHeap.size() > maxHeap.size() + 1) {
        maxHeap.push(minHeap.top());
        minHeap.pop();
    }
}

void Market::MedianPriorityQueue::insert(int num) {
    if (maxHeap.empty() || num < maxHeap.top()) {
        maxHeap.push(num);
    } else {
        minHeap.push(num);
    }
        balanceHeaps();
}

double Market::MedianPriorityQueue::getMedian() {
    if (maxHeap.size() == minHeap.size()) {
        return (maxHeap.top() + minHeap.top()) / 2.0;
    } else if (maxHeap.size() > minHeap.size()) {
        return maxHeap.top();
    } else {
        return minHeap.top();
    }
}

#endif // MARKET_HPP