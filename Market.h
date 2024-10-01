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

// no need for trader ID, because we can represent it as the index of our vector
// we'll keep it for now
struct Trader {
    uint32_t traderID;
    uint32_t totalBought = 0;
    uint32_t totalSold = 0;
    long long netTransfer = 0; // Use long long for larger sums

    // Constructor
    Trader(uint32_t tID) : traderID(tID) {}

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

    // ctor
    Order(uint32_t ts, uint32_t tID, uint32_t sID, bool buy, uint32_t p, uint32_t q, const uint32_t orderN) 
        : timestamp(ts), traderID(tID), stockID(sID), isBuy(buy),
            price(p), quantity(q), orderNum(orderN) {}
};

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

// fast median
class MedianPriorityQueue {
    private:
        std::priority_queue<uint32_t> maxHeap; // smaller half - no comp makes it an auto MaxPQ
        std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t>> minHeap; // larger half
        void balanceHeaps();
    public:
        void insert(uint32_t num);
        uint32_t getMedian();
        void clear();
};

// ----------------------------------------------------------------------- //
//                     Market Class Definitions!!!                        //
// --------------------------------------------------------------------- //

class Market {
public:
    Market(uint32_t numStocks_in, uint32_t numTraders_in, bool v, bool m, bool tI, bool tT); // x
    void process_input_PR(std::stringstream &ss); // x
    void process_input_TL(); // x
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
    std::vector<MedianPriorityQueue> medianPQ;
    

    // private member functions
    void matchOrders(uint32_t stockID); // x
    void outputMedianPrices(uint32_t time); // x
    void updateTimeTravelers(const Order& order); // x

};
    
// ----------------------------------------------------------------------- //
//                 PUBLIC MEMBER FUCNTION IMPLEMENTATIONS                 //
// --------------------------------------------------------------------- //


Market::Market(uint32_t numStocks_in, uint32_t numTraders_in, bool v, bool m, bool tI, bool tT) : 
        numStocks(numStocks_in), numTraders(numTraders_in), verbose(v), median(m),
        traderInfo(tI), timeTravelers(tT), currentTime(0), tradesCompleted(0), 
        arrivalCounter(0), traders(numTraders, Trader(0)), 
        buyOrders(numStocks), sellOrders(numStocks) {
        

    // resize our traveler, info & median vectors IF AND ONLY IF the mode is used, else its a waste...
    if (timeTravelers) time_traveler_tracker.resize(numStocks);
    if (median) medianPQ.resize(numStocks);
    if (traderInfo) {
        // Initialize traders with correct IDs
        for (uint32_t i = 0; i < numTraders; ++i) {
            traders[i] = Trader(i);
        }
    }

} // Market ctor

