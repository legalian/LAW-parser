# LAW-parser
VERY simple language with very poor error messages designed to compile to a very specific machine


The LAW parser does not recognize comments. Will in future version.
Reads the .law file and excecutes it.
Terrible things happen if syntax is done wrong; error messages are not descripive or even existant in some cases.
(this might be fixed in a future version idk currently leaving off a semicolon after a curly brace causes the whole block to be ignored.)

This language is designed to run on a custom 16 bit computer built out of IC chips and wires, and no other computer.
As such, it doesn't compile to assembly that a regular computer would run, just an array of integers that will be stored in the
breadboard computer's RAM. It's also lacking a standard input, but these limitations are only because the language has a very 
specific intented use. It's also incredibly minimalistic.

Included in this package is a breadboard computer simulator, mainly because i got sick of how long it took to test the language on the
breadboard computer each time. It excecutes the program in the exact same way the breadboard computer would, and mimics its architecture.
In some future version of this i'm going to add in an extra buffer where instead of the computer's address buffer going directly to the
multiplexer into the ram's address input, it goes into a full adder with a secondary address buffer. like 80% of the assembly this language
generates is to get around having to add the stack offset to variable addresses, and the full adder would take care of that problem.

all variables are just 16 bit words; no types here. the ? operator is used to clamp values to 1 or 0 and the ! is the inverse clamp.
technically signed comparison is implemented just not tested. it's <=$,>=$,<$,>$

syntax is as follows:

Conditional:
if () {

};
Conditional with elif-else:
if () {

} elif () {

} else {

};
Loop:
while () {

};

Variable declarations:
var i;
var o=0;

Math:
i+o;
i<<1;
i<4;
o>=6;
(other symbols include +,-,&,^,|,<<,>>,<=,>=,<,>,?,!,~,@,!=,==,=)

~ is reference, @ is dereference

Defining functions:
def whyamihere(q,y) {
  if (q<=0) {
    return 1;
  } else {
    return whyamihere(q-1,y)+y;
  };
};


Standard out:
output 789





