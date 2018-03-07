using namespace std;
/* OPERATIONS: Internal Representation */
enum code_ops { GET=0, PUT, LOAD, LOADI, STORE, STOREI, ADD, ADDI, SUB,
    SUBI, SHR, SHL, INC, DEC, ZERO, JUMP, JZERO, JODD, HALT };

/* OPERATIONS: External Representation */
string op_name[] = { "GET", "PUT", "LOAD", "LOADI", "STORE", "STOREI", "ADD", "ADDI", "SUB",
    "SUBI", "SHR", "SHL", "INC", "DEC", "ZERO", "JUMP", "JZERO", "JODD", "HALT" };


/* Code generation functions */
int greater_internal(int x, int y);


/* External code */
vector<string> external_code;


/* Initial data location = 0 */
int data_array = 0;

/* Reserves a data location */
int new_data_location() {
    return data_array++;
}

 int build_number( long long number) {
    if(number < 0) return 0;
    external_code.push_back(op_name[ZERO]);
    int location = new_data_location();
    if(number == 0) {
        external_code.push_back(op_name[STORE]+" "+to_string(location));
    }
    vector <string> x;
    while(number != 0) {
        if(number % 2 == 0){
            x.push_back(op_name[SHL]);
            number = number /2;
        } else {
            x.push_back(op_name[INC]);
            number--;
        }
    }
    for(int i=x.size()-1;i>=0;i--) {
        external_code.push_back(x[i]);
    }
    external_code.push_back(op_name[STORE]+" "+to_string(location));
    return location;
}

int copy_var(int location) {
    int copy = new_data_location();
    external_code.push_back(op_name[LOAD]+ " " + to_string(location));
    external_code.push_back(op_name[STORE]+ " " + to_string(copy));
    return copy;
}

int get_value_location(int primary_location, int array_location) {
    build_number(array_location);
    external_code.push_back(op_name[ADD]+ " " + to_string(primary_location));
    int result_location = new_data_location();
    external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
    external_code.push_back(op_name[LOADI]+ " " + to_string(result_location));
    external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
    return result_location;
}

void read(int primary_location, int array_location) {
    external_code.push_back(op_name[GET]);
    if(array_location == -5){
        external_code.push_back(op_name[STORE]+ " " + to_string(primary_location));
    } else if(array_location >=0) {
        int get_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(get_location));
        build_number(array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        external_code.push_back(op_name[LOAD]+ " " + to_string(get_location));
        external_code.push_back(op_name[STOREI]+ " " + to_string(result_location));
    }
}

void write(int primary_location, int array_location) {
    if(array_location == -5){
        external_code.push_back(op_name[LOAD]+ " " + to_string(primary_location));
    } else if(array_location >=0) {
        build_number(array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        external_code.push_back(op_name[LOADI]+ " " + to_string(result_location));
    }
    external_code.push_back(op_name[PUT]);
}

int init_for_loop() {
    external_code.push_back(op_name[LOAD]); //load first value
    external_code.push_back(op_name[STORE]); // store in for_var
    int acc = new_data_location();
    external_code.push_back(op_name[LOAD]); //load seceond vaue
    external_code.push_back(op_name[INC]); // a=value2 + 1
    external_code.push_back(op_name[SUB]); // a=y+1-x
    external_code.push_back(op_name[STORE]+ " " + to_string(acc)); // save y+1-x to acc
    //acc value is in a now
    external_code.push_back(op_name[JZERO]); // a=0 so leave the loop
    return acc;
}

//do it before decreasing accumulator
int dec_for_loop_iterator(int it) {
    external_code.push_back(op_name[LOAD]+ " " + to_string(it));
    external_code.push_back(op_name[DEC]);
    external_code.push_back(op_name[STORE]+ " " + to_string(it));
}

//do it before decreasing accumulator
int inc_for_loop_iterator(int it) {
    external_code.push_back(op_name[LOAD]+ " " + to_string(it));
    external_code.push_back(op_name[INC]);
    external_code.push_back(op_name[STORE]+ " " + to_string(it));
}


int dec_for_loop_accumulator(int acc) {
    //decrease accumlator
    external_code.push_back(op_name[LOAD]+ " " + to_string(acc));
    external_code.push_back(op_name[DEC]);
    external_code.push_back(op_name[STORE]+ " " + to_string(acc));

    external_code.push_back(op_name[JUMP]); //jump to first line of commands again

    return 1;
}



int place_for_loop_arguments(int first_inst, int val1, int val2, int for_var) {
    int k = external_code.size() - 1;

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[JUMP]) {
            external_code[i] +=  " " +to_string(first_inst - 1); // -1, before first instruction there is jzero
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[JZERO]) {
            external_code[i] +=  " " +to_string(external_code.size());
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[SUB]) {
            external_code[i] +=  " " +to_string(val1);
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[LOAD]) {
            external_code[i] +=  " " +to_string(val2);
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[STORE]) {
            external_code[i] +=  " " +to_string(for_var);
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[LOAD]) {
            external_code[i] +=  " " +to_string(val1);
            k=i;
            break;
        }
    }


}

