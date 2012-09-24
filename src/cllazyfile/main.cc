#include "lazyInstMgr.h"
//     g++ main.cc -I. -o main

int main (int argc, char ** argv ) {
    std::string n;
    if( argc == 2 ) {
        n = argv[1];
    } else {
        n = "somefile.stp";
    }
    lazyInstMgr mgr;
    mgr.openFile( n );
}