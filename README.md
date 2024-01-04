# AtomCompiler

A toy compiler for entertainment

# Grammar
~~~
CompUnit: (Decl | FunctionDef)*

Decl: VarDecl | FunctionDecl

CType: ('int' | 'float' | 'void') '*'*

VarDecl: Const? CType VarDef (',' VarDef)* ';'

VarDef:
	Ident ('[' Expr ']')*
	| Ident ('[' Expr ']')* '=' InitVal

InitVal:
	Expr
	| '{' (InitVal ( ',' InitVal)*)? '}'

FunctionDecl:
	CType Ident '(' FuncFParams? ')' ';'

FunctionDef: CType Ident '(' FuncFParams? ')' Block

FuncFParams: FuncFParam ( ',' FuncFParam)*

FuncFParam:
	CType Ident ('[' ']' ('[' Expr ']')*)?

VarRef: Ident

IndexedRef: Ident ('[' Expr ']')+

Dereference: '*'+ Expr

AddressOf: '&' Expr

Number: IntNumber | FloatNumber

PrimaryExpr:
	'(' Expr ')'
	| VarRef
	| IndexedRef
	| Number
	| Dereference
	| AddressOf

UnaryExpr:
	PrimaryExpr
	| Ident '(' FuncRParams? ')'
	| ('+'|'-'|'!') UnaryExpr

FuncRParams: Expr (',' Expr)*

MulExpr: UnaryExpr (('*'|'/') UnaryExpr)*

AddExpr: MulExpr (('+'|'-') MulExpr)*

RelExpr: AddExpr (('<' | '>' | '<=' | '>=') AddExpr)*

EqExpr: RelExpr (('==' | '!=') RelExpr)*

LAndExpr: EqExpr ('&&' EqExpr)*

LOrExpr: LAndExpr ('||' LAndExpr)*

Expr: LOrExpr

Block: '{' (Decl | Stmt)* '}'

Lval: VarRef | IndexedRef

Stmt:  
	Lval '=' Expr ';' 
	| Expr? ';'
	| Block
	| 'if' '(' Expr ')' Stmt ('else' Stmt)?
	| 'while' '(' Expr ')' Stmt
	| 'break' ';'
	| 'continue' ';'
	| 'return' Expr? ';'

~~~
## TODO list

`Pointer`  
`AddressOf`  
`Dereference`

# Build

## Ubuntu22.04

~~~sh
sudo apt install antlr4 libantlr4-runtime-dev llvm-dev
mkdir build
cd build
cmake -DUPDATE_ANTLR=ON ..
make -j `nproc`
~~~
