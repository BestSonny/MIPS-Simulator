/* On my honor, I have neither given nor received unauthorized aid on this assignment */
//
//  simulator.hpp
//
//  Created by Pan He on 9/27/2018
//  CDA 5155: Computer Architecture Principles Fall 2018 (MIPS Pipeline)
//  Copyright (c) 2018. All rights reserved.

#ifndef PIPELINE_HPP_
#define PIPELINE_HPP_

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include "utils.hpp"
#include <bitset>
#include <queue>
using namespace std;

#define BUF_1_SIZE 8
#define BUF_2_SIZE 2
#define BUF_3_SIZE 2
#define BUF_4_SIZE 2
#define BUF_5_SIZE 2
#define BUF_6_SIZE 1
#define BUF_7_SIZE 1
#define BUF_8_SIZE 1
#define BUF_9_SIZE 1
#define BUF_10_SIZE 1
#define BUF_11_SIZE 1
#define BUF_12_SIZE 1

#define FETCH_DECODE_PER_CYCLE 4
#define ISSUE_DECODE_PER_CYCLE 8


class Pipeline{

public:
  int currentInstructionNumber;
  Node* root;
  Node* currentNode;
  ofstream disassemblyFile; // A variable to hold the disassembly write-to file
  ofstream simFile; // A variable to hold the simulation write-to file
  vector<string> decodedInstructions; // A vector to hold our decoded instructions, to be read later to perform operations
  vector<int> DataValues; // A vector to hold the register values
  vector<string> instructions; // A vector to hold our decoded instructions, to be read later to perform operations
  string currentLineOrInstruction, opcode, operation, previous_operation, output, decodedOutput;
  vector<string>::iterator iter;
  uint32_t hi;
	uint32_t lo;

  vector<int> Buf1; // Pre-Issue buffer (Buf1)
  vector<int> Buf2;
  vector<int> Buf3;
  vector<int> Buf4;
  vector<int> Buf5;
  int Buf6[3];
  uint32_t Buf7[3];
  int Buf8[3];
  int Buf9[3];
  int Buf10[3];//0 - instr, 1- destination reg, 2 - result/source/mem dest
  int Buf11[3];
  int32_t Buf12[3];


  vector<int> Buf1_Update; // Pre-Issue buffer (Buf1)
  vector<int> Buf2_Update;
  vector<int> Buf3_Update;
  vector<int> Buf4_Update;
  vector<int> Buf5_Update;
  int Buf6_Update[3];
  uint32_t Buf7_Update[3];
  int Buf8_Update[3];
  int Buf9_Update[3];
  int Buf10_Update[3];
  int Buf11_Update[3];
  int32_t Buf12_Update[3];

  int preIssueRead[34];
  int preIssueWrite[34];	//for checking harzeds within preIssueBuffer
  int regsInRead[34], regsInWrite[34]; //issue() records regs to read/write

  int waitInstr; //index of waitting instr
  int execInstr; //index of executing instr

  int cycle;
  int inNu;

  vector<int> modRegisterValues;

public:
  Pipeline(vector<string> instructions){
    this->currentInstructionNumber = 256;
    this->root = new Node();
    this->currentNode = root;
    this->disassemblyFile.open("sample_disassembly.txt");
    this->simFile.open("simulation.txt");
    this->instructions = instructions;
    this->iter = this->instructions.begin();
    this->hi = 0;
    this->lo = 0;
    this->decodedInstructions.clear();

    this->cycle = 1;

    std::fill(this->preIssueWrite, this->preIssueWrite+34, 0);//reset preIssue read/write table
  	std::fill(this->preIssueRead, this->preIssueRead+34, 0);
    std::fill(this->regsInRead, this->regsInRead+34, 0);
    std::fill(this->regsInWrite, this->regsInWrite+34, 0);

    std::fill(this->Buf6_Update, this->Buf6_Update+3, -1);
    std::fill(this->Buf7_Update, this->Buf7_Update+3, -1);
    std::fill(this->Buf8_Update, this->Buf8_Update+3, -1);
    std::fill(this->Buf9_Update, this->Buf9_Update+3, -1);
    std::fill(this->Buf10_Update, this->Buf10_Update+3, -1);
    std::fill(this->Buf11_Update, this->Buf11_Update+3, -1);
    std::fill(this->Buf12_Update, this->Buf12_Update+3, -1);


    std::fill(this->Buf6, this->Buf6+3, -1);
    std::fill(this->Buf7, this->Buf7+3, -1);
    std::fill(this->Buf8, this->Buf8+3, -1);
    std::fill(this->Buf9, this->Buf9+3, -1);
    std::fill(this->Buf10, this->Buf10+3, -1);
    std::fill(this->Buf11, this->Buf11+3, -1);
    std::fill(this->Buf12, this->Buf12+3, -1);



    this->waitInstr = -1; //index of waitting instr
    this->execInstr = -1; //index of executing instr
    // Fill the modRegistersValues vector with 0's for every register that exists
    for(int i=0; i<=31; i++) {
        modRegisterValues.push_back(0);
    }

  }
	~Pipeline(){
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
    // reset
    this->currentInstructionNumber = 256;
    pipeline();
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

        DataValues.push_back(registerValue); // Store the just-computed register value into the DataValues vector for later access

        output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + to_string(registerValue) + "\n");

    }
    decodedInstructions.push_back(this->decodedOutput); // Save our decoded instructions to the decodedInstructions vector
  }

