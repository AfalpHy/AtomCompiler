grammar ATC;

compUnit: (decl | funcDef)* EOF;

decl: constDecl | varDecl;

constDecl: 'const' bType constDef (',' constDef)* ';';

constDef: Ident ( '[' constExpr ']')* '=' constInitVal;

constInitVal:
	constExpr
	| '{' ( constInitVal ( ',' constInitVal)*)? '}';

varDecl: bType varDef ( ',' varDef)* ';';

varDef:
	Ident ('[' constExpr ']')*
	| Ident ('[' constExpr ']')* '=' initVal;

initVal: expr | '{' ( initVal ( ',' initVal)*)? '}';

funcDef: funcType Ident '(' funcFParams? ')' block;

funcFParams: funcFParam ( ',' funcFParam)*;

funcFParam: bType Ident ('[' ']' ( '[' expr ']')*)?;

block: '{' blockItem* '}';

blockItem: decl | stmt;

stmt:
	lval '=' expr ';'
	| expr? ';'
	| block
	| 'if' '(' cond ')' stmt ( 'else' stmt)?
	| 'while' '(' cond ')' stmt
	| 'break' ';'
	| 'continue' ';'
	| 'return' expr? ';';

expr: addExpr;

cond: lOrExpr;

lval: Ident ('[' expr ']')*;

primaryExpr: '(' expr ')' | lval | number;

number: IntConst | FloatConst;

unaryExpr:
	primaryExpr
	| Ident '(' funcRParams? ')'
	| unaryOp unaryExpr;

unaryOp: '+' | '-' | '!';

funcRParams: expr (',' expr)*;

mulExpr: unaryExpr (('*' | '/' | '%') unaryExpr)*;

addExpr: mulExpr ( ('+' | '-') mulExpr)*;

relExpr: addExpr ( ('<' | '>' | '<=' | '>=') addExpr)*;

eqExpr: relExpr ( ('==' | '!=') relExpr)*;

lAndExpr: eqExpr ('&&' eqExpr)*;

lOrExpr: lAndExpr ('||' lAndExpr)*;

constExpr: addExpr;

funcType: 'void' | bType;

bType: 'int' | 'float';

Ident: [_a-zA-Z] [_a-zA-Z0-9]*;

IntConst: DecimalConst | OctConst | HexConst;

fragment DecimalConst: [1-9] [0-9]* | '0';

fragment OctConst: '0' [0-7] [0-8]*;

fragment HexConst: ('0x' | '0X') [0-9a-fA-F]+;

FloatConst: DecimalFloatingConst | HexFloatingConst;

fragment DecimalFloatingConst:
	FractionalConst Exponent?
	| Digit Exponent;

fragment FractionalConst: Digit? '.' Digit?;

fragment Exponent: ( 'E' | 'e') ( '+' | '-')? Digit;

fragment Digit: [0-9]+;

fragment HexFloatingConst: ('0x' | '0X') HexFractionalConst BinaryExponent?
	| ('0x' | '0X') HexDigit BinaryExponent?;

fragment HexFractionalConst: HexDigit? '.' HexDigit?;

fragment BinaryExponent: ('p' | 'P') ('+' | '-')? Digit;

fragment HexDigit: [a-fA-F0-9]+;

WS: [ \r\n\t]+ -> skip;

LineComment: '//' .*? '\r'? '\n' -> skip;
MultLineComment: '/*' .*? '*/' -> skip;
