grammar ATC;

compUnit: (decl | functionDef)* EOF;

decl: varDecl;

varDecl: Const? (Int | Float) varDef ( ',' varDef)* ';';

varDef:
	Ident ('[' expr ']')*
	| Ident ('[' expr ']')* '=' initVal;

initVal: expr | '{' ( initVal ( ',' initVal)*)? '}';

functionDef: (Int | Float | Void) Ident '(' funcFParams? ')' block;

funcFParams: funcFParam ( ',' funcFParam)*;

funcFParam: (Int | Float) Ident ('[' ']' ( '[' expr ']')*)?;

block: '{' (decl | stmt)* '}';

stmt:
	varRef '=' expr ';'
	| expr? ';'
	| block
	| If '(' cond ')' stmt ( Else stmt)?
	| While '(' cond ')' stmt
	| Break ';'
	| Continue ';'
	| Return expr? ';';

expr: lOrExpr;

cond: lOrExpr;

varRef: Ident ('[' expr ']')*;

primaryExpr: '(' expr ')' | varRef | number;

number: IntConst | FloatConst;

unaryExpr:
	primaryExpr
	| Ident '(' funcRParams? ')'
	| unaryOp unaryExpr;

unaryOp: PlusMinus | '!';

funcRParams: expr (',' expr)*;

mulExpr: unaryExpr (MulDIV unaryExpr)*;

addExpr: mulExpr ( PlusMinus mulExpr)*;

relExpr: addExpr ( Cmp addExpr)*;

eqExpr: relExpr ( EqNe relExpr)*;

lAndExpr: eqExpr (And eqExpr)*;

lOrExpr: lAndExpr (Or lAndExpr)*;

Const: 'const';

Int: 'int';

Float: 'float';

Void: 'void';

If: 'if';

Else: 'else';

While: 'while';

Break: 'break';

Continue: 'continue';

Return: 'return';

MulDIV: '*' | '/' | '%';

PlusMinus: '+' | '-';

Cmp: '<' | '>' | '<=' | '>=';

EqNe: '==' | '!=';

And: '&&';

Or: '||';

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