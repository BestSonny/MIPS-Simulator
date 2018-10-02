/* On my honor, I have neither given nor received unauthorized aid on this assignment */
//
//  simulator.hpp
//
//  Created by Pan He on 9/27/2018
//  CDA 5155: Computer Architecture Principles Fall 2018 (MIPS Simulator)
//  Copyright (c) 2018. All rights reserved.

#ifndef SIMULATOR_HPP_
#define SIMULATOR_HPP_

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include "utils.hpp"
#include <bitset>
using namespace std;

class Simulator{

public:
  int currentInstructionNumber;
  Node* root;
  Node* currentNode;
  ofstream disassemblyFile; // A variable to hold the disassembly write-to file
  ofstream simFile; // A variable to hold the simulation write-to file
  vector<string> decodedInstructions; // A vector to hold our decoded instructions, to be read later to perform operations
  vector<int> registerValues; // A vector to hold the register values
  vector<string> instructions; // A vector to hold our decoded instructions, to be read later to perform operations
  string currentLineOrInstruction, opcode, operation, previous_operation, output, decodedOutput;
  vector<string>::iterator iter;
  uint32_t hi;
	uint32_t lo;
public:
  Simulator(vector<string> instructions){
    this->currentInstructionNumber = 256;
    this->root = new Node();
    this->currentNode = root;
    this->disassemblyFile.open("sample_disassembly.txt");
    this->simFile.open("sample_simulation.txt");
    this->instructions = instructions;
    this->iter = this->instructions.begin();
    this->hi = 0;
    this->lo = 0;
    this->decodedInstructions.clear();
  }
	~Simulator(){
    ;
  }

public:

	/// Run the program
	void run(){

    do { // Loop through the vector
        this->currentLineOrInstruction = *this->iter; // Save the current line to our variable
        decode(); //decode
        step(); // Increment the instruction number
    } while(fetch()); //quit if the end of the instructions

    this->disassemblyFile << this->output; // Write to the dissasembly.txt file
    this->disassemblyFile.close();
    simulate();
    this->simFile.close();
  }

  // Decodes the instruction into its parts
	void decode(){
    string categoryNumber = this->currentLineOrInstruction.substr(0,3); // Get the current instruction's category number
    if(this->previous_operation != "BREAK") {
        if(categoryNumber == "000") { // The instruction is a category 1
          decode_category_one();
        } else if(categoryNumber == "001") { // The instruction is a category 2
          decode_category_two();
        } else if(categoryNumber == "010") { // The instruction is a category 3
          decode_category_three();
        } else if(categoryNumber == "011") { // The instruction is a category 4
          decode_category_four();
        } else if(categoryNumber == "100") { // The instruction is a category 5
          decode_category_five();
        }
        this->previous_operation = operation;
    } else { // The previous operation was BREAK, so we are now reading in the registers' values

        string bvs = this->currentLineOrInstruction; // Get the binary value of the number
        int32_t registerValue = convertStringToInt32(bvs);
        // registerValue = decimal;

        registerValues.push_back(registerValue); // Store the just-computed register value into the registerValues vector for later access

        output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + to_string(registerValue) + "\n");

    }
    decodedInstructions.push_back(this->decodedOutput); // Save our decoded instructions to the decodedInstructions vector
  }

