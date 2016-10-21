//
//  Parser.cpp
//  LAW_parser
//
//  Created by Parker on 8/27/16.
//  Copyright © 2016 Parker. All rights reserved.
//

#include "Parser.hpp"



#define SETADDR(x) BaseOperation(0,new ConstantUse(x),new StackIgnoreVariableReference(1)).LinkAnonymousReturns(CompLocation("ADDR"),0)

int* instructions;
int instructionindex;
char* parse_string;

//int stringlength_under_inspect;
//instructions[0]=1

bool Operation::UsesALU() {return true;}
bool ConstantUse::UsesALU() {return false;}
bool StackIgnoreVariableReference::UsesALU() {return false;}

bool isnumeric(char test) {
    const char numbers[] = "1234567890";
    for (int k=0;k<10;k++) {
        if (test==numbers[k]) {
            return true;
        }
    }
    return false;
}
bool isletter(char test) {
    const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int k=0;k<52;k++) {
        if (test==letters[k]) {
            return true;
        }
    }
    return false;
}
int gettokenrank(std::string symbol) {
    const std::string sym_0[9][8] = {{"(",")","\n"},
        {"!","?","~","@","\n"},
        {"+","-","\n"},
        {"<<",">>","\n"},
        {"<",">","<=",">=","<$",">$","<=$",">=$"},
        {"==","!=","\n"},
        {"|","&","^","\n"},
        {"=","\n"},
        {",","\n"}};
    for (int rank=0;rank<9;rank++) {
        for (int ind=0;ind<8;ind++) {
            if (sym_0[rank][ind]=="\n") {break;}
            if (sym_0[rank][ind]==symbol) {return rank;}
        }
    }
    return -1;
}


Scope::Scope() {}
Scope::Scope(Parseable* singlestatement) {
    statements.push_back(singlestatement);
}
Scope::Scope(Scope* trickledown) {
    functions = trickledown->functions;
    if (trickledown->arguments!=NULL) {
        scope_shift = 1;
        for (int j=0;j<trickledown->arguments->size();j++) {
            addVar((*(trickledown->arguments))[j]);
        }
        trickledown->arguments=NULL;
    } else {
        vars = trickledown->vars;
        scope_shift=trickledown->scope_shift;
    }
}
void Scope::addVar(std::string newvar) {
    std::cout<<"created new variable: "<<newvar<<" at "<<scope_shift<<"\n";
    vars.insert(std::pair<std::string,int>(newvar,scope_shift));
    scope_shift++;
}
int Scope::getVar(std::string newvar) {
    return vars[newvar];
}

void Scope::addFunc(std::string newfunc,int*addresspointer,int args) {
    std::cout<<"created new function: "<<newfunc<<" with "<<args<<" arguments.\n";
    functions.insert(std::pair<std::string,std::pair<int*,int>>(newfunc,std::pair<int*,int>(addresspointer,args)));
    
}
std::pair<int*,int> Scope::getFunc(std::string func) {
    
    return functions[func];
}
void shiftToken(EmptyStatement*& tokenchain,std::string& fragment,Scope* self) {
    if (fragment.length()>0) {
        if      (fragment=="if")    {tokenchain = new Conditional(false);}
        else if (fragment=="while") {tokenchain = new Loop();}
        else if (fragment=="var")   {tokenchain = new Declaror(self);}
        
        else if (fragment=="elif")   {tokenchain->addelif();}
        else if (fragment=="else")   {tokenchain->addelse();}
        
        else if (fragment=="def")    {tokenchain = new Function();}
        else if (fragment=="return") {tokenchain = new ReturnStatement();}
        else if (fragment=="output") {tokenchain = new OutputStatement();}
        else {
            tokenchain->push_back(self,fragment);
        }
        fragment = "";
    }
}
bool iswhitespace(char possiblewhitespace) {
    return possiblewhitespace==' ' or possiblewhitespace=='\n'
        or possiblewhitespace=='\t' or possiblewhitespace=='\xb8'
        or possiblewhitespace=='\372' or possiblewhitespace=='\x02'
        or possiblewhitespace=='\310' or possiblewhitespace=='\x1a'
        or possiblewhitespace=='\x03';
}


