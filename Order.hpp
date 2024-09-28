// Project Identifier: 0E04A31E0D60C01986ACB20081C9D8722A1899B6
#ifndef ORDER_HPP
#define ORDER_HPP
#pragma once
#include <string>


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

#endif // ORDER_HPP
