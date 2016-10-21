//
//  main.cpp
//  LAW_parser
//
//  Created by Parker on 8/27/16.
//  Copyright © 2016 Parker. All rights reserved.
//

#include <iostream>
#include "Parser.hpp"



int main() {
    int RAM[65536];
    int ADDR = 0;
    int A = 0;
    int B = 0;
    int C = 0;
    int D = 0;
    int TRASH = 0;
    int COUNTER = 0;
    DO_PARSE("Target.law",RAM);
    
    std::cout<<"parse done!\n";
    
#define DEBUGEXECUTE false
    while (true) {
        if (COUNTER>65535) {throw;}
        int header = (RAM[COUNTER]&65280)>>8;
        int outputbuf = (RAM[COUNTER]&240)>>4;
        int inputbuf = RAM[COUNTER]&15;
        int ALUinstruction = RAM[COUNTER];
        if (DEBUGEXECUTE) {std::cout<<"EXECUTED "<<assemblytostring(RAM[COUNTER])<<"\n";}
//        assemblytostring(RAM[COUNTER]);
        COUNTER++;
        if (header == 0) {
            
            if (ALUinstruction == 0) {C = A+B;}
            if (ALUinstruction == 1) {C = A-B;TRASH=A>=B;}
            if (ALUinstruction == 2) {C = A&B;}
            if (ALUinstruction == 3) {C = A&B;}
            if (ALUinstruction == 4) {C = A|B;}
            if (ALUinstruction == 5) {C = A^B;}
            if (ALUinstruction == 6) {C = A<<B;}
            if (ALUinstruction == 7) {C = A>>B;}
        }
        else if (header == 1) {
            
            int listento = 0;
            if (inputbuf==0) {listento=TRASH;}
            if (inputbuf==1) {listento=C;}
            if (inputbuf==2) {listento=D;}
            if (inputbuf==3) {listento=RAM[ADDR];if (ADDR>65535) {throw;}}
            if (listento!=0) {
                COUNTER++;
            }
        }
        else if (header == 2) {
            
            int sending = RAM[COUNTER++];
            if (DEBUGEXECUTE) {std::cout<<"EXECUTED "<<assemblytostring(sending)<<"\n";}
            if (outputbuf==0) {COUNTER=sending;}
            if (outputbuf==1) {A=sending;}
            if (outputbuf==2) {B=sending;}
            if (outputbuf==3) {D=sending;}
            if (outputbuf==4) {ADDR=sending;}
            if (outputbuf==5) {RAM[ADDR]=sending;if (ADDR>65535) {throw;}}
            if (outputbuf==6) {std::cout<<sending<<"\n";}
        }
        else if (header == 3) {
            
            int movingvalue = 0;
            if (inputbuf==0) {movingvalue=TRASH;}
            if (inputbuf==1) {movingvalue=C;}
            if (inputbuf==2) {movingvalue=D;}
            if (inputbuf==3) {movingvalue=RAM[ADDR];if (ADDR>65535) {throw;}}
            if (outputbuf==0) {COUNTER=movingvalue;}
            if (outputbuf==1) {A=movingvalue;}
            if (outputbuf==2) {B=movingvalue;}
            if (outputbuf==3) {D=movingvalue;}
            if (outputbuf==4) {ADDR=movingvalue;}
            if (outputbuf==5) {RAM[ADDR]=movingvalue;}
            if (outputbuf==6) {std::cout<<movingvalue<<"\n";}
        }
        else if (header == 4) {
            break;
        }
    }
    return 0;
}
