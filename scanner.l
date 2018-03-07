%{
	using namespace std;
	#include <iostream>
	#include <stdio.h>
    #include <string.h>
	#include <string>
    #include <stdlib.h>
	#include <utility>
	#include "parser.tab.h"

	extern int errors;
	extern void yyerror(const char *);
	char linebuf[500];

	int lineno = 1;

%}

%option noyywrap yylineno

%%
\n.* 							{ strcpy(linebuf, yytext+1); /* save the next line */
									lineno++;
									yyless(1);/* give back all but the \n to rescan */}
[0-9]+ 				            { yylval.int_value = atoll(yytext); return(NUMBER); }
\([^)]*[)\n]			        { /* ignore comments */ }
":="                            { return(ASSIGN); }
"="                             { return(EQ); }
"<>"                            { return(NEQ); }
"<"                             { return(LESS); }
">"                             { return(GREATER); }
"<="                            { return(LESSEQ); }
">="                            { return(GREATEREQ); }
"["                             { return *yytext; }
"]"                             { return *yytext; }
"+"                             { return *yytext; }
"/"                             { return *yytext; }
"*"                             { return *yytext; }
"%"                             { return *yytext; }
"-"                             { return *yytext; }
";"                             { return *yytext; }
"VAR"                           { return(VAR); }
"BEGIN"                         { return(BEGiN); }
"END"                           { return(END); }
"IF"                            { return(IF); }
"THEN"                          { return(THEN); }
"ELSE"                          { return(ELSE); }
"ENDIF"                         { return(ENDIF); }
"WHILE"                         { return(WHILE); }
"DO"                            { return(DO); }
"ENDWHILE"                      { return(ENDWHILE); }
"FOR"                           { return(FOR); }
"FROM"                          { return(FROM); }
"TO"                            { return(TO); }
"ENDFOR"                        { return(ENDFOR); }
"DOWNTO"                        { return(DOWNTO); }
"READ"                          { return(READ); }
"WRITE"                         { return(WRITE); }
[_a-z]+                         { yylval.id = (char *) strdup(yytext); return(PIDIDENTIFIER);}
[ \t]+                        	{ /* eat up whitespace */ }
.                               { cerr<<"Error: Unknown character "<< yytext <<endl; yyerror(" "); errors++;}
[_a-z0-9^&)(}{!@#$]+			{ cerr<<"Error: Wrong variable name"<<endl; yyerror(" "); errors++;}


%%
