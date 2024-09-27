#include <vector>
#include <queue>


struct Order {
    int timestamp;
    int traderID;
    int stockID;
    bool isBuy;      // true for buy, false for sell
    int price;
    int quantity;
    int arrivalOrder; // For tie-breaker based on arrival

    // Constructor
    Order(int ts, int tID, int sID, bool buy, int p, int q, int aOrder)
        : timestamp(ts), traderID(tID), stockID(sID), isBuy(buy), price(p), quantity(q), arrivalOrder(aOrder) {}
};


struct BuyOrderComparator {
    bool operator()(const Order& a, const Order& b) {
        if (a.price == b.price) {
            return a.arrivalOrder > b.arrivalOrder; // Earlier arrival has higher priority
        }
        return a.price < b.price; // Higher price has higher priority
    }
};

struct SellOrderComparator {
    bool operator()(const Order& a, const Order& b) {
        if (a.price == b.price) {
            return a.arrivalOrder > b.arrivalOrder;
        }
        return a.price > b.price; // Lower price has higher priority
    }
};

std::vector<std::priority_queue<Order, std::vector<Order>, BuyOrderComparator>> buyOrders;
std::vector<std::priority_queue<Order, std::vector<Order>, SellOrderComparator>> sellOrders;