Scope* parse(Scope* trickledown,int start) {
    int ind=start;
    int depth=0;
    
    Scope* self = new Scope(trickledown);
    EmptyStatement* tokenchain = new EmptyStatement();
    
    std::string fragment = "";
    while (depth>=0 and ind<strlen(parse_string)) {
        if (parse_string[ind]=='{') {
            if (depth++==0) {
                shiftToken(tokenchain,fragment,self);
                tokenchain->push_back_scope(parse(self,ind+1));
            }
        }
        else if (parse_string[ind]=='}') {depth--;}
        else if (depth==0) {
            if (parse_string[ind]==';') {
                shiftToken(tokenchain,fragment,self);
//                tokenchain->push_back(self,fragment);
                
                self->statements.push_back(tokenchain);
                tokenchain = new EmptyStatement();
                //statement_parse(tokens,0,(int)tokens.size());gfhjknhbiuvyfctdfuguhjnhiobugyfvtcdrujuusdsrssd
            } else if (iswhitespace(parse_string[ind])) {
                shiftToken(tokenchain,fragment,self);
                //tokenchain->push_back(self,fragment);
            } else {
                if (isnumeric(parse_string[ind])) {
                    if (fragment.length()!=0&&!isnumeric(fragment[0])&&!isletter(fragment[0])) {
                        shiftToken(tokenchain,fragment,self);
//                        tokenchain->push_back(self,fragment);
                    }
                    fragment += parse_string[ind];
                } else if (isletter(parse_string[ind])) {
                    if (fragment.length()!=0&&!isletter(fragment[0])) {
                        shiftToken(tokenchain,fragment,self);
//                        tokenchain->push_back(self,fragment);
                    }
                    fragment += parse_string[ind];
                } else {
                    if (gettokenrank(fragment+parse_string[ind])==-1) {
                        //tokenchain->push_back(self,fragment);
                        shiftToken(tokenchain,fragment,self);
                        fragment = parse_string[ind];
                        if (gettokenrank(fragment)==-1) {std::cout<<(int)parse_string[ind];throw;}
                    } else {
                        fragment += parse_string[ind];
                    }
                }
            }
        }
        ind++;
    }
    return self;
}


void DO_PARSE(std::string filepath,int* ram) {
    std::ifstream file("/Users/legalian/dev/K_parser/K_parser/"+filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    
    parse_string = new char[size];
    
    if (file.read(parse_string, size))
    {
        /* worked! */
    }
    file.close();
    
    instructions = ram;
    instructionindex = 2;
    
    Parseable* bejeezus = parse(new Scope(),0);
    
    
    instructions[0]=stringtoassembly("SET A");
    
    bejeezus->LinkAnonymousReturns(0);
    
//    instructions[instructionindex++]=stringtoassembly("SET ADDR");
//    instructions[instructionindex++]=300;
//    instructions[instructionindex++]=stringtoassembly("MOV RAM OUTPUT");
//    instructions[instructionindex++]=stringtoassembly("SET ADDR");
//    instructions[instructionindex++]=301;
//    instructions[instructionindex++]=stringtoassembly("MOV RAM OUTPUT");
//    
    instructions[instructionindex++]=stringtoassembly("STP");
    instructions[1] = instructionindex+1;
    for (int y=0;y<instructionindex;y++) {
        std::cout<<"INSTRUCTION: "<<assemblytostring(instructions[y])<<"\n";
    }
}


VariableReference* Operation::AssumeVariable() {throw;}
VariableReference* VariableReference::AssumeVariable() {return this;}

Operation* extract_op(std::string fragment,Scope* self) {
    if (fragment=="?") {return new Clamp(false);}
    if (fragment=="!") {return new Clamp(true);}
    if (fragment=="~") {return new RefDeref(false);}
    if (fragment=="@") {return new RefDeref(true);}

    if (fragment=="+")  {return new BaseOperation(0);}
    if (fragment=="-")  {return new BaseOperation(1);}
    if (fragment=="&")  {return new BaseOperation(2);}
    if (fragment=="|")  {return new BaseOperation(4);}
    if (fragment=="^")  {return new BaseOperation(5);}
    if (fragment=="<<") {return new BaseOperation(6);}
    if (fragment==">>") {return new BaseOperation(7);}
    if (fragment==">")   {return new CompoundComparison(false,false,false);}
    if (fragment=="<")   {return new CompoundComparison(false,false,true);}
    if (fragment==">=")  {return new CompoundComparison(false,true,false);}
    if (fragment=="<=")  {return new CompoundComparison(false,true,true);}
    if (fragment==">$")  {return new CompoundComparison(true,false,false);}
    if (fragment=="<$")  {return new CompoundComparison(true,false,true);}
    if (fragment==">=$") {return new CompoundComparison(true,true,false);}
    if (fragment=="<=$") {return new CompoundComparison(true,true,true);}
    if (fragment=="==") {return new CompoundEquality(true);}
    if (fragment=="!=") {return new CompoundEquality(false);}

    if (fragment=="=") {return new Assignment();}
    
    if (isnumeric(fragment[0])) {
        return new ConstantUse(std::stoi(fragment));
    }
    
    if (isletter(fragment[0])) {
        std::pair<int*,int> pair = self->getFunc(fragment);
        return new VariableReference(self->getVar(fragment),pair.first,pair.second);
    }
    throw;
}
//Parseable* extract_parse(std::string fragment,Scope* self) {
//    if (fragment=="if") {return new Conditional();}
//    if (fragment=="while") {return new Loop();}
//    if (fragment=="var") {return new Declaror(self);}
//    return extract_op(fragment,self);
//}
ParenEnclosure::ParenEnclosure() {}

bool ParenEnclosure::push_back(Scope* self,std::string& fragment) {
    if (paren!=NULL) {
        if (paren->push_back(self,fragment)) {
            if (secure) {
                operations.push_back(paren);
                secure=false;
            } else {
                operations[operations.size()-1]->push_back(operations[operations.size()-1],paren,true);
            }
            paren = NULL;
        }
    } else {
        if (fragment=="(") {
            paren = new ParenEnclosure();
        } else if (fragment==",") {
            secure = true;
        } else if (fragment==")") {
            return true;
        } else {
            if (secure) {
                
                operations.push_back(extract_op(fragment,self));
                secure=false;
            } else {
                
                operations[operations.size()-1]->push_back(operations[operations.size()-1],extract_op(fragment,self),false);
            }
        }
    }
    return false;
}

void EmptyStatement::push_back(Scope* self,std::string& fragment) {
    if (paren!=NULL) {
        if (paren->push_back(self,fragment)) {
            push_back(paren,true);
            paren = NULL;
        }
    } else {
        if (fragment=="(") {
            paren = new ParenEnclosure();
        } else {
            push_back(extract_op(fragment,self),false);
        }
    }
    fragment="";
}
void Function::push_back(Scope* self,std::string& fragment) {
    if (secure) {
        name=fragment;
        secure=false;
    }
    else if (fragment=="(" or fragment==",") {}
    else if (isletter(fragment[0])) {
        params.push_back(fragment);
    }
    else if (fragment==")") {
        self->arguments=&params;
        self->addFunc(name,&startingaddress,(int)params.size());
    }
    else {
        throw;
    }
}
void Declaror::push_back(Scope* self,std::string& fragment) {
    
    if (bin==NULL) {
        if (!isletter(fragment[0])) {throw;}
        self->addVar(fragment);
    }
    EmptyStatement::push_back(self,fragment);
//    this->EmptyStatement::push_back(self,fragment);

    //push_back(extract_op(fragment,self),false);
}

void Parseable::push_back(Operation* other,bool) {throw;}
void Parseable::LinkAnonymousReturns(int stackshift) {throw;}
void Operation::LinkAnonymousReturns(int stackshift) {LinkAnonymousReturns(CompLocation(),stackshift);}
void Operation::push_back(Operation*& self,Operation* other,bool protect) {
    self = other;
    other->adopt(this);
}
void Operation::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift) {throw;}
void Operation::adopt(Operation*) {throw;}