protected:

  void display_lock(){
    string registersSection = "\nPreIssueRead\n";
    registersSection.append("R00:\t" + to_string(preIssueRead[0]) + "\t" + to_string(preIssueRead[1]) + "\t");
    registersSection.append(to_string(preIssueRead[2]) + "\t" + to_string(preIssueRead[3]) + "\t");
    registersSection.append(to_string(preIssueRead[4]) + "\t" + to_string(preIssueRead[5]) + "\t");
    registersSection.append(to_string(preIssueRead[6]) + "\t" + to_string(preIssueRead[7]) + "\n");
    registersSection.append("R08:\t" + to_string(preIssueRead[8]) + "\t" + to_string(preIssueRead[9]) + "\t");
    registersSection.append(to_string(preIssueRead[10]) + "\t" + to_string(preIssueRead[11]) + "\t");
    registersSection.append(to_string(preIssueRead[12]) + "\t" + to_string(preIssueRead[13]) + "\t");
    registersSection.append(to_string(preIssueRead[14]) + "\t" + to_string(preIssueRead[15]) + "\n");
    registersSection.append("R16:\t" + to_string(preIssueRead[16]) + "\t" + to_string(preIssueRead[17]) + "\t");
    registersSection.append(to_string(preIssueRead[18]) + "\t" + to_string(preIssueRead[19]) + "\t");\
    registersSection.append(to_string(preIssueRead[20]) + "\t" + to_string(preIssueRead[21]) + "\t");
    registersSection.append(to_string(preIssueRead[22]) + "\t" + to_string(preIssueRead[23]) + "\n");
    registersSection.append("R24:\t" + to_string(preIssueRead[24]) + "\t" + to_string(preIssueRead[25]) + "\t");
    registersSection.append(to_string(preIssueRead[26]) + "\t" + to_string(preIssueRead[27]) + "\t");\
    registersSection.append(to_string(preIssueRead[28]) + "\t" + to_string(preIssueRead[29]) + "\t");
    registersSection.append(to_string(preIssueRead[30]) + "\t" + to_string(preIssueRead[31]) + "\n\n");

    registersSection.append("R00:\t" + to_string(preIssueWrite[0]) + "\t" + to_string(preIssueWrite[1]) + "\t");
    registersSection.append(to_string(preIssueWrite[2]) + "\t" + to_string(preIssueWrite[3]) + "\t");
    registersSection.append(to_string(preIssueWrite[4]) + "\t" + to_string(preIssueWrite[5]) + "\t");
    registersSection.append(to_string(preIssueWrite[6]) + "\t" + to_string(preIssueWrite[7]) + "\n");
    registersSection.append("R08:\t" + to_string(preIssueWrite[8]) + "\t" + to_string(preIssueWrite[9]) + "\t");
    registersSection.append(to_string(preIssueWrite[10]) + "\t" + to_string(preIssueWrite[11]) + "\t");
    registersSection.append(to_string(preIssueWrite[12]) + "\t" + to_string(preIssueWrite[13]) + "\t");
    registersSection.append(to_string(preIssueWrite[14]) + "\t" + to_string(preIssueWrite[15]) + "\n");
    registersSection.append("R16:\t" + to_string(preIssueWrite[16]) + "\t" + to_string(preIssueWrite[17]) + "\t");
    registersSection.append(to_string(preIssueWrite[18]) + "\t" + to_string(preIssueWrite[19]) + "\t");\
    registersSection.append(to_string(preIssueWrite[20]) + "\t" + to_string(preIssueWrite[21]) + "\t");
    registersSection.append(to_string(preIssueWrite[22]) + "\t" + to_string(preIssueWrite[23]) + "\n");
    registersSection.append("R24:\t" + to_string(preIssueWrite[24]) + "\t" + to_string(preIssueWrite[25]) + "\t");
    registersSection.append(to_string(preIssueWrite[26]) + "\t" + to_string(preIssueWrite[27]) + "\t");\
    registersSection.append(to_string(preIssueWrite[28]) + "\t" + to_string(preIssueWrite[29]) + "\t");
    registersSection.append(to_string(preIssueWrite[30]) + "\t" + to_string(preIssueWrite[31]) + "\n\n");

    registersSection.append("R00:\t" + to_string(regsInRead[0]) + "\t" + to_string(regsInRead[1]) + "\t");
    registersSection.append(to_string(regsInRead[2]) + "\t" + to_string(regsInRead[3]) + "\t");
    registersSection.append(to_string(regsInRead[4]) + "\t" + to_string(regsInRead[5]) + "\t");
    registersSection.append(to_string(regsInRead[6]) + "\t" + to_string(regsInRead[7]) + "\n");
    registersSection.append("R08:\t" + to_string(regsInRead[8]) + "\t" + to_string(regsInRead[9]) + "\t");
    registersSection.append(to_string(regsInRead[10]) + "\t" + to_string(regsInRead[11]) + "\t");
    registersSection.append(to_string(regsInRead[12]) + "\t" + to_string(regsInRead[13]) + "\t");
    registersSection.append(to_string(regsInRead[14]) + "\t" + to_string(regsInRead[15]) + "\n");
    registersSection.append("R16:\t" + to_string(regsInRead[16]) + "\t" + to_string(regsInRead[17]) + "\t");
    registersSection.append(to_string(regsInRead[18]) + "\t" + to_string(regsInRead[19]) + "\t");\
    registersSection.append(to_string(regsInRead[20]) + "\t" + to_string(regsInRead[21]) + "\t");
    registersSection.append(to_string(regsInRead[22]) + "\t" + to_string(regsInRead[23]) + "\n");
    registersSection.append("R24:\t" + to_string(regsInRead[24]) + "\t" + to_string(regsInRead[25]) + "\t");
    registersSection.append(to_string(regsInRead[26]) + "\t" + to_string(regsInRead[27]) + "\t");\
    registersSection.append(to_string(regsInRead[28]) + "\t" + to_string(regsInRead[29]) + "\t");
    registersSection.append(to_string(regsInRead[30]) + "\t" + to_string(regsInRead[31]) + "\n\n");

    registersSection.append("R00:\t" + to_string(regsInWrite[0]) + "\t" + to_string(regsInWrite[1]) + "\t");
    registersSection.append(to_string(regsInWrite[2]) + "\t" + to_string(regsInWrite[3]) + "\t");
    registersSection.append(to_string(regsInWrite[4]) + "\t" + to_string(regsInWrite[5]) + "\t");
    registersSection.append(to_string(regsInWrite[6]) + "\t" + to_string(regsInWrite[7]) + "\n");
    registersSection.append("R08:\t" + to_string(regsInWrite[8]) + "\t" + to_string(regsInWrite[9]) + "\t");
    registersSection.append(to_string(regsInWrite[10]) + "\t" + to_string(regsInWrite[11]) + "\t");
    registersSection.append(to_string(regsInWrite[12]) + "\t" + to_string(regsInWrite[13]) + "\t");
    registersSection.append(to_string(regsInWrite[14]) + "\t" + to_string(regsInWrite[15]) + "\n");
    registersSection.append("R16:\t" + to_string(regsInWrite[16]) + "\t" + to_string(regsInWrite[17]) + "\t");
    registersSection.append(to_string(regsInWrite[18]) + "\t" + to_string(regsInWrite[19]) + "\t");\
    registersSection.append(to_string(regsInWrite[20]) + "\t" + to_string(regsInWrite[21]) + "\t");
    registersSection.append(to_string(regsInWrite[22]) + "\t" + to_string(regsInWrite[23]) + "\n");
    registersSection.append("R24:\t" + to_string(regsInWrite[24]) + "\t" + to_string(regsInWrite[25]) + "\t");
    registersSection.append(to_string(regsInWrite[26]) + "\t" + to_string(regsInWrite[27]) + "\t");\
    registersSection.append(to_string(regsInWrite[28]) + "\t" + to_string(regsInWrite[29]) + "\t");
    registersSection.append(to_string(regsInWrite[30]) + "\t" + to_string(regsInWrite[31]) + "\n\n");

    simFile << registersSection;
  }
  void display(){

    string empty = "";
    // printing of file starts
    simFile << "--------------------" << endl;
    simFile << "Cycle " << cycle++ << ":" << endl << endl;
    simFile << "IF:\n\tWaiting: " << (waitInstr < 0 ? empty : "[" + decodedInstructions[waitInstr] + "]")
      << "\n\tExecuted: " << (execInstr < 0 ? empty : "[" + decodedInstructions[execInstr] + "]") << "\n";
    int buffersize = this->Buf1.size();
    simFile << "Buf1:\n\tEntry 0: " << (0 < buffersize ? "[" + decodedInstructions[this->Buf1[0]] + "]" : empty)
      << "\n\tEntry 1: " << (1 < buffersize ? "[" + decodedInstructions[this->Buf1[1]] + "]" : empty)
      << "\n\tEntry 2: " << (2 < buffersize ? "[" + decodedInstructions[this->Buf1[2]] + "]" : empty)
      << "\n\tEntry 3: " << (3 < buffersize ? "[" + decodedInstructions[this->Buf1[3]] + "]" : empty)
      << "\n\tEntry 4: " << (4 < buffersize ? "[" + decodedInstructions[this->Buf1[4]] + "]" : empty)
      << "\n\tEntry 5: " << (5 < buffersize ? "[" + decodedInstructions[this->Buf1[5]] + "]" : empty)
      << "\n\tEntry 6: " << (6 < buffersize ? "[" + decodedInstructions[this->Buf1[6]] + "]" : empty)
      << "\n\tEntry 7: " << (7 < buffersize ? "[" + decodedInstructions[this->Buf1[7]] + "]" : empty) << "\n";
    buffersize = this->Buf2.size();
    simFile << "Buf2:\n\tEntry 0: " << (0 < buffersize ? "[" + decodedInstructions[this->Buf2[0]] + "]" : empty)
      << "\n\tEntry 1: " << (1 < buffersize ? "[" + decodedInstructions[this->Buf2[1]] + "]" : empty) << "\n";
    buffersize = this->Buf3.size();
    simFile << "Buf3:\n\tEntry 0: " << (0 < buffersize ? "[" + decodedInstructions[this->Buf3[0]] + "]" : empty)
      << "\n\tEntry 1: " << (1 < buffersize ? "[" + decodedInstructions[this->Buf3[1]] + "]" : empty) << "\n";
    buffersize = this->Buf4.size();
    simFile << "Buf4:\n\tEntry 0: " << (0 < buffersize ? "[" + decodedInstructions[this->Buf4[0]] + "]" : empty)
      << "\n\tEntry 1: " << (1 < buffersize ? "[" + decodedInstructions[this->Buf4[1]] + "]" : empty) << "\n";
    buffersize = this->Buf5.size();
    simFile << "Buf5:\n\tEntry 0: " << (0 < buffersize ? "[" + decodedInstructions[this->Buf5[0]] + "]" : empty)
      << "\n\tEntry 1: " << (1 < buffersize ? "[" + decodedInstructions[this->Buf5[1]] + "]" : empty) << "\n";
    simFile << "Buf6: " << (this->Buf6[0] < 0 ? empty : "[" + decodedInstructions[this->Buf6[0]] + "]") << "\n";
    simFile << "Buf7: " << (this->Buf7[0] < 0 ? empty : "[" + to_string(this->Buf7[2]) + ", " + to_string(this->Buf7[1]) +  "]") << "\n";
    simFile << "Buf8: " << (this->Buf8[0] < 0 ? empty : "[" + decodedInstructions[this->Buf8[0]] + "]") << "\n";
    simFile << "Buf9: " << (this->Buf9[0] < 0 ? empty : "[" + to_string(this->Buf9[2]) + ", R" + to_string(this->Buf9[1]) +  "]") << "\n";
    simFile << "Buf10: " << (this->Buf10[0] < 0 ? empty : "[" + to_string(this->Buf10[2]) + ", R" + to_string(this->Buf10[1]) + "]") << "\n";
    simFile << "Buf11: " << (this->Buf11[0] < 0 ? empty : "[" + decodedInstructions[this->Buf11[0]] + "]") << "\n";
    simFile << "Buf12: " << (this->Buf12[0] < 0 ? empty : "[" + to_string(this->Buf12[2]) + "]") << "\n\n";

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
    registersSection.append(to_string(modRegisterValues[30]) + "\t" + to_string(modRegisterValues[31]) + "\n");
    registersSection.append("HI:\t" + to_string(modRegisterValues[32]) + "\n");
    registersSection.append("LO:\t" + to_string(modRegisterValues[33]) + "\n\n");

    simFile << registersSection;
    // Create the "Data" section
    string dataSection = "Data\n";
    dataSection.append(to_string(inNu) + ":\t" + to_string(DataValues[0]) + "\t" + to_string(DataValues[1]) + "\t");
    dataSection.append(to_string(DataValues[2]) + "\t" + to_string(DataValues[3]) + "\t");
    dataSection.append(to_string(DataValues[4]) + "\t" + to_string(DataValues[5]) + "\t");
    dataSection.append(to_string(DataValues[6]) + "\t" + to_string(DataValues[7]) + "\n");
    dataSection.append(to_string(inNu + 32) + ":\t" + to_string(DataValues[8]) + "\t" + to_string(DataValues[9]) + "\t");
    dataSection.append(to_string(DataValues[10]) + "\t" + to_string(DataValues[11]) + "\t");
    dataSection.append(to_string(DataValues[12]) + "\t" + to_string(DataValues[13]) + "\t");
    dataSection.append(to_string(DataValues[14]) + "\t" + to_string(DataValues[15]) + "\n");

    simFile << dataSection;
  }
  void pipeline(){
    inNu = 256; // Instruction number of where the register values start
    string previousInstructionString = "";
    for(vector<string>::iterator iter_current = decodedInstructions.begin(); iter_current != decodedInstructions.end() && previousInstructionString != "BREAK"; iter_current++) {
        previousInstructionString = *iter_current;
        inNu = inNu + 4;
    }

    bool isBreak = false;
    int unit1Return = 0, unit2Return = 0, unit3Return = 0, unit4Return = 0, unit5Return = 0, unit6Return = 0;
    while(!isBreak){
      isBreak = instruction_fetch();

      instruction_issue();
      // cout << 2 << "x " << endl;
      unit1Return = executeALU2();
  		unit2Return = executeDIV();
      unit3Return = executeMUL1();
      unit4Return = executeALU1();
      unit5Return = executeMUL2();
      unit6Return = executeMUL3();
      // cout << 3 << "x " << endl;
  		mem();
      // cout << 4 << "x " << endl;
  		wb();
      // cout << 5 << "x " << endl;
      flipLatch();
      // cout << 6 << "x " << endl;
      display();
      // cout << 7 << "x " << endl;
    }
  }

  void flipLatch()
  {
  	this->Buf1 = this->Buf1_Update;
    this->Buf2 = this->Buf2_Update;
    this->Buf3 = this->Buf3_Update;
    this->Buf4 = this->Buf4_Update;
    this->Buf5 = this->Buf5_Update;
    std::copy(std::begin(this->Buf6_Update), std::end(this->Buf6_Update), std::begin(this->Buf6));
    std::copy(std::begin(this->Buf7_Update), std::end(this->Buf7_Update), std::begin(this->Buf7));
    std::copy(std::begin(this->Buf8_Update), std::end(this->Buf8_Update), std::begin(this->Buf8));
    std::copy(std::begin(this->Buf9_Update), std::end(this->Buf9_Update), std::begin(this->Buf9));
    std::copy(std::begin(this->Buf10_Update), std::end(this->Buf10_Update), std::begin(this->Buf10));
    std::copy(std::begin(this->Buf11_Update), std::end(this->Buf11_Update), std::begin(this->Buf11));
    std::copy(std::begin(this->Buf12_Update), std::end(this->Buf12_Update), std::begin(this->Buf12));

  }

  void getCurrentInstrcution(int index){
    currentNode = this->root->next;
    while(index != 0 && currentNode->next != nullptr){
      currentNode = currentNode = currentNode->next;
      index --;
    }
  }

  void instruction_issue(){
    int preALU2empty = BUF_2_SIZE - this->Buf2.size(); //num of empty slots in ALU 1
    int preDIVempty = BUF_3_SIZE - this->Buf3.size(); //num of empty slots in ALU 1
  	int preMULempty = BUF_4_SIZE - this->Buf4.size(); //num of empty slots in ALU 2
  	int preALU1empty = BUF_5_SIZE - this->Buf5.size(); //num of empty slots in ALU 2

    this->Buf2_Update = this->Buf2;
    this->Buf3_Update = this->Buf3;
    this->Buf4_Update = this->Buf4;
    this->Buf5_Update = this->Buf5;

    std::fill(this->preIssueWrite, this->preIssueWrite+34, 0);//reset preIssue read/write table
  	std::fill(this->preIssueRead, this->preIssueRead+34, 0);

    vector<int> rmFromPreIssue(0);

  	if (preALU1empty + preMULempty + preDIVempty + preALU2empty == 0){
      return;
    }

    int nbInstrToIssue = this->Buf1.size();

    bool buf2ok = true, buf3ok = true, buf4ok = true, buf5ok = true;
  	if (preALU2empty == 0) buf2ok = false;
  	if (preDIVempty == 0) buf3ok = false;
    if (preMULempty == 0) buf4ok = false;
    if (preALU1empty == 0) buf5ok = false;

    while (nbInstrToIssue > 0 &&  (buf2ok || buf3ok || buf4ok || buf5ok)){

      int idx = this->Buf1[this->Buf1.size() - nbInstrToIssue];
      getCurrentInstrcution(idx); // set currentNode to current instruction
      // cout << this->decodedInstructions[idx] << endl;
      int instrCount = (this->currentNode->instructionNumber - 256) / 4;//index of instr
      string categoryNumber = this->instructions[instrCount].substr(0,3); // Get the current instruction's category number
      string opcode = this->instructions[instrCount].substr(3,3); // Get the current instruction's opcode

      // cout << categoryNumber << " " << opcode << " "<< this->currentNode->operation << endl;
      if(categoryNumber == "000") { // The instruction is a category 1

        int rt = stoi(this->currentNode->rt);

        string baseAddress = currentNode->baseAddress;
        int iba = stoi(baseAddress); // base address in integer form

        if(opcode == "101" || opcode == "100"){ // LW && SW

        if (!regsInWrite[iba] && !regsInWrite[rt] && !preIssueWrite[iba] && !preIssueWrite[rt]) { //check RAW (but no WAR for LW?)
    				if (buf2ok)
    				{
    					if (opcode == "100") {
    						// regsInRead[iba] = 1;
    						//So there is no need to record them because no instr can write to rs/rt within 2 cycles
                ;
    					}
    					else if (opcode == "101") {
    						regsInWrite[rt] = 1;
    					}
    					this->Buf2_Update.push_back(idx);
    					rmFromPreIssue.push_back(this->Buf1.size() - nbInstrToIssue);
              buf2ok == BUF_2_SIZE - this->Buf2_Update.size();
    				}
    			}
    			if (opcode == "100") {//update preIssue read/write table for SW
    				preIssueRead[iba] = 1; preIssueWrite[rt] = 1;
    			}
    			else if (opcode == "101") {//update preIssue read/write table for LW
    				preIssueRead[iba] = 1; preIssueWrite[rt] = 1;
    			}
        }
      } else if(categoryNumber == "001") { // The instruction is a category 2

        int rd = stoi(this->currentNode->rd);
        int rt = stoi(this->currentNode->rt);

        if (buf5ok){
          if (opcode == "100" || opcode == "101") {   //SRL, SLA
            //SRL SLA will read rt, write rd
  					if (!regsInRead[rd] && !regsInWrite[rt] && !regsInWrite[rd]
  						&& !preIssueRead[rd] && !preIssueWrite[rt] && !preIssueWrite[rd]) {//check WAW and WAR and RAW?

  						regsInWrite[rd] = 1;  //update regsInRead or regsInWrite
  						this->Buf5_Update.push_back(idx);
  						rmFromPreIssue.push_back(this->Buf1.size() - nbInstrToIssue);
              buf5ok = BUF_5_SIZE - this->Buf5_Update.size();
  					}
  					preIssueRead[rt] = 1; preIssueWrite[rd] = 1;
  				}
          // ADD SUB AND OR
          else if (opcode == "000" || opcode == "001" || opcode== "010" || opcode=="011") {

            int rs = stoi(this->currentNode->rs);

            if (!regsInRead[rd] && !regsInWrite[rd] && !regsInWrite[rs] && !regsInWrite[rt]
  						&& !preIssueRead[rd] && !preIssueWrite[rd] && !preIssueWrite[rs] && !preIssueWrite[rt]) {//check WAW and WAR and RAW

  						regsInWrite[rd] = 1;
  						this->Buf5_Update.push_back(idx);
  						rmFromPreIssue.push_back(this->Buf1.size() - nbInstrToIssue);
              buf5ok = BUF_5_SIZE - this->Buf5_Update.size();
  					}
  					preIssueRead[rs] = 1; preIssueRead[rt] = 1; preIssueWrite[rd] = 1;
  				}
        }

      } else if(categoryNumber == "010") { // The instruction is a category 3

        int rt = stoi(this->currentNode->rt);
        int rs = stoi(this->currentNode->rs);

        if (buf5ok){
          // ADDI ANDI ORI
          if (opcode == "000" || opcode == "001" || opcode== "010") {
  					if (!regsInRead[rt] && !regsInWrite[rt] && !regsInWrite[rs]
  						&& !preIssueRead[rt] && !preIssueWrite[rt] && !preIssueWrite[rs]) {//check WAW and WAR and RAW?
  																						   //update regsInRead or regsInWrite
  						regsInWrite[rt] = 1; //regsInRead[rs] = 1;
  						this->Buf5_Update.push_back(idx);

  						rmFromPreIssue.push_back(this->Buf1.size() - nbInstrToIssue);
              buf5ok = BUF_5_SIZE - this->Buf5_Update.size();
  					}

  					preIssueRead[rs] = 1; preIssueWrite[rt] = 1;
  				}
        }
      }
      else if(categoryNumber == "011") { // The instruction is a category 4

        int rt = stoi(this->currentNode->rt);
        int rs = stoi(this->currentNode->rs);

        if (opcode == "000") { // mult
          if (buf4ok){ // read rs,rt and write in lo, hi
            if (!regsInRead[33] && !regsInWrite[33] && !regsInWrite[rs] && !regsInWrite[rt]
              && !preIssueRead[33] && !preIssueWrite[33] && !preIssueWrite[rs] && !preIssueWrite[rt]) {//check WAW and WAR and RAW
                regsInWrite[33] = 1;
                this->Buf4_Update.push_back(idx);
    						rmFromPreIssue.push_back(this->Buf1.size() - nbInstrToIssue);
    						buf4ok = BUF_4_SIZE - this->Buf4_Update.size();
              }

              preIssueRead[rs] = 1; preIssueRead[rt] = 1; preIssueWrite[33] = 1;

          }
        }else if(opcode == "001"){ //div

          int rt = stoi(this->currentNode->rt);
          int rs = stoi(this->currentNode->rs);
          if (buf3ok){ // read rs,rt and write in hi, lo
            if (!regsInRead[32] && !regsInWrite[32] & !regsInRead[33] && !regsInWrite[33] && !regsInWrite[rs] && !regsInWrite[rt]
              && !preIssueRead[32] && !preIssueWrite[32] && !preIssueRead[33] && !preIssueWrite[33] && !preIssueWrite[rs] && !preIssueWrite[rt]) {//check WAW and WAR and RAW
                regsInWrite[33] = 1;
                regsInWrite[32] = 1;

                this->Buf3_Update.push_back(idx);
                rmFromPreIssue.push_back(this->Buf1.size() - nbInstrToIssue);
    						buf3ok = BUF_3_SIZE - this->Buf3_Update.size();
              }

              preIssueRead[rs] = 1; preIssueRead[rt] = 1; preIssueWrite[33] = 1; preIssueWrite[32] = 1;
          }
        }
      }
      else if(categoryNumber == "100") { // The instruction is a category 5

        int rd = stoi(this->currentNode->rd);

        if (opcode == "000") { // mfhi
          if (buf5ok){ // read hi and write in rd
            if (!regsInRead[rd] && !regsInWrite[rd] && !regsInWrite[32]
  						&& !preIssueRead[rd] && !preIssueWrite[rd] && !preIssueWrite[32] ) {//check WAW and WAR and RAW
                regsInWrite[rd] = 1;
                this->Buf5_Update.push_back(idx);
    						rmFromPreIssue.push_back(this->Buf1.size() - nbInstrToIssue);
    						buf5ok = BUF_5_SIZE - this->Buf5_Update.size();
              }

              preIssueRead[32] = 1;  preIssueWrite[rd] = 1;
          }
        }else if (opcode == "001") { // mflo
          if (buf5ok){ // read lo and write in rd
            if (!regsInRead[rd] && !regsInWrite[rd] && !regsInWrite[33]
              && !preIssueRead[rd] && !preIssueWrite[rd] && !preIssueWrite[33] ) {//check WAW and WAR and RAW
                regsInWrite[rd] = 1;
                this->Buf5_Update.push_back(idx);
                rmFromPreIssue.push_back(this->Buf1.size() - nbInstrToIssue);
                buf5ok = BUF_5_SIZE - this->Buf5_Update.size();
              }

              preIssueRead[33] = 1;  preIssueWrite[rd] = 1;

            }
          }
      }

      nbInstrToIssue--;
    }
    // cout << "==================" << endl;

    //update preIssue buffer TODO, how to erase without knowing order? remove from the last one!
  	for (int i = rmFromPreIssue.size() - 1; i >= 0; i--)
  		this->Buf1_Update.erase(this->Buf1_Update.begin() + rmFromPreIssue[i]);
  }
  int executeALU2() //ALU1 handles one SW/LW from preALU1
  {
    this->Buf6_Update[0] = -1;
  	if (this->Buf2.size() > 0)//FPpreMem = {instr idx, destination reg, result/source/mem dest}
  	{
      int idx = this->Buf2[0];
      this->Buf2_Update.erase(this->Buf2_Update.begin());////remove used instr from preALU1

      getCurrentInstrcution(idx); // set currentNode to current instruction

      int instrCount = (this->currentNode->instructionNumber - 256) / 4;//index of instr
      string categoryNumber = this->instructions[instrCount].substr(0,3); // Get the current instruction's category number
      string opcode = this->instructions[instrCount].substr(3,3); // Get the current instruction's opcode

  		if (opcode == "100" || opcode == "101") { //sw or lw handled exactly the same

        string baseAddress = currentNode->baseAddress;
        int iba = stoi(baseAddress); // base address in integer form
        string offset = currentNode->offset;
        int io = stoi(offset); // offset in integer form
        string rtReg = currentNode->rt;
        int irt = stoi(rtReg); // rt register in integer form

  			this->Buf6_Update[0] = idx;
  			this->Buf6_Update[1] = irt;
        this->Buf6_Update[2] = (io - inNu + modRegisterValues[iba])/4;

  		}
  	}
  	return 0;
  }
  int executeDIV(){

    this->Buf7_Update[0] = -1;

  	if (this->Buf3.size() > 0)//FPpreMem = {instr idx, destination reg, result/source/mem dest}
  	{
      int idx = this->Buf3[0];
      this->Buf3_Update.erase(this->Buf3_Update.begin());////remove used instr from preALU1

      getCurrentInstrcution(idx); // set currentNode to current instruction

      int instrCount = (this->currentNode->instructionNumber - 256) / 4;//index of instr
      string categoryNumber = this->instructions[instrCount].substr(0,3); // Get the current instruction's category number
      string opcode = this->instructions[instrCount].substr(3,3); // Get the current instruction's opcode

      string rtReg = currentNode->rt;
      int rt = stoi(rtReg); // rt register in integer form
      string rsReg = currentNode->rs;
      int rs = stoi(rsReg); // rt register in integer form

      this->Buf7_Update[0] = idx;

      if (modRegisterValues[rs] != 0){
        // Unsure about whether the remainder should be -ve or +ve when dividing a -ve number: This gives a -ve remainder
        int32_t remainder = (int32_t)modRegisterValues[rt] % (int32_t)modRegisterValues[rs];
        int32_t quotient = (int32_t)modRegisterValues[rt] / (int32_t)modRegisterValues[rs];

        this->Buf7_Update[1] = (uint32_t)quotient;
        this->Buf7_Update[2] = (uint32_t)remainder;
      } else{
        this->Buf7_Update[1] = 0;
        this->Buf7_Update[2] = 0;
      }
      regsInRead[rs] = 0; regsInRead[rt] = 0;
  	}
  	return 0;
  }
  int executeMUL1(){
    this->Buf8_Update[0] = -1;

  	if (this->Buf4.size() > 0) {

      int idx = this->Buf4[0];
      this->Buf4_Update.erase(this->Buf4_Update.begin());////remove used instr from preALU1

      // cout << decodedInstructions[idx] << " xxasdasd "<< endl;
      getCurrentInstrcution(idx); // set currentNode to current instruction

      int instrCount = (this->currentNode->instructionNumber - 256) / 4;//index of instr
      string categoryNumber = this->instructions[instrCount].substr(0,3); // Get the current instruction's category number
      string opcode = this->instructions[instrCount].substr(3,3); // Get the current instruction's opcode

      string rtReg = currentNode->rt;
      int rt = stoi(rtReg); // rt register in integer form
      string rsReg = currentNode->rs;
      int rs = stoi(rsReg); // rt register in integer form

      this->Buf8_Update[0] = idx;

  	}
  	return 0;
  }

  int executeMUL2(){
    this->Buf11_Update[0] = -1;

  	if (this->Buf8[0] > 0) {

      int idx = this->Buf8[0];
      this->Buf8_Update[0] = -1;

      // cout << decodedInstructions[idx] << " xxasdasd "<< endl;
      getCurrentInstrcution(idx); // set currentNode to current instruction

      int instrCount = (this->currentNode->instructionNumber - 256) / 4;//index of instr
      string categoryNumber = this->instructions[instrCount].substr(0,3); // Get the current instruction's category number
      string opcode = this->instructions[instrCount].substr(3,3); // Get the current instruction's opcode

      string rtReg = currentNode->rt;
      int rt = stoi(rtReg); // rt register in integer form
      string rsReg = currentNode->rs;
      int rs = stoi(rsReg); // rt register in integer form

      this->Buf11_Update[0] = idx;

  	}
  	return 0;
  }

  int executeMUL3(){
    this->Buf12_Update[0] = -1;

  	if (this->Buf11[0] > 0) {

      int idx = this->Buf11[0];
      this->Buf11_Update[0] = -1;

      // cout << decodedInstructions[idx] << " xxasdasd "<< endl;
      getCurrentInstrcution(idx); // set currentNode to current instruction

      int instrCount = (this->currentNode->instructionNumber - 256) / 4;//index of instr
      string categoryNumber = this->instructions[instrCount].substr(0,3); // Get the current instruction's category number
      string opcode = this->instructions[instrCount].substr(3,3); // Get the current instruction's opcode

      string rtReg = currentNode->rt;
      int rt = stoi(rtReg); // rt register in integer form
      string rsReg = currentNode->rs;
      int rs = stoi(rsReg); // rt register in integer form

      this->Buf12_Update[0] = idx;

      int64_t result = (int64_t)((int32_t)modRegisterValues[rs]) * (int64_t)((int32_t)modRegisterValues[rt]);

      this->Buf12_Update[2] =(int32_t)(result & 0xFFFFFFFF);;
      regsInRead[rs] = 0; regsInRead[rt] = 0;
  	}
  	return 0;
  }

  int executeALU1(){
    this->Buf9_Update[0] = -1;

  	if (this->Buf5.size() > 0) {

      int idx = this->Buf5[0];
      this->Buf5_Update.erase(this->Buf5_Update.begin());////remove used instr from preALU1

      // cout << decodedInstructions[idx] << " xxasdasd "<< endl;
      getCurrentInstrcution(idx); // set currentNode to current instruction

      int instrCount = (this->currentNode->instructionNumber - 256) / 4;//index of instr
      string categoryNumber = this->instructions[instrCount].substr(0,3); // Get the current instruction's category number
      string opcode = this->instructions[instrCount].substr(3,3); // Get the current instruction's opcode

      if(categoryNumber == "000") { // The instruction is a category 1

        string rdReg = currentNode->rd;
        int rd = stoi(rdReg); // rt register in integer form
        string rtReg = currentNode->rt;
        int rt = stoi(rtReg); // rt register in integer form
        string rsReg = currentNode->rs;
        int rs = stoi(rsReg); // rt register in integer form

        this->Buf9_Update[0] = idx;
  			this->Buf9_Update[1] = rd;
  			regsInRead[rt] = 0;//release rt, rs(not actually used)

  			if (opcode == "100") { //
  				this->Buf9_Update[2] = (unsigned int)modRegisterValues[rs]>>rt; // Compute the bitwise OR of registers rt and rs
  			}
  			else if (opcode == "101") {//sra()
  				this->Buf9_Update[2] = modRegisterValues[rs]>>rt; // Compute the bitwise OR of registers rt and rs
  			}

      } else if(categoryNumber == "001") { // The instruction is a category 2

        string rdReg = currentNode->rd;
        int rd = stoi(rdReg); // rt register in integer form
        string rtReg = currentNode->rt;
        int rt = stoi(rtReg); // rt register in integer form
        string rsReg = currentNode->rs;
        int rs = stoi(rsReg); // rt register in integer form

        this->Buf9_Update[0] = idx;
        this->Buf9_Update[1] = rd;

        //assume all the cat-2 will release rs,rt in read here
  			if (opcode == "000") {//add();
  				this->Buf9_Update[2] = modRegisterValues[rs] + modRegisterValues[rt];
  				regsInRead[rs] = 0; regsInRead[rt] = 0;
  			}
  			else if (opcode == "001") {//sub();
  				this->Buf9_Update[2] = modRegisterValues[rs] - modRegisterValues[rt];
  				regsInRead[rs] = 0; regsInRead[rt] = 0;
  			}
  			else if (opcode == "010") {//and();
  				this->Buf9_Update[2] = modRegisterValues[rt]&modRegisterValues[rs];
  				regsInRead[rs] = 0; regsInRead[rt] = 0;
  			}
  			else if (opcode == "010") {//or();
  				this->Buf9_Update[2] = modRegisterValues[rt]|modRegisterValues[rs];
  				regsInRead[rs] = 0; regsInRead[rt] = 0;
  			}
      } else if(categoryNumber == "010") { // The instruction is a category 3

        string rtReg = currentNode->rt;
        int rt = stoi(rtReg); // rt register in integer form
        string rsReg = currentNode->rs;
        int rs = stoi(rsReg); // rt register in integer form

        this->Buf9_Update[0] = idx;
  			this->Buf9_Update[1] = rt;

        string immediate = currentNode->immediate;
        int io = stoi(immediate); // immediate value in integer form

        if (opcode == "000") {//addi()
  				this->Buf9_Update[2] = modRegisterValues[rs] + io;
  				regsInRead[rs] = 0;
  			}
  			else if (opcode == "001") {//andi()
  				this->Buf9_Update[2] = modRegisterValues[rs] & io;
  				regsInRead[rs] = 0;
  			}
  			else if (opcode == "010") {//ori()
  				this->Buf9_Update[2] = modRegisterValues[rs] | io;
  				regsInRead[rs] = 0;
  			}
      } else if(categoryNumber == "100") { // The instruction is a category 5

        string rdReg = currentNode->rd;
        int rd = stoi(rdReg); // rt register in integer form

        this->Buf9_Update[0] = idx;
  			this->Buf9_Update[1] = rd;

        if (opcode == "000") {//MFHI
  				this->Buf9_Update[2] = modRegisterValues[32];
  				regsInRead[32] = 0;
  			}
  			else if (opcode == "001") {//MFLO
  				this->Buf9_Update[2] = modRegisterValues[33];
  				regsInRead[33] = 0;
  			}
      }
  	}
  	return 0;
  }

  void mem()//executes SW and pass LW to postMEM
  {
  	if (this->Buf6[0] >= 0)
  	{

      getCurrentInstrcution(this->Buf6[0]); // set currentNode to current instructio

  		if (this->currentNode->operation == "SW") {//SW

        DataValues[this->Buf6[2]] = modRegisterValues[this->Buf6[1]];
  			regsInRead[this->Buf6[1]] = 0;//release (preMem[1])rt in read for SW
  		}
  		else if (this->currentNode->operation == "LW") {//LW

  			this->Buf10_Update[0] = this->Buf6[0];//instr forwarding
  			this->Buf10_Update[1] = this->Buf6[1]; // 1 - destination register
  			this->Buf10_Update[2] = DataValues[this->Buf6[2]];// 2 - actual value to be stored
  		}
  		else
  			return;
  		this->Buf6[0] = -1;//clearing queue
  		return;
  	}
  	else
  		this->Buf10_Update[0] = -1;
  }

  void wb()
  {

  	if (this->Buf9[0] != -1) {
  		modRegisterValues[this->Buf9[1]] = this->Buf9[2];// 1 - destination register, 2 - value
  		regsInWrite[this->Buf9[1]] = 0;//release rd(or rt)
  		this->Buf9[0] = -1;//reseting this is unnecessary, since it will be replaced by FPpostALU anyway
  	}

    if (this->Buf9[0] != -1) {
  		modRegisterValues[this->Buf9[1]] = this->Buf9[2];// 1 - destination register, 2 - value
  		regsInWrite[this->Buf9[1]] = 0;//release rd(or rt)
  		this->Buf9[0] = -1;//reseting this is unnecessary, since it will be replaced by FPpostALU anyway
  	}

  	//postMEM write back - only for LW instr
  	if (this->Buf10[0] != -1) {
  		modRegisterValues[this->Buf10[1]] = this->Buf10[2];// 1 - destination register, 2 - value
  		regsInWrite[this->Buf10[1]] = 0;//release rd//release rt for LW

  		this->Buf10[0] = -1;
  	}

    if (this->Buf12[0] != -1) {

  		modRegisterValues[33] = this->Buf12[2];// 1 - destination register, 2 - value
  		regsInWrite[33] = 0;//release rd//release rt for LW
      regsInWrite[32] = 0;//release rd//release rt for LW

  		this->Buf12[0] = -1;
  	}

    if (this->Buf7[0] != -1) {

  		modRegisterValues[33] = this->Buf7[1];// 1 - destination register, 2 - value
  		regsInWrite[32] = this->Buf7[2]; //release rd//release rt for LW
      // regsInWrite[32] = 0;//release rd//release rt for LW
      regsInWrite[33] = 0;//release rd//release rt for LW
      regsInWrite[32] = 0;//release rd//release rt for LW

  		this->Buf7[0] = -1;
  	}
  	return;
  }

  bool instruction_fetch(){ //return 1 if BREAK; will finish handling NOP, Branch, BREAK
    bool break_flag = false;
    int count = 0;
    int instrCount;
    string instr;
    // cout << 1 << endl;
    this->execInstr = -1;

    this->Buf1_Update = this->Buf1;
    int preIssueSize = this->Buf1.size();

    for (int i = 0; i < preIssueSize; i++) {
        getCurrentInstrcution(this->Buf1[i]); // set currentNode to current instruction
        instrCount = (this->currentNode->instructionNumber - 256) / 4;//index of instr

        string categoryNumber = this->instructions[instrCount].substr(0,3); // Get the current instruction's category number
        if(categoryNumber == "000") { // The instruction is a category 1
          string opcode = this->instructions[instrCount].substr(3,3); // Get the current instruction's opcode
          if(opcode == "101"){ // LW
            this->preIssueWrite[stoi(this->currentNode->rt)] = 1;
          }
        } else if(categoryNumber == "001") { // The instruction is a category 2
          // cout << "b" << endl;
          this->preIssueWrite[stoi(this->currentNode->rd)] = 1;
        } else if(categoryNumber == "010") { // The instruction is a category 3
          this->preIssueWrite[stoi(this->currentNode->rt)] = 1;
        }
        else if(categoryNumber == "011") { // The instruction is a category 4
          // cout << "c" << endl;
          string opcode = this->instructions[instrCount].substr(3,3); // Get the current instruction's opcode
          if(opcode == "000"){ // MULT
            this->preIssueWrite[33] = 1; // LO register
            // this->preIssueWrite[32] = 1; // HI register
          }else if(opcode == "001") {
            this->preIssueWrite[33] = 1; // LO register
            this->preIssueWrite[32] = 1; // HI register
          }
        }
        else if(categoryNumber == "100") { // The instruction is a category 5
          this->preIssueWrite[stoi(this->currentNode->rd)] = 1;
        }
    }
    // cout << 2 << endl;

    if (this->waitInstr != -1){ // there is a pending branch instr, fetch stalled

      instr = this->decodedInstructions[waitInstr];//this could be J, BEQ, BNE, BGTZ, BREAK
      getCurrentInstrcution(waitInstr); // set currentNode to current instruction
      string currentOperation = currentNode->operation; // Get the current instruction's operation
      if(currentOperation == "BEQ") {

          string offset = currentNode->offset;
          int io = stoi(offset); // offset in integer form
          string rtReg = currentNode->rt;
          int irt = modRegisterValues[stoi(rtReg)]; // rt register in integer form
          string rsReg = currentNode->rs;
          int irs = modRegisterValues[stoi(rsReg)]; // rs register in integer form

          int nextInstructionNumber = currentNode->instructionNumber + io; // Get the next instruction number (to be used if rt == rs)

          if (this->regsInWrite[stoi(rsReg)] || this->regsInWrite[stoi(rtReg)] || this->preIssueWrite[stoi(rsReg)] || this->preIssueWrite[stoi(rtReg)]) {//check if rs, rt in use
            return 0; //still stalled
          }

          if(irt == irs) {
              Node* startingNode = root;
              while(startingNode->next->instructionNumber != nextInstructionNumber) { // Find the instruction at the offsetted location
                  startingNode = startingNode->next;
              }
              currentNode = startingNode->next; // Set the current node to the offsetted location

              this->currentInstructionNumber = currentNode->instructionNumber + 4;

          }

          execInstr = waitInstr;
    			waitInstr = -1;

      } else if(currentOperation == "BNE") {
        ////////////sdadadasdas
        string offset = currentNode->offset;
        int io = stoi(offset); // offset in integer form
        string rtReg = currentNode->rt;
        int irt = modRegisterValues[stoi(rtReg)]; // rt register in integer form
        string rsReg = currentNode->rs;
        int irs = modRegisterValues[stoi(rsReg)]; // rs register in integer form

        int nextInstructionNumber = currentNode->instructionNumber + io; // Get the next instruction number (to be used if rt == rs)

        if (this->regsInWrite[stoi(rsReg)] || this->regsInWrite[stoi(rtReg)] || this->preIssueWrite[stoi(rsReg)] || this->preIssueWrite[stoi(rtReg)]) {//check if rs, rt in use
          return 0; //still stalled
        }

        if(irt != irs) {
            Node* startingNode = root;
            while(startingNode->next->instructionNumber != nextInstructionNumber) { // Find the instruction at the offsetted location
                startingNode = startingNode->next;
            }
            currentNode = startingNode->next; // Set the current node to the offsetted location

            this->currentInstructionNumber = currentNode->instructionNumber + 4;
        }


        execInstr = waitInstr;
        waitInstr = -1;

      } else if(currentOperation == "BGTZ") {

          string offset = currentNode->offset;
          int io = stoi(offset); // offset in integer form
          string rsReg = currentNode->rs;
          int irs = modRegisterValues[stoi(rsReg)]; // rs register in integer form
          int nextInstructionNumber = currentNode->instructionNumber + io; // Get the next instruction number (to be used if rs > 0)


          if (this->regsInWrite[stoi(rsReg)] || this->preIssueWrite[stoi(rsReg)])
    				return 0;//still stalled

          if(irs > 0) {
              Node* startingNode = root;
              while(startingNode->next->instructionNumber != nextInstructionNumber) { // Find the instruction at the offsetted location
                  startingNode = startingNode->next;
              }
              currentNode = startingNode->next; // Set the current node to the offsetted location

              this->currentInstructionNumber = currentNode->instructionNumber + 4;
          }

          execInstr = waitInstr;
    			waitInstr = -1;

      }
      return break_flag; //won't fetch because of the stall
    }

    // cout << 3 << endl;

    while(this->Buf1_Update.size() < BUF_1_SIZE && count < FETCH_DECODE_PER_CYCLE){

      instrCount = (this->currentInstructionNumber - 256) / 4;//index of current instr
      getCurrentInstrcution(instrCount); // set currentNode to current instruction

      string currentOperation = currentNode->operation; // Get the current instruction's operation

      if(currentOperation == "J") {

          string instructionLocation = currentNode->location;
          int iil = stoi(instructionLocation); // Instruction location in integer form

          Node* startingNode = root;
          while(startingNode->next->instructionNumber != iil) { // Find the jump location
              startingNode = startingNode->next;
          }
          currentNode = startingNode; // Set the current node to the jump location

          this->currentInstructionNumber = currentNode->instructionNumber;
          this->execInstr = instrCount;

          this->currentInstructionNumber += 4;

          return break_flag;


      } else if(currentOperation == "BEQ") {

          string offset = currentNode->offset;
          int io = stoi(offset); // offset in integer form
          string rtReg = currentNode->rt;
          int irt = modRegisterValues[stoi(rtReg)]; // rt register in integer form
          string rsReg = currentNode->rs;
          int irs = modRegisterValues[stoi(rsReg)]; // rs register in integer form

          int nextInstructionNumber = currentNode->instructionNumber + io; // Get the next instruction number (to be used if rt == rs)

  				this->waitInstr = instrCount;
          this->currentInstructionNumber += 4;
          return break_flag;

      } else if(currentOperation == "BNE") {
        ////////////sdadadasdas
        string offset = currentNode->offset;
        int io = stoi(offset); // offset in integer form
        string rtReg = currentNode->rt;
        int irt = modRegisterValues[stoi(rtReg)]; // rt register in integer form
        string rsReg = currentNode->rs;
        int irs = modRegisterValues[stoi(rsReg)]; // rs register in integer form

        int nextInstructionNumber = currentNode->instructionNumber + io; // Get the next instruction number (to be used if rt == rs)

        this->waitInstr = instrCount;
        this->currentInstructionNumber += 4;
        return break_flag;
        // }

      } else if(currentOperation == "BGTZ") {

          string offset = currentNode->offset;
          int io = stoi(offset); // offset in integer form
          string rsReg = currentNode->rs;
          int irs = modRegisterValues[stoi(rsReg)]; // rs register in integer form

          int nextInstructionNumber = currentNode->instructionNumber + io; // Get the next instruction number (to be used if rs > 0)

          this->waitInstr = instrCount;
          this->currentInstructionNumber += 4;
          return break_flag;
          // }

      } else if(currentOperation == "BREAK") {
        break_flag = true;
        this->execInstr = instrCount;
				return break_flag;
      }
      else {//others can be sw, lw, sll, srl, sra, just push to preIssue
				this->Buf1_Update.push_back(instrCount);
			}
      count += 1;
      this->currentInstructionNumber += 4;

    }

    // cout << 4 << endl;
    return break_flag;
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

    int rd = stoi(this->currentLineOrInstruction.substr(6,5)); // rt register number in binary format
    int decimal = convertBinaryToDecimal(rd);
    string rd_naked = to_string(decimal);
    string rd_printable = "R" + rd_naked;

    this->decodedOutput = operation + " " + rd_printable;
    output.append(this->currentLineOrInstruction + "\t" + to_string(this->currentInstructionNumber) + "\t" + this->decodedOutput + "\n");

    Node* tempNode = new Node(this->currentInstructionNumber, operation, "", "", "", "", "", rd_naked, "", nullptr);
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