int place_for_loop_arguments_downto_version(int first_inst, int val1, int val2, int for_var) {
    int k = external_code.size() - 1;

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[JUMP]) {
            external_code[i] +=  " " +to_string(first_inst - 1); // -1, before first instruction there is jzero
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[JZERO]) {
            external_code[i] +=  " " +to_string(external_code.size());
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[SUB]) {
            external_code[i] +=  " " +to_string(val2);
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[LOAD]) {
            external_code[i] +=  " " +to_string(val1);
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[STORE]) {
            external_code[i] +=  " " +to_string(for_var);
            k=i;
            break;
        }
    }

    for(int i=k; i>=0; i--){
        if(external_code[i] == op_name[LOAD]) {
            external_code[i] +=  " " +to_string(val1);
            k=i;
            break;
        }
    }


}

void assign(int source_primary_location, int source_array_location, int target_primary_location, int target_array_location) {

    if(source_array_location == -5 && target_array_location == -5) {
        external_code.push_back(op_name[LOAD]+ " " + to_string(source_primary_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(target_primary_location));
    } else if(source_array_location == -5){
        build_number(target_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(target_primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location)); //store final target location
        external_code.push_back(op_name[LOAD]+ " " + to_string(source_primary_location));
        external_code.push_back(op_name[STOREI]+ " " + to_string(result_location));
    } else if(target_array_location == -5){
        build_number(source_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(source_primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        external_code.push_back(op_name[LOADI]+ " " + to_string(result_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(target_primary_location));
    } else {
        build_number(source_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(source_primary_location));
        int source_result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(source_result_location));

        build_number(target_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(target_primary_location));
        int target_result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(target_result_location));

        external_code.push_back(op_name[LOADI]+ " " + to_string(source_result_location));
        external_code.push_back(op_name[STOREI]+ " " + to_string(target_result_location));
    }
}

int place_jzero(int a){
    external_code.push_back(op_name[LOAD]+ " " + to_string(a));
    external_code.push_back(op_name[JZERO]);
    return 1;
}

int place_jzero_argument(int a){
    for(int i=external_code.size()-1; i>=0; i--){
        if(external_code[i] == op_name[JZERO]) {
            external_code[i] +=  " " +to_string(a);
            return 1;
        }
    }
    return 0;
}

int place_jump(){
    external_code.push_back(op_name[JUMP]);
    return 1;
}

int place_jump_argument(int a) {
    for(int i=external_code.size()-1; i>=0; i--){
        if(external_code[i] == op_name[JUMP]) {
            external_code[i] += " " +to_string(a);
            return 1;
        }
    }
    return 0;
}

int add(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        external_code.push_back(op_name[LOAD]+" "+to_string(value1_primary_location));
        external_code.push_back(op_name[ADD]+" "+to_string(value2_primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+" "+to_string(result_location));
        return result_location;
    } else if (value1_array_location == -5) {
        build_number(value2_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(value2_primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        external_code.push_back(op_name[LOAD]+ " " + to_string(value1_primary_location));
        external_code.push_back(op_name[ADDI]+ " " + to_string(result_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        return result_location;
    } else if (value2_array_location == -5) {
        build_number(value1_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(value1_primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        external_code.push_back(op_name[LOAD]+ " " + to_string(value2_primary_location));
        external_code.push_back(op_name[ADDI]+ " " + to_string(result_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        return result_location;
    } else {
        int result1_location = new_data_location();
        int result2_location = new_data_location();
        build_number(value1_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(value1_primary_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result1_location));

        build_number(value2_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(value2_primary_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result2_location));

        external_code.push_back(op_name[LOADI]+ " " + to_string(result1_location));
        external_code.push_back(op_name[ADDI]+ " " + to_string(result2_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result2_location));
        return result2_location;
    }
    return -1;
}

int sub(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        external_code.push_back(op_name[LOAD]+" "+to_string(value1_primary_location));
        external_code.push_back(op_name[SUB]+" "+to_string(value2_primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+" "+to_string(result_location));
        return result_location;
    } else if (value1_array_location == -5) {
        build_number(value2_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(value2_primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        external_code.push_back(op_name[LOAD]+ " " + to_string(value1_primary_location));
        external_code.push_back(op_name[SUBI]+ " " + to_string(result_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        return result_location;
    } else if (value2_array_location == -5) {
        build_number(value1_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(value1_primary_location));
        int result_location = new_data_location();
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        external_code.push_back(op_name[LOADI]+ " " + to_string(result_location));

        external_code.push_back(op_name[SUB]+ " " + to_string(value2_primary_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result_location));
        return result_location;
    } else {
        int result1_location = new_data_location();
        int result2_location = new_data_location();
        build_number(value1_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(value1_primary_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result1_location));

        build_number(value2_array_location);
        external_code.push_back(op_name[ADD]+ " " + to_string(value2_primary_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result2_location));

        external_code.push_back(op_name[LOADI]+ " " + to_string(result1_location));
        external_code.push_back(op_name[SUBI]+ " " + to_string(result2_location));
        external_code.push_back(op_name[STORE]+ " " + to_string(result2_location));
        return result2_location;
    }
    return -1;
}


int mult_internal(int x, int y) {
    int acc = new_data_location();
    int temp_x = new_data_location();
    int temp_y = new_data_location();
    external_code.push_back(op_name[ZERO]);
    external_code.push_back(op_name[STORE]+ " " + to_string(acc));
    int s = external_code.size();


    external_code.push_back(op_name[LOAD]+ " " + to_string(y)); //s
    //if y == 0 return acc
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+20)); //s+1
    //create temp y
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_y)); //s+2


    external_code.push_back(op_name[LOAD]+ " " + to_string(x)); //s + 3
    //create temp x
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x)); //s+4

    //if x == 0 return acc
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_x)); //s + 5
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+20));// s+6


    // if x % 2 != 0 jump
    external_code.push_back(op_name[JODD]+ " " + to_string(s+14)); //s+7

    // x%2 = 0
    external_code.push_back(op_name[SHR]); //s+8
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x));
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_y));
    external_code.push_back(op_name[SHL]);
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_y));
    //jump back to checking x=0
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+5)); //s+13

    // x%2 != 0
    external_code.push_back(op_name[DEC]); //s+14
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x));
    external_code.push_back(op_name[LOAD]+ " " + to_string(acc));
    external_code.push_back(op_name[ADD]+ " " + to_string(temp_y));
    external_code.push_back(op_name[STORE]+ " " + to_string(acc));
    //jump back to s
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+5));
    //s+20 ->

    return acc;
}