void EmptyStatement::addelse() {throw;}
void EmptyStatement::addelif() {throw;}
void EmptyStatement::push_back_scope(Scope*) {throw;}


void VariableReference::push_back(Operation*& self,Operation* other,bool protect) {
    if (protect and Rside==NULL) {
        Rside = other;
    } else {
        self = other;
        other->adopt(this);
    }
}

void Conditional::addelse() {
    if (elseblock==NULL) {
        elseblock = new Conditional(true);
    } else {
        elseblock->addelse();
    }
}
void Conditional::addelif() {
    if (elseblock==NULL) {
        elseblock = new Conditional(false);
    } else {
        elseblock->addelif();
    }
}
void Conditional::push_back_scope(Scope* newguy) {
    if (elseblock!=NULL) {
        elseblock->push_back_scope(newguy);
    } else {
        captivescope=newguy;
    }
}


void Function::push_back_scope(Scope* newguy) {captivescope = newguy;}
void Loop::push_back_scope(Scope* newguy) {captivescope=newguy;}



void Assignment::adopt(Operation* other) {
    address = other->AssumeVariable()->address;
}
void GTEQComparison::adopt(Operation* other) {
    Lside=other;
}

void Dependant::push_back(Operation*& self,Operation* other,bool protect) {
    
    if ((priority>other->priority)&&!protect) {
        self = other;
        other->adopt(this);
    } else {
        if (Rside==NULL) {
            Rside = other;
        } else {
            Rside->push_back(Rside,other,protect);
        }
    }
}
void Loop::push_back(Operation* other,bool protect) {
    if (!protect) {
        throw;
    } else {
        if (Rside==NULL) {
            Rside = other;
        } else {
            Rside->push_back(Rside,other,protect);
        }
    }
}
void Conditional::push_back(Operation* other,bool protect) {
    if (elseblock==NULL) {
//        if (!protect) {
//            throw;
//        } else {
        if (Rside==NULL) {
            Rside = other;
        } else {
            Rside->push_back(Rside,other,protect);
        }
//        }
    } else {
        elseblock->push_back(other,protect);
    }
}
void EmptyStatement::push_back(Operation *newguy,bool protect) {
    
    if (bin==NULL) {
        bin=newguy;
    } else {
        bin->push_back(bin,newguy,protect);
    }
}

