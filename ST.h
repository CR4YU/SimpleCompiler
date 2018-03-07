using namespace std;

extern int new_data_location();
extern int data_array;


struct variable {
    string name;
    int location;
    int size;
    bool initialized;
    bool modifiable;
};
typedef struct variable variable;


vector<variable> variables;


void add_variable(string name, int size) {
    variable v;
    v.name = name;
    v.location = data_array;
    v.modifiable = true;
    v.size = size;
    // size = 0 -> var
    // size > 0 -> table
    if(size > 0) {
        v.initialized = true; //treat tables as initialized on init
    } else {
        v.initialized = false; // variables are not initalized
    }

    if(size == 0){
        new_data_location();
    } else {
        data_array += size;
    }
    variables.push_back(v);
}

void set_not_modifiable(string name) {
    for(int i=0; i<variables.size(); i++) {
        if(variables[i].name == name) {
            variables[i].modifiable = false;
        }
    }
}

bool is_modifiable(string name) {
    for(int i=0; i<variables.size(); i++) {
        if(variables[i].name == name) {
            return variables[i].modifiable;
        }
    }
    return true;
}

void delete_var(string name) {
    for(int i=0; i<variables.size(); i++) {
        if(variables[i].name == name) {
            variables.erase(variables.begin()+i);
            break;
        }
    }
}


int get_variable(string name, int index) {
    for(int i=0; i<variables.size(); i++) {
        if(variables[i].name == name) {
            if(index == -1 && variables[i].size == 0){
                return variables[i].location; //return var location
            } else if(index == -1){
                return -3; // tried to use as var but it is table
            } else if(index >=0 && variables[i].size == 0) {
                return -3; //tried to use as table but it is var
            } else if(index >= variables[i].size) {
                return -2; // range error
            }
            return variables[i].location + index; // return location of tab[index]
        }
    }
    //var not found
    return -1;
}


bool is_initialized_var(string name) {
    for(int i=0; i<variables.size(); i++) {
        if(variables[i].name == name) {
            return variables[i].initialized;
        }
    }
    return false;
}

int set_initialized_var(string name) {
    for(int i=0; i<variables.size(); i++) {
        if(variables[i].name == name) {
            variables[i].initialized = true;
        }
    }
    return 0;
}
