// Project Identifier: 0E04A31E0D60C01986ACB20081C9D8722A1899B6
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <Market.hpp
#include "P2random.hpp" // Include the pseudorandom generator header


  // Option definitions for getopt_long
    static struct option long_options[] = {
        {"verbose", no_argument, nullptr, 'v'},
        {"median", no_argument, nullptr, 'm'},
        {"trader_info", no_argument, nullptr, 'i'},
        {"time_travelers", no_argument, nullptr, 't'},
        {nullptr, 0, nullptr, 0}
    };

int main(int argc, char *argv[]) {
    bool verbose = false;
    bool median = false;
    bool traderInfo = false;
    bool timeTravelers = false;

    // Parse options using getopt_long
    while ((opt = getopt_long(argc, argv, "vmit", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'v': 
                verbose = true; // verbose
                break;
            case 'm':
                median = true; // median
                break;
            case 'i':
                traderInfo = true; // trader_info
                break;
            case 't':
                timeTravelers = true; // time_travelers
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-v] [-m] [-i] [-t]\n";
                exit(1);
        } // switch
    } // while

    //  -------------------------------------------------------------- //
    //                end getopts... DRIVER CODE HERE                 //
    //  ------------------------------------------------------------ //

    // prelim cin for our info
    std::string line = "";
    std::string = "";
    int traders = 0;
    int stocks = 0;

    std::getline(cin, line); // gets rid of comment
    std::getline(cin, line); // reads mode
    cin >> traders >> stocks; // reads number of traders and stocks
    std::istringstream ss; // creates a stream if we have PR mode

    // create an instance of Market Class, "market"
    Market market(stocks, traders, verbose, median, traderInfo, timeTravelers);

    if (line == "TL") { // process the rest of our cin stream
        market.proccess_input(cin, mode);
        return;
    } else if (line == "PR") { // proccess PR mode
        std::string aux = ""; // used as junk read for preceeding symbols & words
        size_t seed = 0;
        size_t orders = 0;
        size_t a_rate = 0;
        cin >> aux >> seed >> aux >> orders >> aux >> a_rate;
        P2random::PR_init(ss, seed, traders, stocks, orders, a_rate);
        market.process_input(ss, mode);
    } else { // neither mode
        std::cerr << "Neither Input Mode Read\n";
        exit(1);
    }

    market.printEndOfDaySummary(); // check ordering for summary!!!

    if (traderInfo) {
        market.printTraderInfo();
    }
    if (timeTravelers) {
        market.printTimeTravelerInfo();
    }

    return 0;
}