void Operation::LinkNamedReturns(int arguments,int stackshift) {
    throw;
}
void ParenEnclosure::LinkNamedReturns(int arguments,int stackshift) {
    if (arguments!=operations.size()) {throw;}
    for (int k=0;k<operations.size();k++) {
        operations[k]->LinkAnonymousReturns(CompLocation(stackshift+k),stackshift+k);
    }
}
void EmptyStatement::LinkAnonymousReturns(int stackshift) {
    bin->LinkAnonymousReturns(stackshift);
}
void ReturnStatement::LinkAnonymousReturns(int stackshift) {
    bin->LinkAnonymousReturns(CompLocation("D"),stackshift);
    SETADDR(0);
    instructions[instructionindex++]=stringtoassembly("MOV RAM COUNTER");
    
}
void OutputStatement::LinkAnonymousReturns(int stackshift) {
    bin->LinkAnonymousReturns(CompLocation("OUTPUT"),stackshift);
}

void ParenEnclosure::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift){
    operations[0]->LinkAnonymousReturns(desiredreturn,stackshift);
    for (int k=1;k<operations.size();k++) {
        operations[k]->LinkAnonymousReturns(stackshift);
    }
}

void Assignment::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift){
    Rside->LinkAnonymousReturns(CompLocation(address),stackshift);
    if (!desiredreturn.cares()) {return;}
    if (desiredreturn.isram) {
        if (address!=desiredreturn.ramlocation) {
            instructions[instructionindex++]=stringtoassembly("MOV RAM D");
            //instructions[instructionindex++]=stringtoassembly("SET ADDR");
            //instructions[instructionindex++]=desiredreturn.ramlocation;
            SETADDR(desiredreturn.ramlocation);
            instructions[instructionindex++]=stringtoassembly("MOV D RAM");
        }
    } else {
        instructions[instructionindex++]=stringtoassembly("MOV RAM "+desiredreturn.registerlocation());
    }
}
void VariableReference::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift){
    
    if (Rside==NULL) {
        if (!desiredreturn.cares()) {return;}
        if (desiredreturn.isram) {
            if (address!=desiredreturn.ramlocation) {
                SETADDR(address);
                instructions[instructionindex++]=stringtoassembly("MOV RAM D");
                SETADDR(desiredreturn.ramlocation);
                instructions[instructionindex++]=stringtoassembly("MOV D RAM");
            }
        } else {
            SETADDR(address);
            instructions[instructionindex++]=stringtoassembly("MOV RAM "+desiredreturn.registerlocation());
        }
    } else {
        
        Rside->LinkNamedReturns(functionarguments,stackshift+1);
        instructions[instructionindex++]=stringtoassembly("SET ADDR");
        instructions[instructionindex++]=1;
        instructions[instructionindex++]=stringtoassembly("MOV RAM A");
        instructions[instructionindex++]=stringtoassembly("SET B");
        instructions[instructionindex++]=stackshift;
        instructions[instructionindex++]=stringtoassembly("ALU 0");
        instructions[instructionindex++]=stringtoassembly("MOV C RAM");
        
        
        SETADDR(0);
        instructions[instructionindex++]=stringtoassembly("SET RAM");
        int laterset = instructionindex++;
        
//        instructions[instructionindex++]=stringtoassembly("SET OUTPUT");
//        instructions[instructionindex++]=*functionaddress;
//        std::cout<<"jumped to: "<<*functionaddress<<"\n";
        instructions[instructionindex++]=stringtoassembly("SET COUNTER");
        instructions[instructionindex++]=*functionaddress;
        instructions[laterset]=instructionindex;
        
        
        instructions[instructionindex++]=stringtoassembly("SET ADDR");
        instructions[instructionindex++]=1;
        instructions[instructionindex++]=stringtoassembly("MOV RAM A");
        instructions[instructionindex++]=stringtoassembly("SET B");
        instructions[instructionindex++]=stackshift;
        instructions[instructionindex++]=stringtoassembly("ALU 1");
        instructions[instructionindex++]=stringtoassembly("MOV C RAM");
        
        if (desiredreturn.cares()) {
            if (desiredreturn.isram) {
                SETADDR(desiredreturn.ramlocation);
                instructions[instructionindex++]=stringtoassembly("MOV D RAM");
            } else {
                if (desiredreturn.registerlocation()!="D") {
                    instructions[instructionindex++]=stringtoassembly("MOV D "+desiredreturn.registerlocation());
                }
            }
        }
    }
}

StackIgnoreVariableReference::StackIgnoreVariableReference(int addr) : VariableReference(addr,0,0) {}

void StackIgnoreVariableReference::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift) {
    instructions[instructionindex++]=stringtoassembly("SET ADDR");
    instructions[instructionindex++]=1;
    
    instructions[instructionindex++]=stringtoassembly("MOV RAM "+desiredreturn.registerlocation());
}