int mult(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        return mult_internal(value1_primary_location,value2_primary_location);
    } else if (value1_array_location == -5) {
        int result_location = get_value_location(value2_primary_location, value2_array_location);
        return mult_internal(value1_primary_location, result_location);
    } else if (value2_array_location == -5) {
        int result_location = get_value_location(value1_primary_location, value1_array_location);
        return mult_internal(result_location, value2_primary_location);
    } else {
        int result1_location = get_value_location(value1_primary_location, value1_array_location);
        int result2_location = get_value_location(value2_primary_location, value2_array_location);

        return mult_internal(result1_location, result2_location);
    }
    return -1;
}

int div_internal(int x, int y) {

    int acc = new_data_location();
    int temp_x = new_data_location();
    int temp_y = new_data_location();
    external_code.push_back(op_name[LOAD]+ " " + to_string(x));
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x));
    external_code.push_back(op_name[LOAD]+ " " + to_string(y));
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_y));
    external_code.push_back(op_name[ZERO]);
    external_code.push_back(op_name[STORE]+ " " + to_string(acc));

    //s:
    int s = external_code.size();

    //if y == 0 return 0
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_y)); //s
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+40)); //s+1


    //check y is 1, then return x
    external_code.push_back(op_name[DEC]); // s+2
    //if 0 means x/1, return x
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+5)); //s+3

    //y>1 so jump to checking y is 2
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+8)); //s+4

    //return x
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_x)); //s+5
    external_code.push_back(op_name[STORE]+ " " + to_string(acc)); //s+6
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+40)); //s+7



    //check y is 2, then return shr x
    external_code.push_back(op_name[DEC]); // s+8
    //if y is 2 go shr
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+11)); //s+9
    //jump to checking x%2 and y%2
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+15)); //s+10

    //load x, shr and return x
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_x)); //s+11
    external_code.push_back(op_name[SHR]); //s+12
    external_code.push_back(op_name[STORE]+ " " + to_string(acc)); //s+13
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+40)); //s+14


    //if x%2 == 0 and y%2 == 0 repeat shr
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_y)); //s+15
    external_code.push_back(op_name[JODD]+ " " + to_string(s+25));//s+16

    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_x)); //s+17
    external_code.push_back(op_name[JODD]+ " " + to_string(s+25)); //s+18
    external_code.push_back(op_name[SHR]);//s+19
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x)); //s+20
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_y));
    external_code.push_back(op_name[SHR]);
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_y));
    //go back to checking y = 1
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+2)); //s+24



    // do a = x - y
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_x)); //s+25
    external_code.push_back(op_name[SUB]+ " " + to_string(temp_y));
    //if a==0 there is chance that x<y (if x<y break)
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+29)); //s+27
    // x>y ok, jump to next part
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+33)); //s+28

    //do a = y - x, if 0 then x=y, can do last iteration and acc++
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_y)); //s+ 29
    external_code.push_back(op_name[SUB]+ " " + to_string(temp_x)); //s+30
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+33));  //s+31
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+40));   //s+32


    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_x)); //s+33
    external_code.push_back(op_name[SUB]+ " " + to_string(temp_y));
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x)); //s+35
    external_code.push_back(op_name[LOAD]+ " " + to_string(acc));
    external_code.push_back(op_name[INC]);
    external_code.push_back(op_name[STORE]+ " " + to_string(acc));
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+25));
    //s+40 ->

    return acc;
}

