%{
    using namespace std;

    #include <iostream>
    #include "termcolor.hpp"
    #include <stdio.h>
    #include <utility>
    #include <math.h>
    #include <stdlib.h>
    #include <string.h>
    #include <string>
    #include <vector>
    #include "ST.h"
    #include "CG.h"


    void yyerror(const char *);
    int yylex(void);
    int errors = 0;

    extern int lineno;
    extern char linebuf[500];

    void install_variable(string name, int size);
    int check_variable(string id, int index);
    int check_initialization(string id);
    int check_modification_possibility(string id);

%}
/* Bison declarations. */

%union{
    char * id; /* For returning identifiers */
    long long int_value;
    struct {
        int primary_location;
        int array_location;
        bool need_init_check;
        bool external_var;
        char * var_id;
    }locs;
    int condition_location;
    int value_location;

    struct {
        int jump_loc;
        int acc_loc;
    } for_locs;
}


%locations

%start program

%token <int_value> NUMBER
%token <id> PIDIDENTIFIER
%token VAR BEGiN END
%token IF THEN ELSE ENDIF
%token WHILE DO ENDWHILE
%token FOR FROM TO ENDFOR DOWNTO
%token READ WRITE
%token ASSIGN EQ NEQ LESS GREATER LESSEQ GREATEREQ


%left '-' '+'
%left '*' '/'


%type <locs> identifier value expression for_variable
%type <condition_location> condition
%type <int_value> Else While
%type <value_location> for_loop_value
%type <for_locs> Do

%%
/* Grammar rules. */

program         :
                VAR vdeclarations BEGiN commands END
                ;

vdeclarations : vdeclarations PIDIDENTIFIER { install_variable($2,0); }
               | vdeclarations PIDIDENTIFIER'['NUMBER']' { install_variable($2,$4);}
               | error
               |
               ;

commands       : commands command
                | command {yyerrok;}
                ;

command         : identifier ASSIGN expression';'{
                    if( !$<locs.external_var>1 ) {
                        set_initialized_var($<locs.var_id>1);
                    };
                    if($<locs.need_init_check>1 && $<locs.external_var>1)
                        check_initialization($<locs.var_id>1);
                    if($<locs.need_init_check>3)
                        check_initialization($<locs.var_id>3);

                    if(!$<locs.external_var>1) {
                        check_modification_possibility($<locs.var_id>1);
                    }

                    assign($<locs.primary_location>3,$<locs.array_location>3,
                    $<locs.primary_location>1, $<locs.array_location>1);

                }
                | IF condition THEN commands Else commands Endif {
                    place_jzero_argument($5);
                }
                | IF condition THEN commands Endif {
                    place_jzero_argument(external_code.size());
                }
                | While condition DO commands Endwhile {
                    place_jump_argument($1);
                    place_jzero_argument(external_code.size());
                }
                | FOR for_variable FROM for_loop_value TO for_loop_value Do commands ENDFOR {
                    int for_var = $<locs.primary_location>2;
                    string for_var_id= $<locs.var_id>2;
                    int value1 = $4;
                    int value2 = $6;
                    int acc = $<for_locs.acc_loc>7;
                    int first_instuction_jump = $<for_locs.jump_loc>7;

                     inc_for_loop_iterator(for_var);
                     dec_for_loop_accumulator(acc);

                     //fill placeholders
                     place_for_loop_arguments(first_instuction_jump, value1, value2, for_var );

                     delete_var(for_var_id);
                 }
                 | FOR for_variable FROM for_loop_value DOWNTO for_loop_value Do commands ENDFOR {
                     int for_var = $<locs.primary_location>2;
                     string for_var_id= $<locs.var_id>2;
                     int value1 = $4;
                     int value2 = $6;
                     int acc = $<for_locs.acc_loc>7;
                     int first_instuction_jump = $<for_locs.jump_loc>7;

                      dec_for_loop_iterator(for_var);
                      dec_for_loop_accumulator(acc);

                      place_for_loop_arguments_downto_version(first_instuction_jump, value1, value2, for_var );

                      delete_var(for_var_id);
                 }
                 | READ identifier';' {
                     if(!$<locs.external_var>2) {
                         set_initialized_var($<locs.var_id>2);
                     }
                     int init_ok = 1;
                     if($<locs.need_init_check>2 && $<locs.external_var>2){
                        init_ok = check_initialization($<locs.var_id>2);
                     }
                     if(!$<locs.external_var>2){
                         check_modification_possibility($<locs.var_id>2);
                     }
                     if(init_ok == 1){
                         read($<locs.primary_location>2,$<locs.array_location>2);
                     }
                 }
                 | WRITE value';' {
                     if($<locs.need_init_check>2){
                         check_initialization($<locs.var_id>2);
                     }
                     write($<locs.primary_location>2,$<locs.array_location>2);}
                 | error
                 ;

