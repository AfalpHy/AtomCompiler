grammar ATC;

compUnit: (decl | functionDef)* EOF;

decl: varDecl | functionDecl;

cType: (Int | Float | Void) Star*;

varDecl: Const? cType varDef (Comma varDef)* Semicolon;

varDef:
	Ident (LeftBracket expr RightBracket)*
	| Ident (LeftBracket expr RightBracket)* '=' initVal;

functionDecl:
	cType Ident LeftParenthesis funcFParams? RightParenthesis Semicolon;

initVal:
	expr
	| LeftCurlyBracket (initVal ( Comma initVal)*)? RightCurlyBracket;

functionDef: cType Ident LeftParenthesis funcFParams? RightParenthesis block;

funcFParams: funcFParam ( Comma funcFParam)*;

funcFParam:
	cType Ident (LeftBracket RightBracket (LeftBracket expr RightBracket)*)?;

expr: lOrExpr;

varRef: Ident;

indexedRef: Ident (LeftBracket expr RightBracket)+;

dereference: Star expr;

number: IntConst | FloatConst;

primaryExpr:
	LeftParenthesis expr RightParenthesis
	| varRef
	| indexedRef
	| number
	| dereference;

unaryExpr:
	primaryExpr
	| Ident LeftParenthesis funcRParams? RightParenthesis
	| unaryOp unaryExpr;

unaryOp: PlusMinus | Not;

funcRParams: expr (Comma expr)*;

mulExpr: unaryExpr (mulDiv unaryExpr)*;

addExpr: mulExpr ( PlusMinus mulExpr)*;

relExpr: addExpr ( Cmp addExpr)*;

eqExpr: relExpr ( EqNe relExpr)*;

lAndExpr: eqExpr (And eqExpr)*;

lOrExpr: lAndExpr (Or lAndExpr)*;

block: LeftCurlyBracket (decl | stmt)* RightCurlyBracket;

lval: varRef | indexedRef;

stmt:
	lval '=' expr Semicolon
	| expr? Semicolon
	| block
	| If LeftParenthesis expr RightParenthesis stmt (Else stmt)?
	| While LeftParenthesis expr RightParenthesis stmt
	| Break Semicolon
	| Continue Semicolon
	| Return expr? Semicolon;

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

Comma: ',';

Semicolon: ';';

LeftParenthesis: '(';

RightParenthesis: ')';

LeftBracket: '[';

RightBracket: ']';

LeftCurlyBracket: '{';

RightCurlyBracket: '}';

Star: '*';

Not: '!';

DivMod: '/' | '%';

mulDiv: Star | DivMod;

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

fragment DecimalFloatingConst: FractionalConst Exponent? | Digit Exponent;

fragment FractionalConst: Digit? '.' Digit?;

fragment Exponent: ( 'E' | 'e') ( '+' | '-')? Digit;

fragment Digit: [0-9]+;

fragment HexFloatingConst: ('0x' | '0X') HexFractionalConst BinaryExponent?
	| ('0x' | '0X') HexDigit BinaryExponent?;

fragment HexFractionalConst: HexDigit? '.' HexDigit?;

fragment BinaryExponent: ('p' | 'P') ('+' | '-')? Digit;

fragment HexDigit: [a-fA-F0-9]+;

WS: [ \r\n\t]+ -> skip;

LineComment: '//' .*? '\r'? '\n' EOF? -> skip;
MultLineComment: '/*' .*? '*/' EOF? -> skip;