// n/m
int div_internal_logaritmic(int n, int m) {
    int a = new_data_location();
    int c = new_data_location();
    int temp_m = new_data_location();
    int temp_a_plus_c = new_data_location();
    int mult_result = new_data_location();
    int comparison_result = new_data_location();

    int s = external_code.size();

    //init a=0,c=1 and temp_m = m
    external_code.push_back(op_name[ZERO]); //s
    external_code.push_back(op_name[STORE]+ " " + to_string(a));
    external_code.push_back(op_name[INC]);
    external_code.push_back(op_name[STORE]+ " " + to_string(c));
    external_code.push_back(op_name[LOAD]+ " " + to_string(m));
    //m = 0 ? return 0
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+64));
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_m));
    // n = 0? return 0
    external_code.push_back(op_name[LOAD]+ " " + to_string(n));
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+64));



    /*
    while c∗m <= n do
        c := 2∗c
    endwhile
    */
    external_code.push_back(op_name[LOAD]+ " " + to_string(c)); //s +9
    external_code.push_back(op_name[SHL]);
    external_code.push_back(op_name[STORE]+ " " + to_string(c));
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_m));
    external_code.push_back(op_name[SHL]);
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_m));
    external_code.push_back(op_name[SUB]+ " " + to_string(n));
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+9));



    external_code.push_back(op_name[LOAD]+ " " + to_string(c)); //s+17
    external_code.push_back(op_name[DEC]);
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+64));
    external_code.push_back(op_name[INC]);
    external_code.push_back(op_name[SHR]);
    external_code.push_back(op_name[STORE]+ " " + to_string(c));
    external_code.push_back(op_name[ADD]+ " " + to_string(a));
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_a_plus_c)); //s+24
    mult_result = mult_internal(temp_a_plus_c, m);
    comparison_result = greater_internal(mult_result, n);
    external_code.push_back(op_name[LOAD]+ " " + to_string(comparison_result));// s+25+ 22 + 10 = s+57
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+60));
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+17));



    //a = a+c
    external_code.push_back(op_name[LOAD]+ " " + to_string(a)); //s+60
    external_code.push_back(op_name[ADD]+ " " + to_string(c));
    external_code.push_back(op_name[STORE]+ " " + to_string(a));
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+17)); //s+63
    //s+64 ->

    return a;
}




