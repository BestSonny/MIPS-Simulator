#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

struct Node {
    int instructionNumber;
    string operation, baseAddress, location, offset, immediate, rt, rd, rs;
    Node* next;
    Node() {
        instructionNumber = 0;
        operation = "";
        baseAddress = "";
        location = "";
        offset = "";
        immediate = "";
        rt = "";
        rd = "";
        rs = "";
        next = nullptr;
    }
    Node(int in, string op, string ba, string lo, string of, string im, string rtr, string rdr, string rsr, Node* ne) {
        instructionNumber = in;
        operation = op;
        baseAddress = ba;
        location = lo;
        offset = of;
        immediate = im;
        rt = rtr;
        rd = rdr;
        rs = rsr;
        next = ne;
    }
    ~Node() {
        free(next);
    }
};


int convertBinaryToDecimal(int n)
{
    int decimalNumber = 0, i = 0, remainder;
    while (n!=0)
    {
        remainder = n%10;
        n /= 10;
        decimalNumber += remainder*pow(2,i);
        ++i;
    }
    return decimalNumber;
}

#endif