void ConstantUse::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift){
    if (!desiredreturn.cares()) {return;}
    if (desiredreturn.isram) {
//        instructions[instructionindex++]=stringtoassembly("SET ADDR");
//        instructions[instructionindex++]=desiredreturn.ramlocation;
        SETADDR(desiredreturn.ramlocation);
        instructions[instructionindex++]=stringtoassembly("SET RAM");
        instructions[instructionindex++]=data;
    } else {
        instructions[instructionindex++]=stringtoassembly("SET "+desiredreturn.registerlocation());
        instructions[instructionindex++]=data;
    }
}
void GTEQComparison::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift){
    if (!desiredreturn.cares()) {
        Lside->LinkAnonymousReturns(CompLocation(),stackshift);
        Rside->LinkAnonymousReturns(CompLocation(),stackshift);
        return;
    }
    
    bool l_uses_alu = Lside->UsesALU();//!(typeid(Lside) == typeid(VariableReference) or typeid(Lside) == typeid(ConstantUse));
    bool r_uses_alu = Rside->UsesALU();//!(typeid(Rside) == typeid(VariableReference) or typeid(Rside) == typeid(ConstantUse));
    
    if (r_uses_alu) {
        if (l_uses_alu) {
            Lside->LinkAnonymousReturns(CompLocation(stackshift),stackshift);
            Rside->LinkAnonymousReturns(CompLocation("D"),stackshift+1);
            SETADDR(stackshift);
            instructions[instructionindex++]=stringtoassembly("MOV RAM A");
            instructions[instructionindex++]=stringtoassembly("MOV D B");
        } else {
            Rside->LinkAnonymousReturns(CompLocation("B"),stackshift);
            Lside->LinkAnonymousReturns(CompLocation("A"),stackshift);
        }
    } else {
        Lside->LinkAnonymousReturns(CompLocation("A"),stackshift);
        Rside->LinkAnonymousReturns(CompLocation("B"),stackshift);
    }
    instructions[instructionindex++]=stringtoassembly("ALU 1");
    if (desiredreturn.isram) {
        instructions[instructionindex++]=stringtoassembly("MOV TRASH D");
        SETADDR(desiredreturn.ramlocation);
        instructions[instructionindex++]=stringtoassembly("MOV D RAM");
    } else {
        if (desiredreturn.registerlocation()!="TRASH") {
            instructions[instructionindex++]=stringtoassembly("MOV TRASH "+desiredreturn.registerlocation());
        }
    }
}
void BaseOperation::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift) {
    if (!desiredreturn.cares()) {
        Lside->LinkAnonymousReturns(CompLocation(),stackshift);
        Rside->LinkAnonymousReturns(CompLocation(),stackshift);
        return;
    }
    bool l_uses_alu = Lside->UsesALU();//!(typeid(Lside) == typeid(VariableReference) or typeid(Lside) == typeid(ConstantUse));
    bool r_uses_alu = Rside->UsesALU();//!(typeid(Rside) == typeid(VariableReference) or typeid(Rside) == typeid(ConstantUse));
    
    if (r_uses_alu) {
        if (l_uses_alu) {
            Lside->LinkAnonymousReturns(CompLocation(stackshift),stackshift);
            Rside->LinkAnonymousReturns(CompLocation("D"),stackshift+1);
            SETADDR(stackshift);
            instructions[instructionindex++]=stringtoassembly("MOV RAM A");
            instructions[instructionindex++]=stringtoassembly("MOV D B");
        } else {
            Rside->LinkAnonymousReturns(CompLocation("B"),stackshift);
            Lside->LinkAnonymousReturns(CompLocation("A"),stackshift);
        }
    } else {
        Lside->LinkAnonymousReturns(CompLocation("A"),stackshift);
        Rside->LinkAnonymousReturns(CompLocation("B"),stackshift);
    }
    instructions[instructionindex++]=stringtoassembly("ALU " + std::to_string(op_id));
    if (desiredreturn.isram) {
//        instructions[instructionindex++]=stringtoassembly("SET ADDR");
//        instructions[instructionindex++]=desiredreturn.ramlocation;
        instructions[instructionindex++]=stringtoassembly("MOV C D");
        SETADDR(desiredreturn.ramlocation);
        instructions[instructionindex++]=stringtoassembly("MOV D RAM");
    } else {
        if (desiredreturn.registerlocation()!="C") {
            instructions[instructionindex++]=stringtoassembly("MOV C "+desiredreturn.registerlocation());
        }
    }
}
void CompoundComparison::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift) {
    std::cout<<equality_allowed<<" "<<flip_operands<<" "<<sign<<"\n";
    if (flip_operands) {
        flip_operands = false;
        Operation* swi = Rside;
        Rside=Lside;
        Lside=swi;
    }
    if (sign) {
        if (equality_allowed) {
            Clamp(false,new BaseOperation(2,new ConstantUse(32768),new BaseOperation(1,Rside,Lside))).LinkAnonymousReturns(desiredreturn,stackshift);
        } else {
            Clamp(true ,new BaseOperation(2,new ConstantUse(32768),new BaseOperation(1,Lside,Rside))).LinkAnonymousReturns(desiredreturn,stackshift);
        }
    } else {
        if (equality_allowed) {
            GTEQComparison(Lside,Rside).LinkAnonymousReturns(desiredreturn,stackshift);
        } else {
            BaseOperation(5,new ConstantUse(1),new GTEQComparison(Rside,Lside)).LinkAnonymousReturns(desiredreturn,stackshift);
        }
    }
}
void CompoundEquality::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift) {
    if (eq_aot_neq) {
        Clamp(true ,new BaseOperation(5,Lside,Rside)).LinkAnonymousReturns(desiredreturn,stackshift);
    } else {
        Clamp(false,new BaseOperation(5,Lside,Rside)).LinkAnonymousReturns(desiredreturn,stackshift);
    }
}
void Clamp::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift) {
    if (!desiredreturn.cares()) {
        Rside->LinkAnonymousReturns(desiredreturn,stackshift);
        return;
    }
    if (desiredreturn.isram) {
        Conditional within = Conditional(Rside,new Assignment(desiredreturn.ramlocation,new ConstantUse((int)!inverted)));
        within.addelse();
        within.push_back_scope(new Scope(new Assignment(desiredreturn.ramlocation,new ConstantUse((int)inverted))));
        within.LinkAnonymousReturns(stackshift);
    } else {
        Conditional within = Conditional(Rside,new Assignment(stackshift,new ConstantUse((int)!inverted)));
        within.addelse();
        within.push_back_scope(new Scope(new Assignment(stackshift,new ConstantUse((int)inverted))));
        within.LinkAnonymousReturns(stackshift+1);
        instructions[instructionindex++]=stringtoassembly("MOV RAM "+desiredreturn.registerlocation());
    }
}
void RefDeref::LinkAnonymousReturns(CompLocation desiredreturn,int stackshift) {
    if (!desiredreturn.cares()) {
        Rside->LinkAnonymousReturns(desiredreturn,stackshift);
        return;
    }
    if (inverted) {
        Rside->LinkAnonymousReturns(CompLocation("ADDR"),stackshift);
        if (desiredreturn.isram) {
            instructions[instructionindex++]=stringtoassembly("MOV RAM D");
            SETADDR(desiredreturn.ramlocation);
            instructions[instructionindex++]=stringtoassembly("MOV D RAM");
        } else {
            instructions[instructionindex++]=stringtoassembly("MOV RAM "+desiredreturn.registerlocation());
        }
    } else {
        VariableReference* varguy = Rside->AssumeVariable();
        if (desiredreturn.isram) {
            SETADDR(desiredreturn.ramlocation);
            instructions[instructionindex++]=stringtoassembly("SET RAM");
        } else {
            instructions[instructionindex++]=stringtoassembly("SET "+desiredreturn.registerlocation());
        }
        instructions[instructionindex++]=varguy->address;
        delete varguy;
    }
}
void Function::LinkAnonymousReturns(int stackshift) {
    
//    instructions[instructionindex++]=stringtoassembly("MOV COUNTER OUTPUT");
    instructions[instructionindex++]=stringtoassembly("SET COUNTER");
    int laterset = instructionindex++;
    startingaddress = instructionindex;
    captivescope->LinkAnonymousReturns(stackshift);
    SETADDR(0);
    instructions[instructionindex++]=stringtoassembly("MOV RAM COUNTER");
//    ReturnStatement* toreturn = new ReturnStatement();
//    toreturn->bin = new ConstantUse(0);
//    toreturn->LinkAnonymousReturns(stackshift);
    
    
    instructions[laterset] = instructionindex;
//    instructions[instructionindex++]=stringtoassembly("MOV COUNTER OUTPUT");
}
void Loop::LinkAnonymousReturns(int stackshift) {
    
    int loopbegin = instructionindex;
//    if (checkafter) {
//    captivescope->LinkAnonymousReturns(stackshift);
//    }
    Rside->LinkAnonymousReturns(CompLocation(stackshift),stackshift);
    instructions[instructionindex++]=stringtoassembly("SET D");
    int laterset = instructionindex++;
    instructions[instructionindex++]=stringtoassembly("SKP RAM");
    instructions[instructionindex++]=stringtoassembly("MOV D COUNTER");
//    if (checkafter) {
//        instructions[instructionindex++]=stringtoassembly("SET COUNTER");
//        instructions[instructionindex++]=loopbegin;
//    } else {
    captivescope->LinkAnonymousReturns(stackshift);
    instructions[instructionindex++]=stringtoassembly("SET COUNTER");
    instructions[instructionindex++]=loopbegin;
//    }
    instructions[laterset]=instructionindex;
}
void Conditional::LinkAnonymousReturns(int stackshift) {
    if (isuseless) {
        captivescope->LinkAnonymousReturns(stackshift);
        return;
    }
    Rside->LinkAnonymousReturns(CompLocation(stackshift),stackshift);
    instructions[instructionindex++]=stringtoassembly("SET D");
    int laterset = instructionindex++;
    instructions[instructionindex++]=stringtoassembly("SKP RAM");
    instructions[instructionindex++]=stringtoassembly("MOV D COUNTER");
    captivescope->LinkAnonymousReturns(stackshift);
    if (elseblock!=NULL) {
        instructions[instructionindex++]=stringtoassembly("SET COUNTER");
        int laterset2 = instructionindex++;
        instructions[laterset]=instructionindex;
        elseblock->LinkAnonymousReturns(stackshift);
        instructions[laterset2]=instructionindex;
    } else {
        instructions[laterset]=instructionindex;
    }
}

