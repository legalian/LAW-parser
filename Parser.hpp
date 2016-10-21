//
//  Parser.hpp
//  LAW_parser
//
//  Created by Parker on 8/27/16.
//  Copyright © 2016 Parker. All rights reserved.
//

#ifndef Parser_hpp
#define Parser_hpp

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <fstream>

struct Scope;
struct Operation;
struct VariableReference;
struct ParenEnclosure;

struct CompLocation {
    bool isram;
    int ramlocation;
    std::string registerlocation();
    bool cares();
    CompLocation();
    CompLocation(int);
    CompLocation(std::string);
};

struct Parseable {
    virtual void LinkAnonymousReturns(int);
    virtual void push_back(Operation*,bool);
};
struct EmptyStatement : Parseable {
    Operation* bin = NULL;
    ParenEnclosure* paren = NULL;
    void LinkAnonymousReturns(int) override;
    void push_back(Operation*,bool) override;
    virtual void push_back(Scope*,std::string&);
    virtual void addelif();
    virtual void addelse();
    virtual void push_back_scope(Scope*);
    ~EmptyStatement();
};
struct ReturnStatement : EmptyStatement {
    void LinkAnonymousReturns(int) override;
};
struct OutputStatement : EmptyStatement {
    void LinkAnonymousReturns(int) override;
};
struct Declaror : EmptyStatement {
    Scope*& target;
    Declaror(Scope*&);
    void push_back(Scope*,std::string&) override;
};
struct ControlStructure : EmptyStatement {
    Parseable* captivescope;
    Operation* Rside = NULL;
    ControlStructure(Operation* rside,Parseable* captive);
    ControlStructure();
    ~ControlStructure();
};
struct Function : ControlStructure {
    std::vector<std::string> params;
    bool secure=true;
    int startingaddress;
    std::string name;
    void LinkAnonymousReturns(int) override;
    void push_back_scope(Scope*) override;
    void push_back(Scope*,std::string&) override;
};
struct Loop : ControlStructure {
    Loop(Operation* rside,Parseable* captive);
    Loop();
    void push_back_scope(Scope*) override;
    void push_back(Operation*,bool) override;
    void LinkAnonymousReturns(int) override;
};
struct Conditional : Loop {
    Conditional(Operation* rside,Parseable* captive);
    Conditional(Parseable* captive);
    Conditional(bool useless);
    Conditional* elseblock=NULL;
    void push_back_scope(Scope*) override;
    void push_back(Operation*,bool) override;
    bool isuseless = false;
    void LinkAnonymousReturns(int) override;
    void addelif() override;
    void addelse() override;
    ~Conditional();
};


struct Scope : Parseable {
    int defined_vars_this_lev = 0;
    int scope_shift = 0;
    std::map<std::string,int> vars;
    std::vector<std::string>* arguments=NULL;
    std::map<std::string,std::pair<int*,int>> functions;
    std::vector<Parseable*> statements;
    Scope(Scope*);
    Scope();
    Scope(Parseable*);
    void addVar(std::string);
    int getVar(std::string);
    void addFunc(std::string,int* addresspointer,int);
    std::pair<int*,int> getFunc(std::string);
    void LinkAnonymousReturns(int) override;
    ~Scope();
};
struct Operation : Parseable {
    int priority;
    virtual void push_back(Operation*&,Operation*,bool);
    virtual void LinkAnonymousReturns(CompLocation,int);
    virtual void LinkNamedReturns(int,int);
    virtual void adopt(Operation*);
    virtual VariableReference* AssumeVariable();
    void LinkAnonymousReturns(int) override;
    int references;
    virtual bool UsesALU();
};
struct ParenEnclosure : Operation {
    std::vector<Operation*> operations;
    bool secure = true;
    ParenEnclosure();
    ParenEnclosure* paren = NULL;
    bool push_back(Scope* self,std::string& fragment);
    void LinkAnonymousReturns(CompLocation,int) override;
    void LinkNamedReturns(int,int) override;
    ~ParenEnclosure();
};
struct Dependant : Operation {
    Operation* Rside = NULL;
    Dependant();
    Dependant(Operation* initialize);
    void push_back(Operation*&,Operation*,bool) override;
    ~Dependant();
};
struct ConstantUse : Operation {
    int data;
    ConstantUse(int constant);
    bool UsesALU() override;
    void LinkAnonymousReturns(CompLocation,int) override;
};
struct VariableReference : Dependant {
    int address;
    int* functionaddress;
    int functionarguments;
    VariableReference(int addr,int* faddr,int fargs);
    VariableReference* AssumeVariable() override;
    void push_back(Operation*&,Operation*,bool) override;
    void LinkAnonymousReturns(CompLocation,int) override;
};
struct StackIgnoreVariableReference : VariableReference {
    StackIgnoreVariableReference(int);
    bool UsesALU() override;
    void LinkAnonymousReturns(CompLocation,int) override;
};
struct Assignment : Dependant {
    int address;
    Assignment(int addr,Operation* rside);
    Assignment();
    void adopt(Operation*) override;
    void LinkAnonymousReturns(CompLocation,int) override;
};
struct GTEQComparison : Dependant {
    Operation* Lside = NULL;
    void adopt(Operation*) override;
    GTEQComparison(Operation* lside,Operation* rside);
    GTEQComparison();
    void LinkAnonymousReturns(CompLocation,int) override;
    ~GTEQComparison();
};
struct BaseOperation : GTEQComparison {
    int op_id;
    BaseOperation(int opid,Operation* lside,Operation* rside);
    BaseOperation(int opid);
    void LinkAnonymousReturns(CompLocation,int) override;
};
struct CompoundComparison : GTEQComparison {
    bool sign;
    bool equality_allowed;
    bool flip_operands;
    CompoundComparison(bool sig,bool equ,bool flip,Operation* lside,Operation* rside);
    CompoundComparison(bool sig,bool equ,bool flip);
    void LinkAnonymousReturns(CompLocation,int) override;
};
struct CompoundEquality : GTEQComparison {
    bool eq_aot_neq;
    CompoundEquality(bool agaotlt,Operation* lside,Operation* rside);
    CompoundEquality(bool agaotlt);
    void LinkAnonymousReturns(CompLocation,int) override;
};
struct Clamp : Dependant {
    bool inverted;
    Clamp(bool inv,Operation* rside);
    Clamp(bool inv);
    void LinkAnonymousReturns(CompLocation,int) override;
};
struct RefDeref : Dependant {
    bool inverted;
    RefDeref(bool inv,Operation* rside);
    RefDeref(bool inv);
    void LinkAnonymousReturns(CompLocation,int) override;
};
std::vector<std::string> split(const std::string&,char);
int stringtoassembly(const std::string &);
void DO_PARSE(std::string,int[]);
Parseable* parse(Operation*,int start);
std::string assemblytostring(int target);
#endif /* Parser_hpp */
