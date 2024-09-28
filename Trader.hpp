// Project Identifier: 0E04A31E0D60C01986ACB20081C9D8722A1899B6
#ifndef TRADER_HPP
#define TRADER_HPP
#pragma once
#include <cstdint>


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

#endif // TRADER_HPP