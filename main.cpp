#include <iostream> 
#include <getopt.h>

void parseCommandLine(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"median", no_argument, 0, 'm'},
        {"trader_info", no_argument, 0, 'i'},
        {"time_travelers", no_argument, 0, 't'},
        {0, 0, 0, 0}
    };



int main () {
  bool verbose = false;
  bool median = false;
  bool traderInfo = false;
  bool timeTravelers = false;

  
  int opt;
    while ((opt = getopt_long(argc, argv, "vmit", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 'm':
                median = true;
                break;
            case 'i':
                traderInfo = true;
                break;
            case 't':
                timeTravelers = true;
                break;
            default:
                // Handle unknown options if necessary
                break;
        }
    }
  
  return 0;
}