protected:

  void simulate(){
    int inNu = 256; // Instruction number of where the register values start
    string previousInstructionString = "";
    for(vector<string>::iterator iter_current = decodedInstructions.begin(); iter_current != decodedInstructions.end() && previousInstructionString != "BREAK"; iter_current++) {
        previousInstructionString = *iter_current;
        inNu = inNu + 4;
    }

    vector<int> modRegisterValues;

    // Fill the modRegistersValues vector with 0's for every register that exists
    for(int i=0; i<=31; i++) {
        modRegisterValues.push_back(0);
    }

    // Set the starting node (instruction)
    currentNode = root->next;

    if(currentNode->next != nullptr) {
        int cycle = 1;
        while(currentNode != nullptr) {

            string currentOperation = currentNode->operation; // Get the current instruction's operation

            int currentInstructionNumber = currentNode->instructionNumber; // Get the current instruction number
            output = "Cycle " + to_string(cycle) + ":\t" + to_string(currentInstructionNumber) + "\t"; // Create the output string
            // cout << output << endl;
            // cout << cycle << endl;
            if(currentOperation == "J") {

                string instructionLocation = currentNode->location;
                int iil = stoi(instructionLocation); // Instruction location in integer form

                Node* startingNode = root;
                while(startingNode->next->instructionNumber != iil) { // Find the jump location
                    startingNode = startingNode->next;
                }
                currentNode = startingNode; // Set the current node to the jump location

                output.append("J #" + instructionLocation + "\n\n");

            } else if(currentOperation == "BEQ") {

                string offset = currentNode->offset;
                int io = stoi(offset); // offset in integer form
                string rtReg = currentNode->rt;
                int irt = modRegisterValues[stoi(rtReg)]; // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = modRegisterValues[stoi(rsReg)]; // rs register in integer form

                int nextInstructionNumber = currentNode->instructionNumber + io; // Get the next instruction number (to be used if rt == rs)

                if(irt == irs) {
                    Node* startingNode = root;
                    while(startingNode->next->instructionNumber != nextInstructionNumber) { // Find the instruction at the offsetted location
                        startingNode = startingNode->next;
                    }
                    currentNode = startingNode->next; // Set the current node to the offsetted location
                }

                output.append("BEQ R" + rsReg + ", R" + rtReg + ", #" + offset + "\n\n");

            } else if(currentOperation == "BNE") {
              ////////////sdadadasdas
              string offset = currentNode->offset;
              int io = stoi(offset); // offset in integer form
              string rtReg = currentNode->rt;
              int irt = modRegisterValues[stoi(rtReg)]; // rt register in integer form
              string rsReg = currentNode->rs;
              int irs = modRegisterValues[stoi(rsReg)]; // rs register in integer form

              int nextInstructionNumber = currentNode->instructionNumber + io; // Get the next instruction number (to be used if rt == rs)

              if(irt != irs) {
                  Node* startingNode = root;
                  while(startingNode->next->instructionNumber != nextInstructionNumber) { // Find the instruction at the offsetted location
                      startingNode = startingNode->next;
                  }
                  currentNode = startingNode->next; // Set the current node to the offsetted location
              }

              output.append("BNE R" + rsReg + ", R" + rtReg + ", #" + offset + "\n\n");

            } else if(currentOperation == "BGTZ") {

                string offset = currentNode->offset;
                int io = stoi(offset); // offset in integer form
                string rsReg = currentNode->rs;
                int irs = modRegisterValues[stoi(rsReg)]; // rs register in integer form

                int nextInstructionNumber = currentNode->instructionNumber + io; // Get the next instruction number (to be used if rs > 0)

                if(irs > 0) {
                    Node* startingNode = root;
                    while(startingNode->next->instructionNumber != nextInstructionNumber) { // Find the instruction at the offsetted location
                        startingNode = startingNode->next;
                    }
                    currentNode = startingNode->next; // Set the current node to the offsetted location
                }

                output.append("BGTZ R" + rsReg + ", #" + offset + "\n\n");

            } else if(currentOperation == "BREAK") {

                output.append("BREAK\n\n");

            } else if(currentOperation == "SW") {

                string baseAddress = currentNode->baseAddress;
                int iba = stoi(baseAddress); // base address in integer form
                string offset = currentNode->offset;
                int io = stoi(offset); // offset in integer form
                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form

                // Store the value from the local register specified into the memory register specified
                registerValues[(io - inNu + modRegisterValues[iba])/4] = modRegisterValues[irt];

                output.append("SW R" + rtReg + ", " + offset + "(R" + baseAddress + ")\n\n");

            } else if(currentOperation == "LW") {

                string baseAddress = currentNode->baseAddress;
                int iba = stoi(baseAddress); // base address in integer form
                string offset = currentNode->offset;
                int io = stoi(offset); // offset in integer form
                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form

                // Store the value from the memory register specified into the local register specified
                modRegisterValues[irt] = registerValues[(io - inNu + modRegisterValues[iba])/4];

                output.append("LW R" + rtReg + ", " + offset + "(R" + baseAddress + ")\n\n");

            } else if(currentOperation == "ADD") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                string rdReg = currentNode->rd;
                int ird = stoi(rdReg); // rd register in integer form

                modRegisterValues[ird] = modRegisterValues[irs] + modRegisterValues[irt]; // Compute the sum rs + rt

                output.append("ADD R" + rdReg + ", R" + rsReg + ", R" + rtReg + "\n\n");

            } else if(currentOperation == "SUB") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                string rdReg = currentNode->rd;
                int ird = stoi(rdReg); // rd register in integer form

                modRegisterValues[ird] = modRegisterValues[irs] - modRegisterValues[irt]; // Compute the difference rs - rt

                output.append("SUB R" + rdReg + ", R" + rsReg + ", R" + rtReg + "\n\n");

            } else if(currentOperation == "AND") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                string rdReg = currentNode->rd;
                int ird = stoi(rdReg); // rd register in integer form

                modRegisterValues[ird] = modRegisterValues[irt]&modRegisterValues[irs]; // Compute the bitwise AND of registers rt and rs

                output.append("AND R" + rdReg + ", R" + rsReg + ", R" + rtReg + "\n\n");

            } else if(currentOperation == "OR") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                string rdReg = currentNode->rd;
                int ird = stoi(rdReg); // rd register in integer form

                modRegisterValues[ird] = modRegisterValues[irt]|modRegisterValues[irs]; // Compute the bitwise OR of registers rt and rs

                output.append("OR R" + rdReg + ", R" + rsReg + ", R" + rtReg + "\n\n");

            } else if(currentOperation == "SRL") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                string rdReg = currentNode->rd;
                int ird = stoi(rdReg); // rd register in integer form

                modRegisterValues[ird] = (unsigned int)modRegisterValues[irs]>>irt; // Compute the bitwise OR of registers rt and rs

                output.append("SRL R" + rdReg + ", R" + rsReg + ", #" + rtReg + "\n\n");

            } else if(currentOperation == "SRA") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                string rdReg = currentNode->rd;
                int ird = stoi(rdReg); // rd register in integer form
                modRegisterValues[ird] = modRegisterValues[irs]>>irt; // Compute the bitwise OR of registers rt and rs

                output.append("SRA R" + rdReg + ", R" + rsReg + ", #" + rtReg + "\n\n");

            } else if(currentOperation == "ADDI") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                string immediate = currentNode->immediate;
                int io = stoi(immediate); // immediate value in integer form

                modRegisterValues[irt] = modRegisterValues[irs] + io;

                output.append("ADDI R" + rtReg + ", R" + rsReg + ", #" + immediate + "\n\n");

            } else if(currentOperation == "ANDI") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                string immediate = currentNode->immediate;
                int io = stoi(immediate); // immediate value in integer form

                modRegisterValues[irt] = modRegisterValues[irs]&io;

                output.append("ANDI R" + rtReg + ", R" + rsReg + ", #" + immediate + "\n\n");

            } else if(currentOperation == "ORI") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                string immediate = currentNode->immediate;
                int io = stoi(immediate); // immediate value in integer form
                // cout << std::bitset<32>(lo) << " " << currentNode->immediate << " " << io << endl;
                modRegisterValues[irt] = modRegisterValues[irs]|io;

                output.append("ORI R" + rtReg + ", R" + rsReg + ", #" + immediate + "\n\n");
            } else if(currentOperation == "MULT") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form
                int64_t result = (int64_t)((int32_t)modRegisterValues[irs]) * (int64_t)((int32_t)modRegisterValues[irt]);
                this->lo = (int32_t)(result & 0xFFFFFFFF);
                this->hi = (int32_t)((result & 0xFFFFFFFF00000000) >> 32);

                output.append("MULT R" + rtReg + ", R" + rsReg + "\n\n");
            } else if(currentOperation == "DIV") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form
                string rsReg = currentNode->rs;
                int irs = stoi(rsReg); // rs register in integer form

                if (modRegisterValues[irs] != 0){
          				// Unsure about whether the remainder should be -ve or +ve when dividing a -ve number: This gives a -ve remainder
          				int32_t remainder = (int32_t)modRegisterValues[irt] % (int32_t)modRegisterValues[irs];
          				int32_t quotient = (int32_t)modRegisterValues[irt] / (int32_t)modRegisterValues[irs];

          				this->lo = (uint32_t)quotient;
          				this->hi = (uint32_t)remainder;
          			} else{
          				this->lo = 0;
          				this->hi = 0;
          			}

                output.append("DIV R" + rtReg + ", R" + rsReg + "\n\n");
            } else if(currentOperation == "MFHI") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form

                modRegisterValues[irt] = this->hi;

                output.append("MFHI R" + rtReg + "\n\n");
            } else if(currentOperation == "MFLO") {

                string rtReg = currentNode->rt;
                int irt = stoi(rtReg); // rt register in integer form

                modRegisterValues[irt] = this->lo;

                output.append("MFLO R" + rtReg + "\n\n");
            }

            // Create the "Registers" section
            string registersSection = "Registers\n";
            registersSection.append("R00:\t" + to_string(modRegisterValues[0]) + "\t" + to_string(modRegisterValues[1]) + "\t");
            registersSection.append(to_string(modRegisterValues[2]) + "\t" + to_string(modRegisterValues[3]) + "\t");
            registersSection.append(to_string(modRegisterValues[4]) + "\t" + to_string(modRegisterValues[5]) + "\t");
            registersSection.append(to_string(modRegisterValues[6]) + "\t" + to_string(modRegisterValues[7]) + "\n");
            registersSection.append("R08:\t" + to_string(modRegisterValues[8]) + "\t" + to_string(modRegisterValues[9]) + "\t");
            registersSection.append(to_string(modRegisterValues[10]) + "\t" + to_string(modRegisterValues[11]) + "\t");
            registersSection.append(to_string(modRegisterValues[12]) + "\t" + to_string(modRegisterValues[13]) + "\t");
            registersSection.append(to_string(modRegisterValues[14]) + "\t" + to_string(modRegisterValues[15]) + "\n");
            registersSection.append("R16:\t" + to_string(modRegisterValues[16]) + "\t" + to_string(modRegisterValues[17]) + "\t");
            registersSection.append(to_string(modRegisterValues[18]) + "\t" + to_string(modRegisterValues[19]) + "\t");\
            registersSection.append(to_string(modRegisterValues[20]) + "\t" + to_string(modRegisterValues[21]) + "\t");
            registersSection.append(to_string(modRegisterValues[22]) + "\t" + to_string(modRegisterValues[23]) + "\n");
            registersSection.append("R24:\t" + to_string(modRegisterValues[24]) + "\t" + to_string(modRegisterValues[25]) + "\t");
            registersSection.append(to_string(modRegisterValues[26]) + "\t" + to_string(modRegisterValues[27]) + "\t");\
            registersSection.append(to_string(modRegisterValues[28]) + "\t" + to_string(modRegisterValues[29]) + "\t");
            registersSection.append(to_string(modRegisterValues[30]) + "\t" + to_string(modRegisterValues[31]) + "\n\n");

            // Create the "Data" section
            string dataSection = "Data\n";
            dataSection.append(to_string(inNu) + ":\t" + to_string(registerValues[0]) + "\t" + to_string(registerValues[1]) + "\t");
            dataSection.append(to_string(registerValues[2]) + "\t" + to_string(registerValues[3]) + "\t");
            dataSection.append(to_string(registerValues[4]) + "\t" + to_string(registerValues[5]) + "\t");
            dataSection.append(to_string(registerValues[6]) + "\t" + to_string(registerValues[7]) + "\n");
            dataSection.append(to_string(inNu + 32) + ":\t" + to_string(registerValues[8]) + "\t" + to_string(registerValues[9]) + "\t");
            dataSection.append(to_string(registerValues[10]) + "\t" + to_string(registerValues[11]) + "\t");
            dataSection.append(to_string(registerValues[12]) + "\t" + to_string(registerValues[13]) + "\t");
            if(currentOperation == "BREAK") {
                dataSection.append(to_string(registerValues[14]) + "\t" + to_string(registerValues[15]) + "\n");
            } else {
                dataSection.append(to_string(registerValues[14]) + "\t" + to_string(registerValues[15]) + "\n");
            }

            // Print the output
            this->simFile << "--------------------\n";
            this->simFile << output;
            this->simFile << registersSection;
            this->simFile << dataSection;
            currentNode = currentNode->next;
            ++cycle;
        }
    }
  }

	/// Fetches the next instruction
	bool fetch(){
    this->iter ++;
    return this->iter != this->instructions.end();
  }

  void decode_category_one(){
    opcode = this->currentLineOrInstruction.substr(3,3); // Get the current instruction's opcode

    if(opcode == "000") { // J instruction

        operation = "J";

        int il = stoi(this->currentLineOrInstruction.substr(6, 26));
        int decimal = convertBinaryToDecimal(il);
        string il_naked = to_string(4*decimal);
        string il_printable = "#" + il_naked;

        this->decodedOutput = "J " + il_printable;
        output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

        Node* tempNode = new Node(this->currentInstructionNumber, "J", "", il_naked, "", "", "", "", "", nullptr);
        if(currentNode != nullptr) {
            while(currentNode->next != nullptr) {
                currentNode = currentNode->next;
            }
        }
        currentNode->next = tempNode;
        currentNode = currentNode->next;

    } else if(opcode == "001") { // BEQ instruction

        operation = "BEQ";

        int rs = stoi(this->currentLineOrInstruction.substr(6,5));
        int decimal = convertBinaryToDecimal(rs);
        string rs_naked = to_string(decimal);
        string rs_printable = "R" + rs_naked;

        int rt = stoi(this->currentLineOrInstruction.substr(11,5));
        decimal = convertBinaryToDecimal(rt);
        string rt_naked = to_string(decimal);
        string rt_printable = "R" + rt_naked;

        decimal = convertStringToInt16(this->currentLineOrInstruction.substr(16,16));
        // decimal = convertBinaryToDecimal(of);
        string of_naked = to_string(4*decimal);
        string of_printable = "#" + of_naked;

        this->decodedOutput = "BEQ " + rs_printable + ", " + rt_printable + ", " + of_printable;
        output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

        Node* tempNode = new Node(this->currentInstructionNumber, "BEQ", "", "", of_naked, "", rt_naked, "", rs_naked, nullptr);
        if(currentNode != nullptr) {
            while(currentNode->next != nullptr) {
                currentNode = currentNode->next;
            }
        }
        currentNode->next = tempNode;
        currentNode = currentNode->next;

    } else if(opcode == "010") { // BEQ instruction

        operation = "BNE";

        int rs = stoi(this->currentLineOrInstruction.substr(6,5));
        int decimal = convertBinaryToDecimal(rs);
        string rs_naked = to_string(decimal);
        string rs_printable = "R" + rs_naked;

        int rt = stoi(this->currentLineOrInstruction.substr(11,5));
        decimal = convertBinaryToDecimal(rt);
        string rt_naked = to_string(decimal);
        string rt_printable = "R" + rt_naked;

        decimal = convertStringToInt16(this->currentLineOrInstruction.substr(16,16));
        // decimal = convertBinaryToDecimal(of);
        string of_naked = to_string(4*decimal);
        string of_printable = "#" + of_naked;

        this->decodedOutput = "BNE " + rs_printable + ", " + rt_printable + ", " + of_printable;
        output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

        Node* tempNode = new Node(this->currentInstructionNumber, "BNE", "", "", of_naked, "", rt_naked, "", rs_naked, nullptr);
        if(currentNode != nullptr) {
            while(currentNode->next != nullptr) {
                currentNode = currentNode->next;
            }
        }
        currentNode->next = tempNode;
        currentNode = currentNode->next;

    } else if(opcode == "011") { // BGTZ instruction

        operation = "BGTZ";

        int rs = stoi(this->currentLineOrInstruction.substr(6,5));
        int decimal = convertBinaryToDecimal(rs);
        string rs_naked = to_string(decimal);
        string rs_printable = "R" + rs_naked;

        decimal = convertStringToInt16(this->currentLineOrInstruction.substr(16,16));
        // decimal = convertBinaryToDecimal(of);
        string of_naked = to_string(4*decimal);
        string of_printable = "#" + of_naked;

        this->decodedOutput = "BGTZ " + rs_printable + ", " + of_printable;
        output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

        Node* tempNode = new Node(this->currentInstructionNumber, "BGTZ", "", "", of_naked, "", "", "", rs_naked, nullptr);
        if(currentNode != nullptr) {
            while(currentNode->next != nullptr) {
                currentNode = currentNode->next;
            }
        }
        currentNode->next = tempNode;
        currentNode = currentNode->next;

    } else if(opcode == "100") { // SW instruction

        operation = "SW";

        int br = stoi(this->currentLineOrInstruction.substr(6,5));
        int decimal = convertBinaryToDecimal(br);
        string br_naked = to_string(decimal);
        string br_printable = "R" + br_naked;

        int rt = stoi(this->currentLineOrInstruction.substr(11,5));
        decimal = convertBinaryToDecimal(rt);
        string rt_naked = to_string(decimal);
        string rt_printable = "R" + rt_naked;

        decimal = convertStringToInt16(this->currentLineOrInstruction.substr(16,16));
        // decimal = convertBinaryToDecimal(of);
        string of_naked = to_string(decimal);
        string of_printable = of_naked;

        this->decodedOutput = "SW " + rt_printable + ", " + of_printable + "(" + br_printable + ")";
        output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

        Node* tempNode = new Node(this->currentInstructionNumber, "SW", br_naked, "", of_naked, "", rt_naked, "", "", nullptr);
        if(currentNode != nullptr) {
            while(currentNode->next != nullptr) {
                currentNode = currentNode->next;
            }
        }
        currentNode->next = tempNode;
        currentNode = currentNode->next;

    } else if(opcode == "101") { // LW instruction

        operation = "LW";

        int br = stoi(this->currentLineOrInstruction.substr(6,5));
        int decimal = convertBinaryToDecimal(br);
        string br_naked = to_string(decimal);
        string br_printable = "R" + br_naked;

        int rt = stoi(this->currentLineOrInstruction.substr(11,5));
        decimal = convertBinaryToDecimal(rt);
        string rt_naked = to_string(decimal);
        string rt_printable = "R" + rt_naked;

        decimal = convertStringToInt16(this->currentLineOrInstruction.substr(16,16));
        // decimal = convertBinaryToDecimal(of);
        string of_naked = to_string(decimal);
        string of_printable = of_naked;

        this->decodedOutput = "LW " + rt_printable + ", " + of_printable + "(" + br_printable + ")";
        output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

        Node* tempNode = new Node(this->currentInstructionNumber, "LW", br_naked, "", of_naked, "", rt_naked, "", "", nullptr);
        if(currentNode != nullptr) {
            while(currentNode->next != nullptr) {
                currentNode = currentNode->next;
            }
        }
        currentNode->next = tempNode;
        currentNode = currentNode->next;
    } else if(opcode == "110") { // BREAK instruction

        operation = "BREAK";

        this->decodedOutput = "BREAK";
        output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

        Node* tempNode = new Node(this->currentInstructionNumber, "BREAK", "", "", "", "", "", "", "", nullptr);
        if(currentNode != nullptr) {
            while(currentNode->next != nullptr) {
                currentNode = currentNode->next;
            }
        }
        currentNode->next = tempNode;
        currentNode = currentNode->next;

    }
  }
  void decode_category_two(){

    opcode = this->currentLineOrInstruction.substr(3,3); // Get the current instruction's opcode

    if(opcode == "000") {
        operation = "ADD";
    } else if(opcode == "001") {
        operation = "SUB";
    } else if(opcode == "010") {
        operation = "AND";
    } else if(opcode == "011") {
        operation = "OR";
    } else if(opcode == "100") {
        operation = "SRL";
    } else if(opcode == "101") {
        operation = "SRA";
    }

    int rs = stoi(this->currentLineOrInstruction.substr(11,5));
    int decimal = convertBinaryToDecimal(rs);
    string rs_naked = to_string(decimal);
    string rs_printable = "R" + rs_naked;

    int rt = stoi(this->currentLineOrInstruction.substr(16,5));
    decimal = convertBinaryToDecimal(rt);
    string rt_naked = to_string(decimal);
    string rt_printable = "R" + rt_naked;
    if(operation == "SRL" || operation == "SRA"){
      rt_printable = "#" + rt_naked;
    }

    int rd = stoi(this->currentLineOrInstruction.substr(6,5));
    decimal = convertBinaryToDecimal(rd);
    string rd_naked = to_string(decimal);
    string rd_printable = "R" + rd_naked;


    this->decodedOutput = operation + " " + rd_printable + ", " + rs_printable + ", " + rt_printable;
    output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");
    Node* tempNode = new Node(this->currentInstructionNumber, operation, "", "", "", "", rt_naked, rd_naked, rs_naked, nullptr);
    if(currentNode != nullptr) {
        while(currentNode->next != nullptr) {
            currentNode = currentNode->next;
        }
    }
    currentNode->next = tempNode;
    currentNode = currentNode->next;

  }
  void decode_category_three(){

    opcode = this->currentLineOrInstruction.substr(3,3); // Get the current instruction's opcode

    if(opcode == "000") {
        operation = "ADDI";
    } else if(opcode == "001") {
        operation = "ANDI";
    } else if(opcode == "010") {
        operation = "ORI";
    }

    int rt = stoi(this->currentLineOrInstruction.substr(6,5)); // rt register number in binary format
    int decimal = convertBinaryToDecimal(rt);
    string rt_naked = to_string(decimal);
    string rt_printable = "R" + rt_naked;

    int rs = stoi(this->currentLineOrInstruction.substr(11,5)); // rs register number in binary format
    decimal = convertBinaryToDecimal(rs);
    string rs_naked = to_string(decimal);
    string rs_printable = "R" + rs_naked;

    if(operation == "ANDI" ||  operation == "ORI"){
      decimal = convertStringToUnsignedInt16(this->currentLineOrInstruction.substr(16,16));
    }else{
      decimal = convertStringToInt16(this->currentLineOrInstruction.substr(16,16));
    }
    // cout << x << endl;
    // int iv = stoi(this->currentLineOrInstruction.substr(16,16)); // immediate value in binary format
    // decimal = convertBinaryToDecimal(iv);
    string iv_naked = to_string(decimal);
    string iv_printable = "#" + iv_naked; // Printable immediate value

    this->decodedOutput = operation + " " + rt_printable + ", " + rs_printable + ", " + iv_printable;
    output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

    Node* tempNode = new Node(this->currentInstructionNumber, operation, "", "", "", iv_naked, rt_naked, "", rs_naked, nullptr);
    if(currentNode != nullptr) {
        while(currentNode->next != nullptr) {
            currentNode = currentNode->next;
        }
    }
    currentNode->next = tempNode;
    currentNode = currentNode->next;

  }
  void decode_category_four(){

    opcode = this->currentLineOrInstruction.substr(3,3); // Get the current instruction's opcode

    if(opcode == "000") {
        operation = "MULT";
    } else if(opcode == "001") {
        operation = "DIV";
    }

    int rt = stoi(this->currentLineOrInstruction.substr(6,5)); // rt register number in binary format
    int decimal = convertBinaryToDecimal(rt);
    string rt_naked = to_string(decimal);
    string rt_printable = "R" + rt_naked;

    int rs = stoi(this->currentLineOrInstruction.substr(11,5)); // rs register number in binary format
    decimal = convertBinaryToDecimal(rs);
    string rs_naked = to_string(decimal);
    string rs_printable = "R" + rs_naked;

    this->decodedOutput = operation + " " + rt_printable + ", " + rs_printable;
    output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

    Node* tempNode = new Node(this->currentInstructionNumber, operation, "", "", "", "", rt_naked, "", rs_naked, nullptr);
    if(currentNode != nullptr) {
        while(currentNode->next != nullptr) {
            currentNode = currentNode->next;
        }
    }
    currentNode->next = tempNode;
    currentNode = currentNode->next;

  }
  void decode_category_five(){

    opcode = this->currentLineOrInstruction.substr(3,3); // Get the current instruction's opcode

    if(opcode == "000") {
        operation = "MFHI";
    } else if(opcode == "001") {
        operation = "MFLO";
    }

    int rt = stoi(this->currentLineOrInstruction.substr(6,5)); // rt register number in binary format
    int decimal = convertBinaryToDecimal(rt);
    string rt_naked = to_string(decimal);
    string rt_printable = "R" + rt_naked;

    this->decodedOutput = operation + " " + rt_printable;
    output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

    Node* tempNode = new Node(this->currentInstructionNumber, operation, "", "", "", "", rt_naked, "", "", nullptr);
    if(currentNode != nullptr) {
        while(currentNode->next != nullptr) {
            currentNode = currentNode->next;
        }
    }
    currentNode->next = tempNode;
    currentNode = currentNode->next;

  }

	/// Increases the program counter by 1 instruction
 void step(){
    this->currentInstructionNumber = this->currentInstructionNumber + 4;
  }

};

#endif /* SIMULATOR_HPP_ */