int div(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        return div_internal_logaritmic(value1_primary_location,value2_primary_location);
    } else if (value1_array_location == -5) {
        int result_location = get_value_location(value2_primary_location, value2_array_location);
        return div_internal_logaritmic(value1_primary_location, result_location);
    } else if (value2_array_location == -5) {
        int result_location = get_value_location(value1_primary_location, value1_array_location);
        return div_internal_logaritmic(result_location, value2_primary_location);
    } else {
        int result1_location = get_value_location(value1_primary_location, value1_array_location);
        int result2_location = get_value_location(value2_primary_location, value2_array_location);
        return div_internal_logaritmic(result1_location, result2_location);
    }
    return -1;
}

//n%m
int mod_internal_logaritmic(int a, int b) {

    int i = new_data_location();
    int temp_a = new_data_location();
    int temp_b = new_data_location();


    int s = external_code.size();

    //init i=0, temp_a, temp_b
    external_code.push_back(op_name[ZERO]); //s
    external_code.push_back(op_name[STORE]+ " " + to_string(i)); //s+1
    external_code.push_back(op_name[LOAD]+ " " + to_string(a));
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+41));
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_a));
    external_code.push_back(op_name[LOAD]+ " " + to_string(b));
    //if b=0 return 0
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+8)); //s+6
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+10)); //s+7
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_a)); //s+8
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+41)); //s+9

    external_code.push_back(op_name[STORE]+ " " + to_string(temp_b)); //s+10


    /*
        while b<=a{
        i++;
        shl b
    }*/
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_b)); //s+11
    external_code.push_back(op_name[SUB]+ " " + to_string(temp_a));
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+15));
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+22));


    // b<=a
    external_code.push_back(op_name[LOAD]+ " " + to_string(i)); //s+15
    external_code.push_back(op_name[INC]);
    external_code.push_back(op_name[STORE]+ " " + to_string(i));
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_b));
    external_code.push_back(op_name[SHL]);
external_code.push_back(op_name[STORE]+ " " + to_string(temp_b));
external_code.push_back(op_name[JUMP]+ " " + to_string(s+11));


    //shr b, check b<=a then a=a-b
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_b)); //s+22
    external_code.push_back(op_name[SHR]);
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_b));
    external_code.push_back(op_name[LOAD]+ " " + to_string(i));
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+41));
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_b));
    external_code.push_back(op_name[SUB]+ " " + to_string(temp_a));
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+34));
    external_code.push_back(op_name[LOAD]+ " " + to_string(i));
    external_code.push_back(op_name[DEC]);
    external_code.push_back(op_name[STORE]+ " " + to_string(i));
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+22));


    //a=a-b
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_a)); //s+34
    external_code.push_back(op_name[SUB]+ " " + to_string(temp_b));
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_a));
    external_code.push_back(op_name[LOAD]+ " " + to_string(i));
    external_code.push_back(op_name[DEC]);
    external_code.push_back(op_name[STORE]+ " " + to_string(i));
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+22));
    //s+41


    return temp_a;
}

int mod_internal(int x, int y) {

    int temp_x = new_data_location();
    external_code.push_back(op_name[ZERO]);
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x));


    int s = external_code.size();

    external_code.push_back(op_name[LOAD]+ " " + to_string(y)); //s
    // a-- if = 0 means x%0 or x%1 then return 0
    external_code.push_back(op_name[DEC]); //s+1
    //go to return 0
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+23)); //s+2


    //check y == 2
    external_code.push_back(op_name[DEC]);//s+3
    // if y=2 go check x%2 = 0
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+6));//s+4

    // y > 2, go to next part
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+9)); //s+5



    // y=2, check x is odd, then return 1
    external_code.push_back(op_name[LOAD]+ " " + to_string(x));//s+6
    external_code.push_back(op_name[JODD]+ " " + to_string(s+26));
    // else x is even so return 0
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+23));//s+8



    //crete temp x
    external_code.push_back(op_name[LOAD]+ " " + to_string(x));//s+9
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x));



    // do a = x - y
    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_x)); //s+11
    external_code.push_back(op_name[SUB]+ " " + to_string(y)); //s+12
    //if a==0 there is chance that x<y (if x<y result is x)
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+15)); //s+13
    // x>y ok, jump to next part
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+19));



    //do a = y - x, if 0 then x=y, it means x%y = 0
    external_code.push_back(op_name[LOAD]+ " " + to_string(y)); //s+15
    external_code.push_back(op_name[SUB]+ " " + to_string(temp_x));
    // if = 0 means temp x = y so return 0
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+23));

    // return x
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+29));//s+18


    external_code.push_back(op_name[LOAD]+ " " + to_string(temp_x)); //s+19
    external_code.push_back(op_name[SUB]+ " " + to_string(y));
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x));

    external_code.push_back(op_name[JUMP]+ " " + to_string(s+11)); //s+22




    //return 0
    external_code.push_back(op_name[ZERO]); //s+23
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x));
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+29));

    //return 1
    external_code.push_back(op_name[ZERO]);//s+26
    external_code.push_back(op_name[INC]);
    external_code.push_back(op_name[STORE]+ " " + to_string(temp_x));
    //s+29 ->



    return temp_x;

}