Else             : ELSE {  place_jump(); $$ = external_code.size(); };
Endif            : ENDIF{ place_jump_argument(external_code.size());};
While            : WHILE { $$ = external_code.size(); };
Endwhile         : ENDWHILE { place_jump(); };


for_variable     : PIDIDENTIFIER{
                     install_variable($1,0);
                     set_initialized_var($1);
                     set_not_modifiable($1);
                     $<locs.primary_location>$ = data_array-1; //location of loop variable is last used memory cell
                     $<locs.var_id>$ = $1;
                 };

for_loop_value   : value {
                    if($<locs.need_init_check>1)
                        check_initialization($<locs.var_id>1);

                    if($<locs.array_location>$ != -5)
                        $$ = get_value_location($<locs.primary_location>1, $<locs.array_location>1);
                    else
                        $$ = copy_var($<locs.primary_location>1);

                };
Do               : DO {
                    $<for_locs.acc_loc>$ = init_for_loop();
                    $<for_locs.jump_loc>$ = external_code.size();
                };



expression      : value { $$ = $1; }
                 | value '+' value {
                     if($<locs.need_init_check>1)
                         check_initialization($<locs.var_id>1);
                     if($<locs.need_init_check>3)
                         check_initialization($<locs.var_id>3);

                    $<locs.primary_location>$ = add($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                    $<locs.array_location>$ = -5;
                    $<locs.external_var>$ = false;
                    $<locs.need_init_check>$ = false;
                 }
                 | value '-' value{
                     if($<locs.need_init_check>1)
                         check_initialization($<locs.var_id>1);
                     if($<locs.need_init_check>3)
                         check_initialization($<locs.var_id>3);

                    $<locs.primary_location>$ = sub($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                    $<locs.array_location>$ = -5;
                    $<locs.external_var>$ = false;
                    $<locs.need_init_check>$ = false;
                }
                 | value '*' value {
                     if($<locs.need_init_check>1)
                         check_initialization($<locs.var_id>1);
                     if($<locs.need_init_check>3)
                         check_initialization($<locs.var_id>3);

                     $<locs.primary_location>$ = mult($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                     $<locs.array_location>$= -5;
                     $<locs.external_var>$ = false;
                     $<locs.need_init_check>$ = false;
                 }
                 | value '/' value {
                     if($<locs.need_init_check>1)
                         check_initialization($<locs.var_id>1);
                     if($<locs.need_init_check>3)
                         check_initialization($<locs.var_id>3);

                     $<locs.primary_location>$ = div($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                     $<locs.array_location>$ = -5;
                     $<locs.external_var>$ = false;
                     $<locs.need_init_check>$ = false;
                 }
                 | value '%' value {
                     if($<locs.need_init_check>1)
                         check_initialization($<locs.var_id>1);
                     if($<locs.need_init_check>3)
                         check_initialization($<locs.var_id>3);

                         $<locs.primary_location>$ = mod($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                         $<locs.array_location>$= -5;
                         $<locs.external_var>$ = false;
                         $<locs.need_init_check>$ = false;
                 }
                 ;

condition   : value EQ value {
                    $$ = eq($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                    place_jzero($$);
                }
             | value NEQ value {
                    $$ = neq($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                    place_jzero($$);
             }
             | value LESS value {
                    $$ = less_($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                    place_jzero($$);
             }
             | value GREATER value {
                    $$ = greater_($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                    place_jzero($$);
             }
             | value LESSEQ value {
                    $$ = lesseq($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                    place_jzero($$);
             }
             | value GREATEREQ value {
                    $$ = greatereq($<locs.primary_location>1,$<locs.array_location>1, $<locs.primary_location>3, $<locs.array_location>3);
                    place_jzero($$);
            }
             ;

value       : NUMBER    {
                    $<locs.primary_location>$ = build_number($1);
                    $<locs.array_location>$ = -5;
                    $<locs.external_var>$ = false;
                    $<locs.need_init_check>$ = false;}
             | identifier { $$ = $1; }
             ;

identifier  : PIDIDENTIFIER {
                    $<locs.primary_location>$= check_variable($1,-1);
                    if($<locs.primary_location>$ == -1){
                        $<locs.need_init_check>$ = false;
                    } else {
                        $<locs.need_init_check>$ = true;
                    }
                    $<locs.array_location>$ = -5;
                    $<locs.var_id>$ = $1;
                    $<locs.external_var>$ = false;}
             | PIDIDENTIFIER'['PIDIDENTIFIER']' {
                    $<locs.primary_location>$ = check_variable($3,-1);
                    if($<locs.primary_location>$ == -1){
                        $<locs.need_init_check>$ = false;
                    } else {
                        $<locs.need_init_check>$ = true;
                    }
                    $<locs.array_location>$ = check_variable($1, 0) ;
                    $<locs.var_id>$ = $3;
                    $<locs.external_var>$ = true;
                }
             | PIDIDENTIFIER'['NUMBER']' {
                    $<locs.primary_location>$ = check_variable($1, $3);
                    $<locs.array_location>$ = -5;
                    $<locs.need_init_check>$ = false;
                    $<locs.external_var>$ = false;
                    $<locs.var_id>$ = $1;
                }
             ;

%%


void yyerror(char const *s){
    cerr<<lineno<<": "<<linebuf<<"\n";
    cerr<<"*********************************************\n"<<endl;
    //printf("%*s\n", 1+tokenpos, "^");
}

int main(void) {
    cerr<<termcolor::red<<"\n";
    yyparse();
    external_code.push_back(op_name[HALT]);
    if(errors == 0) print_code();
    cerr<<termcolor::reset<<"";
}

void install_variable(string name, int size) {
    if(get_variable(name, 0) == -1) {
        add_variable(name, size);
    } else {
        errors++;
        cerr<<"Error: Variable "<<name<<" already defined."<<endl;
        yyerror(" ");
    }
}


int check_variable(string id, int index) {
    int location = get_variable(id, index);
    if(location >= 0) {
        return location;
    } else if (location == -1) {
        errors++;
        cerr<<"Error: Variable "<<id<<" not declared."<<endl;
        yyerror(" ");
        return -1;
    } else if(location == -2) {
        errors++;
        cerr<<"Error: Variable "<<id<<" exceeds the range."<<endl;
        yyerror(" ");
        return -2;
    } else if(location == -3) {
        errors++;
        cerr<<"Error: Variable "<<id<<" used inproperly"<<endl;
        yyerror(" ");
        return -3;
    }
}


int check_initialization(string id) {
    if(!is_initialized_var(id)) {
        errors++;
        cerr<<"Error: Variable "<<id<<" wasn't initialized before."<<endl;
        yyerror(" ");
        return -1;
    }
    return 1;
}

int check_modification_possibility(string id) {
    if(!is_modifiable(id)) {
        errors++;
        cerr<<"Error: For loop variable "<<id<<" cannot be modified."<<endl;
        yyerror(" ");
        return -1;
    }
    return 1;
}
