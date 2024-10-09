// Project Identifier: 0E04A31E0D60C01986ACB20081C9D8722A1899B6
#include <getopt.h>
#include <iostream>
#include <sstream>
#include "Market.h"
#include "P2random.h" // Include the pseudorandom generator header


  // Option definitions for getopt_long
    static struct option long_options[] = {
        {"verbose", no_argument, nullptr, 'v'},
        {"median", no_argument, nullptr, 'm'},
        {"trader_info", no_argument, nullptr, 'i'},
        {"time_travelers", no_argument, nullptr, 't'},
        {nullptr, 0, nullptr, 0}
    };

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    bool verbose = false;
    bool median = false;
    bool traderInfo = false;
    bool timeTravelers = false;
    int gotopt;

    // Parse options using getopt_long
    while ((gotopt = getopt_long(argc, argv, "vmit", long_options, nullptr)) != -1) {
        switch (gotopt) {
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
    std::cout << "Processing orders...\n"; // print before we begin our reads

    // prelim cin for our info
    std::string comment = "";
    std::string in_mode = "";
    uint32_t traders = 0;
    uint32_t stocks = 0;

    std::getline(std::cin, comment); // gets rid of comment

    std::cin >> comment >> in_mode; // use comment to get the "Mode: "

    std::cin >> comment >> traders >> comment >> stocks; // reads number of traders and stocks
    std::stringstream ss; // creates a stream if we have PR mode

    // create an instance of Market Class, "market"
    Market market(stocks, traders, verbose, median, traderInfo, timeTravelers);

    if (in_mode == "TL") { // process the rest of our cin stream
        // ss << std::cin.rdbuf();  // Read all data from std::cin into the stringstream
        market.process_input_TL();
    } else if (in_mode == "PR") { // proccess PR mode
        std::string aux = ""; // used as junk read for preceeding symbols & words
        uint32_t seed = 0;
        uint32_t orders = 0;
        uint32_t a_rate = 0;
        std::cin >> aux >> seed >> aux >> orders >> aux >> a_rate;
        P2random::PR_init(ss, seed, traders, stocks, orders, a_rate);
        market.process_input_PR(ss);
    } else { // neither mode
        std::cerr << "Neither Input Mode Read\n";
        exit(1);
    }

    market.printEndOfDaySummary(); 

    // print Info/TT iff the mode is specified
    if (traderInfo) market.printTraderInfo();
    if (timeTravelers) market.printTimeTravelerInfo();

    return 0;
}