int mod(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        return mod_internal_logaritmic(value1_primary_location, value2_primary_location);
    } else if (value1_array_location == -5) {
        int result_location = get_value_location(value2_primary_location, value2_array_location);
        return mod_internal_logaritmic(value1_primary_location, result_location);
    } else if (value2_array_location == -5) {
        int result_location = get_value_location(value1_primary_location, value1_array_location);
        return mod_internal_logaritmic(result_location, value2_primary_location);
    } else {
        int result1_location = get_value_location(value1_primary_location, value1_array_location);
        int result2_location = get_value_location(value2_primary_location, value2_array_location);
        return mod_internal_logaritmic(result1_location, result2_location);
    }
    return -1;
}


int eq_internal(int x, int y) {
    //result 1 = x-y
    external_code.push_back(op_name[LOAD]+ " " + to_string(x));
    external_code.push_back(op_name[SUB]+ " " + to_string(y));
    int result1_location = new_data_location();
    external_code.push_back(op_name[STORE]+ " " + to_string(result1_location));

    // y-x
    external_code.push_back(op_name[LOAD]+ " " + to_string(y));
    external_code.push_back(op_name[SUB]+ " " + to_string(x));
    // if  = 0 then x=y
    external_code.push_back(op_name[ADD]+ " " + to_string(result1_location));


    int s = external_code.size();

    //if a = 0 then x = y, need to return true (more than 0), jump to m
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+3));

    // x != y -> a = 0
    external_code.push_back(op_name[ZERO]);
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+4));

    // m: a++ means true
    external_code.push_back(op_name[INC]);

    //store result
    external_code.push_back(op_name[STORE]+ " " + to_string(result1_location));

    return result1_location;
}

int eq(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        return eq_internal(value1_primary_location,value2_primary_location);
    } else if (value1_array_location == -5) {
        int result_location = get_value_location(value2_primary_location, value2_array_location);
        return eq_internal(value1_primary_location, result_location);
    } else if (value2_array_location == -5) {
        int result_location = get_value_location(value1_primary_location, value1_array_location);
        return eq_internal(result_location, value2_primary_location);
    } else {
        int result1_location = get_value_location(value1_primary_location, value1_array_location);
        int result2_location = get_value_location(value2_primary_location, value2_array_location);
        return eq_internal(result1_location, result2_location);
    }
    return -1;
}




int neq_internal(int x, int y) {
    //x - y
    external_code.push_back(op_name[LOAD]+ " " + to_string(x));
    external_code.push_back(op_name[SUB]+ " " + to_string(y));
    int result1_location = new_data_location();
    external_code.push_back(op_name[STORE]+ " " + to_string(result1_location));
    //y-x
    external_code.push_back(op_name[LOAD]+ " " + to_string(y));
    external_code.push_back(op_name[SUB]+ " " + to_string(x));

    //(x-y) + (y-x), if >0 x != y
    external_code.push_back(op_name[ADD]+ " " + to_string(result1_location));

    external_code.push_back(op_name[STORE]+ " " + to_string(result1_location));
    return result1_location;
}

int neq(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        return neq_internal(value1_primary_location,value2_primary_location);
    } else if (value1_array_location == -5) {
        int result_location = get_value_location(value2_primary_location, value2_array_location);
        return neq_internal(value1_primary_location, result_location);
    } else if (value2_array_location == -5) {
        int result_location = get_value_location(value1_primary_location, value1_array_location);
        return neq_internal(result_location, value2_primary_location);
    } else {
        int result1_location = get_value_location(value1_primary_location, value1_array_location);
        int result2_location = get_value_location(value2_primary_location, value2_array_location);
        return neq_internal(result1_location, result2_location);
    }
    return -1;
}


