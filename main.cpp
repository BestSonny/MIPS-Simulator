/* On my honor, I have neither given nor received unauthorized aid on this assignment */
//
//  main.cpp
//
//  Created by Pan He on 9/27/2018
//  CDA 5155: Computer Architecture Principles Fall 2018 (MIPS Simulator)
//  Copyright (c) 2018. All rights reserved.

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "simulator.hpp"
using namespace std;

int main(int argc, const char* argv[])
{

    ifstream File(argv[1]);

    vector<string> instructions; // A vector to hold all the instructions in the file
    string currentLine;

    while (getline(File, currentLine)) {
        instructions.push_back(currentLine.substr(0, 32));
    }

    Simulator* mySimulator = new Simulator(instructions);
    mySimulator->run();

    return 0;
}
