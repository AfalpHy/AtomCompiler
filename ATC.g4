grammar ATC;

expr:
	expr '+' expr EOF
	| expr '-' expr EOF
	| expr '*' expr EOF
	| expr '/' expr EOF
	| INT;

INT: [0-9]+;