void Scope::LinkAnonymousReturns(int stackshift) {
    
    for (int ind=0;ind<statements.size();ind++) {
        statements[ind]->LinkAnonymousReturns(stackshift+scope_shift);
    }
}


ConstantUse::ConstantUse(int constant) :
data(constant) {}
VariableReference::VariableReference(int addr,int* faddr,int fargs) :
address(addr),functionaddress(faddr),functionarguments(fargs) {}
Assignment::Assignment(int addr,Operation* rside) :
Dependant(rside),address(addr) {}
GTEQComparison::GTEQComparison(Operation* lside,Operation* rside) :
Lside(lside),Dependant(rside) {}
BaseOperation::BaseOperation(int opid,Operation* lside,Operation* rside) :
GTEQComparison(lside,rside),op_id(opid) {}
CompoundComparison::CompoundComparison(bool sig,bool equ,bool flipped,Operation* lside,Operation* rside) :
GTEQComparison(lside,rside),sign(sig),equality_allowed(equ),flip_operands(flipped) {}
CompoundEquality::CompoundEquality(bool eqaotneq,Operation* lside,Operation* rside) :
GTEQComparison(lside,rside),eq_aot_neq(eqaotneq) {}
Clamp::Clamp(bool inv,Operation* rside) :
inverted(inv),Dependant(rside) {}
RefDeref::RefDeref(bool inv,Operation* rside) :
inverted(inv),Dependant(rside) {}
//FunctionCall::FunctionCall(int location) :
//address(location) {}
ControlStructure::ControlStructure(Operation* rside,Parseable* captive) :
Rside(rside),captivescope(captive) {}
Loop::Loop(Operation* rside,Parseable* captive) :
ControlStructure(rside,captive) {}
Conditional::Conditional(Operation* rside,Parseable* captive) :
Loop(rside,captive) {}
Conditional::Conditional(Parseable* captive) :
Loop(new Operation(),captive),isuseless(true) {}