int less_internal(int x, int y) {

    int result = new_data_location();

    // x-y
    external_code.push_back(op_name[LOAD]+ " " + to_string(x));
    external_code.push_back(op_name[SUB]+ " " + to_string(y));

    int s = external_code.size();

    //if x-y = 0
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+2)); //s
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+6));

    //y-x
    external_code.push_back(op_name[LOAD]+ " " + to_string(y)); //s+2
    external_code.push_back(op_name[SUB]+ " " + to_string(x));

    external_code.push_back(op_name[JZERO]+ " " + to_string(s+6));
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+7));

    external_code.push_back(op_name[ZERO]); //s + 6
    external_code.push_back(op_name[STORE]+ " " + to_string(result)); //s+7

    return result;
}

int less_(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        return less_internal(value1_primary_location,value2_primary_location);
    } else if (value1_array_location == -5) {
        int result_location = get_value_location(value2_primary_location, value2_array_location);
        return less_internal(value1_primary_location, result_location);
    } else if (value2_array_location == -5) {
        int result_location = get_value_location(value1_primary_location, value1_array_location);
        return less_internal(result_location, value2_primary_location);
    } else {
        int result1_location = get_value_location(value1_primary_location, value1_array_location);
        int result2_location = get_value_location(value2_primary_location, value2_array_location);
        return less_internal(result1_location, result2_location);
    }
    return -1;
}


int greater_internal(int x, int y) {
    return less_internal(y,x);
}

int greater_(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        return greater_internal(value1_primary_location,value2_primary_location);
    } else if (value1_array_location == -5) {
        int result_location = get_value_location(value2_primary_location, value2_array_location);
        return greater_internal(value1_primary_location, result_location);
    } else if (value2_array_location == -5) {
        int result_location = get_value_location(value1_primary_location, value1_array_location);
        return greater_internal(result_location, value2_primary_location);
    } else {
        int result1_location = get_value_location(value1_primary_location, value1_array_location);
        int result2_location = get_value_location(value2_primary_location, value2_array_location);
        return greater_internal(result1_location, result2_location);
    }
    return -1;
}


int lesseq_internal(int x, int y) {
    int result = new_data_location();

    // x-y
    external_code.push_back(op_name[LOAD]+ " " + to_string(x));
    external_code.push_back(op_name[SUB]+ " " + to_string(y));

    int s = external_code.size();

    //if a = 0 then x <= y, need to return true (more than 0), jump to m
    external_code.push_back(op_name[JZERO]+ " " + to_string(s+3));

    // x != y -> a = 0
    external_code.push_back(op_name[ZERO]);
    external_code.push_back(op_name[JUMP]+ " " + to_string(s+4));

    // m: a++ means true
    external_code.push_back(op_name[INC]);

    //store result
    external_code.push_back(op_name[STORE]+ " " + to_string(result));

    return result;

}


int lesseq(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        return lesseq_internal(value1_primary_location,value2_primary_location);
    } else if (value1_array_location == -5) {
        int result_location = get_value_location(value2_primary_location, value2_array_location);
        return lesseq_internal(value1_primary_location, result_location);
    } else if (value2_array_location == -5) {
        int result_location = get_value_location(value1_primary_location, value1_array_location);
        return lesseq_internal(result_location, value2_primary_location);
    } else {
        int result1_location = get_value_location(value1_primary_location, value1_array_location);
        int result2_location = get_value_location(value2_primary_location, value2_array_location);
        return lesseq_internal(result1_location, result2_location);
    }
    return -1;
}

int greatereq_internal(int x, int y) {
    return lesseq_internal(y,x);
}

int greatereq(int value1_primary_location, int value1_array_location, int value2_primary_location, int value2_array_location) {
    if(value1_array_location == -5 && value2_array_location == -5) {
        return greatereq_internal(value1_primary_location,value2_primary_location);
    } else if (value1_array_location == -5) {
        int result_location = get_value_location(value2_primary_location, value2_array_location);
        return greatereq_internal(value1_primary_location, result_location);
    } else if (value2_array_location == -5) {
        int result_location = get_value_location(value1_primary_location, value1_array_location);
        return greatereq_internal(result_location, value2_primary_location);
    } else {
        int result1_location = get_value_location(value1_primary_location, value1_array_location);
        int result2_location = get_value_location(value2_primary_location, value2_array_location);
        return greatereq_internal(result1_location, result2_location);
    }
    return -1;
}



void print_code() {
    int i = 0;
    while (i < external_code.size()) {
        cout<<external_code[i]<<endl;
        i++;
    }
}
