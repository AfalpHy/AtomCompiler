grammar ATC;

compUnit: (decl | funcDef)* EOF;

decl: constDecl | varDecl;

constDecl: 'const' BType constDef (',' constDef)* ';';

constDef: Ident ( '[' constExpr ']')* '=' constInitVal;

constInitVal:
	constExpr
	| '{' ( constInitVal ( ',' constInitVal)*)? '}';

varDecl: BType varDef ( ',' varDef)* ';';

varDef:
	Ident ('[' constExpr ']')*
	| Ident ('[' constExpr ']')* '=' initVal;

initVal: expr | '{' ( initVal ( ',' initVal)*)? '}';

funcDef: FuncType Ident '(' funcFParams? ')' block;

funcFParams: funcFParam ( ',' funcFParam)*;

funcFParam: BType Ident ('[' ']' ( '[' expr ']')*)?;

block: '{' blockItem? '}';

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
	| UnaryOp unaryExpr;

UnaryOp: '+' | '-' | '!';

funcRParams: expr (',' expr)*;

mulExpr: unaryExpr (('*' | '/' | '%') unaryExpr)*;

addExpr: mulExpr ( ('+' | '-') mulExpr)*;

relExpr: addExpr ( ('<' | '>' | '<=' | '>=') addExpr)*;

eqExpr: relExpr ( ('==' | '!=') relExpr)*;

lAndExpr: eqExpr ( lAndExpr '&&' eqExpr)*;

lOrExpr: lAndExpr ( lOrExpr '||' lAndExpr)*;

constExpr: addExpr;

FuncType: 'void' | BType;

BType: 'int' | 'float';

Ident: [_a-zA-Z] [_a-zA-Z0-9]*;

IntConst: [0-9]+;

FloatConst: [0-9]+;

WS: [ \n\t]+->skip;