Declaror::Declaror(Scope*& bla) : target(bla) {}
Assignment::Assignment() {}
GTEQComparison::GTEQComparison() {}
BaseOperation::BaseOperation(int opid) : GTEQComparison(),op_id(opid) {}
CompoundComparison::CompoundComparison(bool sig,bool equ,bool flipped) : GTEQComparison(),sign(sig),equality_allowed(equ),flip_operands(flipped) {}
CompoundEquality::CompoundEquality(bool eqaotneq) : GTEQComparison(),eq_aot_neq(eqaotneq) {}
Clamp::Clamp(bool inv) : inverted(inv) {}
RefDeref::RefDeref(bool inv) : inverted(inv) {}
//FunctionCall::FunctionCall(int location) : address(location) {};
ControlStructure::ControlStructure() {}
Loop::Loop() : ControlStructure() {}
Conditional::Conditional(bool useless) : isuseless(useless), Loop() {}

Dependant::Dependant() {}
Dependant::Dependant(Operation* initialize) : Rside(initialize) {}

EmptyStatement::~EmptyStatement() {
//    delete paren;
//    delete bin;
}
ControlStructure::~ControlStructure() {
//    delete captivescope;
//    delete Rside;
}
Conditional::~Conditional() {
//    delete elseblock;
}
Scope::~Scope() {
//    for (int y=0;y<statements.size();y++) {
//        delete statements[y];
//    }
}
ParenEnclosure::~ParenEnclosure() {
//    for (int y=0;y<operations.size();y++) {
//        delete operations[y];
//    }
}
Dependant::~Dependant() {
//    delete Rside;
}
GTEQComparison::~GTEQComparison() {
//    delete Lside;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

int stringtoassembly(const std::string &keywords) {
    std::vector<std::string> magicwords = split(keywords,' ');
    
    int header=0;
    int outputbuf=0;
    int inputbuf=0;
    
    if (magicwords[0]=="ALU") {
        return std::stoi(magicwords[1]);
    }
    else if (magicwords[0]=="SKP") {
        header = 1;
        if (magicwords[1]=="TRASH") {inputbuf=0;}
        if (magicwords[1]=="C")     {inputbuf=1;}
        if (magicwords[1]=="D")     {inputbuf=2;}
        if (magicwords[1]=="RAM")   {inputbuf=3;}
    }
    else if (magicwords[0]=="SET") {
        header = 2;
        if (magicwords[1]=="COUNTER") {outputbuf=0;}
        if (magicwords[1]=="A")       {outputbuf=1;}
        if (magicwords[1]=="B")       {outputbuf=2;}
        if (magicwords[1]=="D")       {outputbuf=3;}
        if (magicwords[1]=="ADDR")    {outputbuf=4;}
        if (magicwords[1]=="RAM")     {outputbuf=5;}
        if (magicwords[1]=="OUTPUT")  {outputbuf=6;}
    }
    else if (magicwords[0]=="MOV") {
        header = 3;
        if (magicwords[1]=="TRASH") {inputbuf=0;}
        if (magicwords[1]=="C")     {inputbuf=1;}
        if (magicwords[1]=="D")     {inputbuf=2;}
        if (magicwords[1]=="RAM")   {inputbuf=3;}
        
        if (magicwords[2]=="COUNTER") {outputbuf=0;}
        if (magicwords[2]=="A")       {outputbuf=1;}
        if (magicwords[2]=="B")       {outputbuf=2;}
        if (magicwords[2]=="D")       {outputbuf=3;}
        if (magicwords[2]=="ADDR")    {outputbuf=4;}
        if (magicwords[2]=="RAM")     {outputbuf=5;}
        if (magicwords[2]=="OUTPUT")  {outputbuf=6;}
    }
    else if (magicwords[0]=="STP") {
        header = 4;
    }
    return (header<<8)+(outputbuf<<4)+inputbuf;
}
bool IsParam=false;
std::string assemblytostring(int target) {
    if (IsParam) {IsParam=false;return std::to_string(target);}
    int header = (target&65280)>>8;
    int outputbuf = (target&240)>>4;
    int inputbuf = target&15;
    int ALUinstruction = target;
    if (header == 0) {
        if (ALUinstruction == 0) {return "PERFORM ADDITION";}
        if (ALUinstruction == 1) {return "PERFORM SUBTRACTION";}
        if (ALUinstruction == 2) {return "PERFORM BITWISE AND";}
        if (ALUinstruction == 3) {return "PERFORM BITWISE AND";}
        if (ALUinstruction == 4) {return "PERFORM BITWISE OR";}
        if (ALUinstruction == 5) {return "PERFORM BITWISE XOR";}
        if (ALUinstruction == 6) {return "PERFORM BITWISE LSHIFT";}
        if (ALUinstruction == 7) {return "PERFORM BITWISE RSHIFT";}
    }
    else if (header == 1) {
        if (inputbuf==0) {return "SKP FROM TRASH";}
        if (inputbuf==1) {return "SKP FROM C";}
        if (inputbuf==2) {return "SKP FROM D";}
        if (inputbuf==3) {return "SKP FROM RAM";}
    }
    else if (header == 2) {
        IsParam=true;
        if (outputbuf==0) {return "SET TO COUNTER";}
        if (outputbuf==1) {return "SET TO A";}
        if (outputbuf==2) {return "SET TO B";}
        if (outputbuf==3) {return "SET TO D";}
        if (outputbuf==4) {return "SET TO ADDR";}
        if (outputbuf==5) {return "SET TO RAM";}
        if (outputbuf==6) {return "SET TO OUTPUT";}
    }
    else if (header == 3) {
        
        std::string toreturn = "MOV ";
        if      (inputbuf==0) {toreturn += "TRASH";}
        else if (inputbuf==1) {toreturn += "C";}
        else if (inputbuf==2) {toreturn += "D";}
        else if (inputbuf==3) {toreturn += "RAM";}
        else {throw;}
        toreturn += " TO ";
        if      (outputbuf==0) {toreturn += "COUNTER";}
        else if (outputbuf==1) {toreturn += "A";}
        else if (outputbuf==2) {toreturn += "B";}
        else if (outputbuf==3) {toreturn += "D";}
        else if (outputbuf==4) {toreturn += "ADDR";}
        else if (outputbuf==5) {toreturn += "RAM";}
        else if (outputbuf==6) {toreturn += "OUTPUT";}
        else {throw;}
        return toreturn;
    }
    else if (header == 4) {
        return "STOP ALL EXECUTION";
    }
    throw;
}


bool CompLocation::cares() {return isram||ramlocation!=10;}
std::string CompLocation::registerlocation() {
    if      (ramlocation==0) {return "TRASH";}
    else if (ramlocation==1) {return "C";}
    else if (ramlocation==2) {return "D";}
    else if (ramlocation==3) {return "RAM";}
    else if (ramlocation==4) {return "COUNTER";}
    else if (ramlocation==5) {return "A";}
    else if (ramlocation==6) {return "B";}
    else if (ramlocation==7) {return "ADDR";}
    else if (ramlocation==8) {return "RAM";}
    else if (ramlocation==9) {return "OUTPUT";}
    else {throw;}
}
CompLocation::CompLocation():                     isram(false),ramlocation(10) {}
CompLocation::CompLocation(int location):         isram(true),ramlocation(location) {}
CompLocation::CompLocation(std::string location): isram(false) {
    
    if      (location=="TRASH")   {ramlocation=0;}
    else if (location=="C")       {ramlocation=1;}
    else if (location=="D")       {ramlocation=2;}
    else if (location=="RAM")     {ramlocation=3;}
    else if (location=="COUNTER") {ramlocation=4;}
    else if (location=="A")       {ramlocation=5;}
    else if (location=="B")       {ramlocation=6;}
    else if (location=="ADDR")    {ramlocation=7;}
    else if (location=="RAM")     {ramlocation=8;}
    else if (location=="OUTPUT")  {ramlocation=9;}
    else {throw;}
}