void Market::process_input_TL() {
    // std::cout << "in p_i\n";
    uint32_t timestamp = 0;
    std::string buySell = "";
    char aux = '\0';
    std::string junk = "";
    uint32_t traderID = 0;
    uint32_t stockID = 0;
    uint32_t price = 0;
    uint32_t quantity = 0;

    // std::cout << "about to start looping\n";
    while (std::cin >> junk) { // time stamp auto read by while loop
        // std::cout << junk << "\n";

        std::cin >> buySell >> aux >> traderID >> aux 
            >> stockID >> aux >> price >> aux >> quantity; // check that read works!!

        // for debug read in test
        // std::cout << "read in... Time: " << timestamp << ", T" << traderID << ", S" 
        //     << stockID << ", $" << price << ", #" << quantity << "\n";


        // Check for input errors
        // removed the < 0 errors because they're all unsigned ints (uint32_t)...
        if (timestamp < currentTime) { // most likely error first :)
            std::cerr << "Error: Timestamps not non-decreasing.\n";
            exit(1);
        } 
        // if (timestamp < 0) {
        //     std::cerr << "Error: Negative timestamp.\n";
        //        exit(1);
        // } 
        if (traderID >= numTraders) {
            std::cerr << "Error: Invalid trader ID.\n";
            exit(1);
        } if (stockID >= numStocks) {
            std::cerr << "Error: Invalid stock ID.\n";
            exit(1);
        } if (price == 0 || quantity <= 0) {
            std::cerr << "Error: Non-positive price or quantity.\n";
            exit(1);
        }

        // Handle timestamp change
        if (timestamp != currentTime) {
            if (median) outputMedianPrices(currentTime); // call Median at each time change
            currentTime = timestamp;
        }


        // CHECK REST
        bool isBuy = (buySell == "BUY");
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
} // process_inpit_TL

void Market::process_input_PR (std::stringstream &ss) {
    uint32_t timestamp = 0;
    std::string buySell = "";
    char aux = '\0';
    std::string junk = "";
    uint32_t traderID = 0;
    uint32_t stockID = 0;
    uint32_t price = 0;
    uint32_t quantity = 0;


    // std::cout << "\n" << "Input stream content: " << ss.str() << std::endl;

    while (ss >> junk) { // time stamp auto read by while loop
        ss >> buySell >> aux >> traderID >> aux 
            >> stockID >> aux >> price >> aux >> quantity; // check that read works!!

        // for debug read in test
        // std::cout << "read in... Time: " << timestamp << ", T" << traderID << ", S" 
        //     << stockID << ", $" << price << ", #" << quantity << "\n";


        // Check for input errors
        if (timestamp < currentTime) { // most likely error first :)
            std::cerr << "Error: Timestamps not non-decreasing.\n";
            exit(1);
        } 
        // if (timestamp < 0) {
        //     std::cerr << "Error: Negative timestamp.\n";
        //        exit(1);
        // } 
        if (traderID >= numTraders) {
            std::cerr << "Error: Invalid trader ID.\n";
            exit(1);
        } if (stockID >= numStocks) {
            std::cerr << "Error: Invalid stock ID.\n";
            exit(1);
        } if (price == 0 || quantity <= 0) {
            std::cerr << "Error: Non-positive price or quantity.\n";
            exit(1);
        }

        // Handle timestamp change
        if (timestamp != currentTime) {
            if (median) outputMedianPrices(currentTime); // call Median at each time change
            currentTime = timestamp;
        }


        // CHECK REST
        bool isBuy = (buySell == "BUY");
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
} // process_input_PR

// End of day summary
void Market::printEndOfDaySummary() {
    std::cout << "---End of Day---\n";
    std::cout << "Trades Completed: " << tradesCompleted << "\n";
} // printEODSummary

// Trader info output
void Market::printTraderInfo() {
    std::cout << "---Trader Info---\n";
    for (const auto& trader : traders) {
        std::cout << "Trader " << trader.traderID << " bought "
                  << trader.totalBought << " and sold " << trader.totalSold
                  << " for a net transfer of $" << trader.netTransfer << "\n";
    }
} // printTraderInfo


// ----------------------------------------------------------------------- //
//                 PRIVATE MEMBER FUCNTION IMPLEMENTATIONS                //
// --------------------------------------------------------------------- //


// Matching logic
void Market::matchOrders(uint32_t stockID) {
    auto& buyPQ = buyOrders[stockID]; // call the vector position's (stockID's) priority queue
    auto& sellPQ = sellOrders[stockID];

    while (!buyPQ.empty() && !sellPQ.empty()) {
        Order buyOrder = buyPQ.top();
        Order sellOrder = sellPQ.top();

        if (sellOrder.price > buyOrder.price) { // price is higher than the buyer's willing to pay
            // no trade made
            break;
        }

        // Determine trade price
        uint32_t trade_price;

        // if the buy order comes after the order was listed as a sell, take the seller's list price
        if (sellOrder.timestamp < buyOrder.timestamp ||
            (sellOrder.timestamp == buyOrder.timestamp &&
             sellOrder.orderNum < buyOrder.orderNum)) {
            trade_price = sellOrder.price;
        } else { // else, take the buyers price offer
            trade_price = buyOrder.price;
        }

        // Determine trade quantity - take minimum of the buyers/sellers
        uint32_t tradeQuantity = std::min(buyOrder.quantity, sellOrder.quantity);

        if (traderInfo) {
            traders[buyOrder.traderID].bought(tradeQuantity, trade_price);
            traders[sellOrder.traderID].sold(tradeQuantity, trade_price);
        }

        // Update quantities
        buyOrder.quantity -= tradeQuantity;
        sellOrder.quantity -= tradeQuantity;

        tradesCompleted++;

        // Update median data
        if (median) {
            medianPQ[stockID].insert(trade_price);
        }

        // Verbose output
        if (verbose) {
            std::cout << "Trader " << buyOrder.traderID << " purchased "
                      << tradeQuantity << " shares of Stock " << stockID
                      << " from Trader " << sellOrder.traderID << " for $"
                      << trade_price << "/share\n";
        }

        // Remove the order from the PQ if no more quantity
        if (buyOrder.quantity == 0) buyPQ.pop();
        if (sellOrder.quantity == 0) sellPQ.pop();

        // other option... but I think ^^ is faster and just as accurate
        // buyPQ.pop();
        // sellPQ.pop();

        // if (buyOrder.quantity > 0) {
        //     buyPQ.push(buyOrder);
        // }
        // if (sellOrder.quantity > 0) {
        //     sellPQ.push(sellOrder);
        // }

    } // while
} // match_orders

// Output median prices
void Market::outputMedianPrices(uint32_t time) {
    for (uint32_t i = 0; i < numStocks; i++) {
        MedianPriorityQueue curr = medianPQ[i];
        uint32_t value = curr.getMedian();

        if (value == UINT32_MAX) continue; // if no median, then continue looping

        std::cout << "Median match price of Stock " << i << " at time " 
                  << time << " is $" << curr.getMedian() << "\n";
        curr.clear();
    } 
} // outputMedianPrices

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
        if (curr_stock.mode == 'n') return;

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
} //printTTinfo



// ----------------------------------------------------------------------- //
//                 MedianPriorityQueue Implementations                    //
// --------------------------------------------------------------------- //


void MedianPriorityQueue::balanceHeaps() {
    if (maxHeap.size() > minHeap.size() + 1) {
        minHeap.push(maxHeap.top());
        maxHeap.pop();
    } else if (minHeap.size() > maxHeap.size() + 1) {
        maxHeap.push(minHeap.top());
        minHeap.pop();
    }
} // fastMedian - balanceHeaps

void MedianPriorityQueue::insert(uint32_t num) {
    if (maxHeap.empty() || num < maxHeap.top()) {
        maxHeap.push(num);
    } else {
        minHeap.push(num);
    }
        balanceHeaps();
} // fastMedian - insert

uint32_t MedianPriorityQueue::getMedian() {
    // return UINT32_MAX if no median exists
    if (maxHeap.empty() && minHeap.empty()) return UINT32_MAX;

    if (maxHeap.size() == minHeap.size()) {
        return (maxHeap.top() + minHeap.top()) / 2;
    } else if (maxHeap.size() > minHeap.size()) {
        return maxHeap.top();
    } else {
        return minHeap.top();
    }
} // fastMedian - getMedian

void MedianPriorityQueue::clear() {
    while (!maxHeap.empty()) {
        maxHeap.pop();
    }
    while (!minHeap.empty()) {
        minHeap.pop();
    }
    return;
} // fastmedian - clear()

#endif // MARKET_HPP