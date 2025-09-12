#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_CMD_LEN 100
#define VARS_MAX 100
int varCount = 0;

// Union because it should only hold one of the values at a time


typedef union{
    long intValue;
    double floatValue;
    char charValue[1 + 1];
    char stringValue[64 + 1];
} VarValue;

typedef enum{
	VARIABLE,
	INT,
	FLOAT,
	STRING,
	CHAR,
	LIST,
	UNDEFINED
} VarType;

typedef enum {
	ASSIGNMENT,
	PRINT,
	APPEND,
	UNKNOWN
} OperationType;

typedef struct {
	char lhs[64];
	char rhs[128];
	char arg1[64];
	char arg2[64];
	char op;      // operator like '+' if present, 0 otherwise
	int arg_count; // 1 or 2
	int valid;     // 1 if successfully parsed
} AssignmentInfo;

typedef struct {
	char var[64];
	int valid;
} PrintInfo;

typedef struct {
    char name[64 + 1];
    VarType type;
	VarValue value;
	struct listNode *next;

} Variable;

Variable *vars[100];


typedef struct{
	union{
		Variable *var;
		long intValue;
		double floatValue;
		char charValue[1 + 1];
		char stringValue[64 + 1];
	} value; // The list can hold variables of any type
	int index; // Index in the list, used for getting individual elements
	struct listNode *next;
} listNode;

typedef struct {
	char arg1[64]; // list name
	char arg2[64]; // item to append
	int valid;
} AppendInfo;

void print(const char *s) { // Takes the char as input and then parses it to see if its a string or variablable.
    if(s == NULL) {
        printf("(null)\n");
        return;
    }
    const char print;
    if(strncmp(s, "\"", 1) == 0) {
        for(int i = 0; i < strlen(s); i++) {
            if(s[i] == '\"') continue;
            printf("%c", s[i]);
        }
        printf("\n");
        return;
    } else {
		printf("Looking for variable: %s\n", s);
		for(int i = 0; i < VARS_MAX; i++){
			if(vars[i] == NULL) break;
			printf("Comparing with: %s\n", vars[i]->name);
			if (vars[i] && strcmp(vars[i]->name, s) == 0) {
				switch(vars[i]->type){
					case INT:
						printf("%d\n", vars[i]->value.intValue);
						return; // Added so the loop does not run after the variable and the value is found. 
						break;
					case FLOAT:
						printf("%lf\n", vars[i]->value.floatValue);
						return;
						break;
					case STRING:
						printf("%s\n", vars[i]->value.stringValue);
						return;
						break;
					case CHAR:
						printf("%c\n", vars[i]->value.charValue[0]);
						return;
						break;
					case LIST:
						while(vars[i]->next != NULL){
							vars[i] = vars[i]->next;
							switch(vars[i]->type){
								case INT:
									printf("%d\n", vars[i]->value.intValue);
									break;
								case FLOAT:
									printf("%lf\n", vars[i]->value.floatValue);
									break;
								case STRING:
									printf("%s\n", vars[i]->value.stringValue);
									break;
								case CHAR:
									printf("%c\n", vars[i]->value.charValue[0]);
									break;
								default:
									break;
							}
						}
					default:
						break;
				}
			}
		}
        return;
    }
}

// Remove all whitespace from a string
void remove_whitespace(const char *src, char *dest) {
	while (*src) {
		if (!isspace((unsigned char)*src)) {
			*dest++ = *src;
		}
		src++;
	}
	*dest = '\0';
}

OperationType parse_command(const char *cmd) {
	char cleaned[256];
	remove_whitespace(cmd, cleaned);
	if (strncmp(cleaned, "print(", 6) == 0) {
		return PRINT;
	} else if (strncmp(cleaned, "append(", 7) == 0) {
		return APPEND;
	} else {
		const char *eq = strchr(cleaned, '=');
		if (eq) {
			int pos = (int)(eq - cleaned);
			if (pos > 0 && isalpha((unsigned char)cleaned[0])) {
				return ASSIGNMENT;
			}
		}
	}
	return UNKNOWN;
}

// Parse an assignment (supports optional single '+')
void parse_assignment(const char *cmd, AssignmentInfo *info) {
    memset(info, 0, sizeof(*info));
    char cleaned[256];
    remove_whitespace(cmd, cleaned);
    const char *eq = strchr(cleaned, '=');
    if (!eq) return;
    size_t lhs_len = (size_t)(eq - cleaned);
    if (lhs_len == 0 || lhs_len >= sizeof(info->lhs)) return;
    strncpy(info->lhs, cleaned, lhs_len);
    info->lhs[lhs_len] = '\0';
    const char *rhs_start = eq + 1;
    if (*rhs_start == '\0') return;
    strncpy(info->rhs, rhs_start, sizeof(info->rhs) - 1);
    info->rhs[sizeof(info->rhs) - 1] = '\0';
    // Check for list assignment
    if (strchr(info->rhs, '[')) {
        info->op = '['; // Use op to indicate list assignment
        info->arg_count = 1;
        strncpy(info->arg1, info->rhs, sizeof(info->arg1) - 1);
        info->arg1[sizeof(info->arg1) - 1] = '\0';
        info->valid = 1;
        return;
    }
    // Look for operator: +, -, *, /
    const char *op_ptr = NULL;
    char ops[] = "+-*/";
    for (int i = 0; ops[i]; i++) {
        op_ptr = strchr(info->rhs, ops[i]);
        if (op_ptr) {
            info->op = ops[i];
            break;
        }
    }
    if (op_ptr) {
        size_t a_len = (size_t)(op_ptr - info->rhs);
        if (a_len == 0 || a_len >= sizeof(info->arg1)) return;
        strncpy(info->arg1, info->rhs, a_len);
        info->arg1[a_len] = '\0';
        const char *second = op_ptr + 1;
        if (*second == '\0') return;
        strncpy(info->arg2, second, sizeof(info->arg2) - 1);
        info->arg2[sizeof(info->arg2) - 1] = '\0';
        info->arg_count = 2;
    } else {
        strncpy(info->arg1, info->rhs, sizeof(info->arg1) - 1);
        info->arg1[sizeof(info->arg1) - 1] = '\0';
        info->arg_count = 1;
    }
    info->valid = 1;
}

void parse_print(const char *cmd, PrintInfo *pi) {
	memset(pi, 0, sizeof(*pi));
	char cleaned[256];
	remove_whitespace(cmd, cleaned);
	if (strncmp(cleaned, "print(", 6) != 0) return;
	const char *inside = cleaned + 6; // after 'print('
	const char *end = strchr(inside, ')');
	if (!end || end == inside) return;
	size_t len = (size_t)(end - inside);
	if (len >= sizeof(pi->var)) len = sizeof(pi->var) - 1;
	strncpy(pi->var, inside, len);
	pi->var[len] = '\0';
	pi->valid = 1;
}

void parse_append(const char *cmd, AppendInfo *ai) {
	memset(ai, 0, sizeof(*ai));
	char cleaned[256];
	remove_whitespace(cmd, cleaned);
	if (strncmp(cleaned, "append(", 7) != 0) return;
	const char *inside = cleaned + 7; // after 'append('
	const char *end = strchr(inside, ')');
	if (!end || end == inside) return;
	const char *comma = strchr(inside, ',');
	if (!comma || comma > end) return;
	size_t len1 = (size_t)(comma - inside);
	size_t len2 = (size_t)(end - (comma + 1));
	if (len1 == 0 || len2 == 0) return;
	if (len1 >= sizeof(ai->arg1)) len1 = sizeof(ai->arg1) - 1;
	if (len2 >= sizeof(ai->arg2)) len2 = sizeof(ai->arg2) - 1;
	strncpy(ai->arg1, inside, len1);
	ai->arg1[len1] = '\0';
	strncpy(ai->arg2, comma + 1, len2);
	ai->arg2[len2] = '\0';
	ai->valid = 1;
}

// Reads up to MAX_CMD_LEN characters (excluding null terminator) until newline or EOF.
// Returns length read, or -1 on EOF with no characters read.
int read_command(char *buffer) {
	int idx = 0;
	int c;
	while (idx < MAX_CMD_LEN && (c = getchar()) != EOF) {
		if (c == '\n') break;
		buffer[idx++] = (char)c;
	}
	if (c == EOF && idx == 0) return -1; // EOF before any data
	buffer[idx] = '\0';
	// If line longer than MAX_CMD_LEN, discard remainder until newline or EOF
	if (c != '\n') {
		while (c != '\n' && c != EOF) {
			c = getchar();
		}
	}
	return idx;
}
// Checks the type of var if it exists. Used for arguments inputed
int findVarType(const char *rhs){
	printf("Finding type of: %s\n", rhs);	
    if(rhs == NULL || *rhs == '\0') return -1;
    if(rhs[0] == '\"' && rhs[strlen(rhs) - 1] == '\"'){
		printf("String detected\n");
        return 3; // String
    }
    if(strlen(rhs) == 3 && rhs[0] == '\'' && rhs[2] == '\''){
		printf("Char detected\n");
        return 4; // Char
    }

    int dotCount = 0;
    for(int i = 0; i < strlen(rhs); i++) {
       if (strncmp(&rhs[i], ".", 1) == 0) {
           dotCount++;
       }
    }
    if(dotCount == 0){
		printf("Integer detected\n");
		return 1;
	} // Integer
    else if(dotCount == 1){
		printf("Float detected\n");
		return 2;
	}// Float

	if(!isdigit(rhs)){
		printf("Another Variable detected\n");
        return 0;
    }
    return -1; // More than one dot is invalid
}
// Gets the var type in from the variable list if it exists
VarType getVarType(const char *varName){
	for(int i = 0; i < VARS_MAX; i++){
		if(vars[i] == NULL) break;
		printf("Comparing with: %s\n", vars[i]->name);
		if(strcmp(vars[i]->name, varName) == 0){
			printf("Variable %s found with type %d\n", varName, vars[i]->type);
			return vars[i]->type;
		}
	}
	printf("Variable %s not found\n", varName);
	return UNDEFINED; // Not found
}

// Gets the int value of a variable if it exists and is of type INT
long getVarInt(const char *varName){
	long outValue = 0;
	for(int i = 0; i < VARS_MAX; i++){
		if(vars[i] == NULL) break;
		if(strcmp(vars[i]->name, varName) == 0){
			if(vars[i]->type == INT){
				outValue = vars[i]->value.intValue;
				return outValue; // Success
			}
			else{
				return -1; // Type mismatch
			}
		}
	}
	return -2; // Not found
}
// Gets the float value of a variable if it exists and is of type FLOAT
double getVarFloat(const char *varName){
	double outValue = 0.0;
	for(int i = 0; i < VARS_MAX; i++){
		if(vars[i] == NULL) break;
		if(strcmp(vars[i]->name, varName) == 0){
			if(vars[i]->type == FLOAT){
				outValue = vars[i]->value.floatValue;
				return outValue; // Success
			}
			else{
				return -1.0; // Type mismatch
			}
		}
	}
	return -2.0; // Not found
}

// Gets the string value of a variable if it exists and is of type STRING
char* getVarString(const char *varName){
	for(int i = 0; i < VARS_MAX; i++){
		if(vars[i] == NULL) break;
		if(strcmp(vars[i]->name, varName) == 0){
			if(vars[i]->type == STRING){
				return vars[i]->value.stringValue; // Success
			}
			else{
				return NULL; // Type mismatch
			}
		}
	}
	return NULL; // Not found
}

// Gets the char value of a variable if it exists and is of type CHAR
char *getVarChar(const char *varName){
	for(int i = 0; i < VARS_MAX; i++){
		if(vars[i] == NULL) break;
		if(strcmp(vars[i]->name, varName) == 0){
			if(vars[i]->type == CHAR){
				return vars[i]->value.charValue; // Success
			}
			else{
				return NULL; // Type mismatch
			}
		}
	}
	return NULL; // Not found
}

int doesVarExist(Variable *var){
	for(int i = 0; i < VARS_MAX; i++){
		if(vars[i] == NULL){
			printf("Var is NULL\n");
			return -2;
		}
		if(strcmp(var->name, vars[i]->name) == 0){
			printf("Var exists at index %d\n", i);
			return i; // Returns then index of the var;
		}
	}
	return -1; // Array full
}

void addVar(Variable *var){
	int check = doesVarExist(var);
	if(check <= -1){ // Does not exist or array full
		vars[0 + varCount] = var;
		printf("Adding var at index %d\n", 0 + varCount);
		varCount++; 
		
	}
	else{ // Exists update at the current index
		vars[check] = var;
		printf("Updating var at index %d\n", check);
		
	}
}

bool is_int(const char *s) {
	if (*s == '-' || *s == '+') s++; // Skip sign
	if (!*s) return 0; // Empty string after sign
	while (*s) {
		if (!isdigit((unsigned char)*s)) return false;
		s++;
	}
	return true;
}

bool is_float(const char *s) {
    char *endptr;
    strtod(s, &endptr);
    return *endptr == '\0' && endptr != s;
}

int main() {
	char cmd[MAX_CMD_LEN + 1];
	
	while (1) {
		printf(">>> ");
		fflush(stdout);
		int len = read_command(cmd);
		if (len < 0) { // EOF
			printf("\n");
			break;
		}
		if (len == 0) {
			// Empty line; continue
			continue;
		}
		OperationType op = parse_command(cmd);
		switch (op) {
			case ASSIGNMENT:
				{
					AssignmentInfo ai; 
					parse_assignment(cmd, &ai);
					if (ai.valid) {
						printf("[ASSIGNMENT] LHS='%s' RHS='%s' ", ai.lhs, ai.rhs);
                        Variable *var = (Variable *)malloc(sizeof(Variable));
						strncpy(var->name, ai.lhs, sizeof(var->name) - 1);
						var->name[sizeof(var->name) - 1] = '\0';

						if (ai.op == '[') {
							printf("List assignment detected: %s = %s\n", ai.lhs, ai.rhs);
							// Check for empty list
							if (strcmp(ai.rhs, "[]") == 0) {
								printf("Empty list initialized: %s\n", ai.lhs);
								addVar(var);
								var->type = LIST;
							}
						}
						else if (ai.arg_count == 1) {
							printf("ARGS: '%s'\n", ai.arg1);
                            switch (findVarType(ai.arg1)){
							case 0:
								printf("Variable assignment not implemented yet.\n");
								break;
                            case 1:
                                var->value.intValue = atoi(ai.arg1);
								var->type = INT;
								printf("Integer assigned: %ld\n", var->value.intValue);
                                break;
							case 2:
								var->value.floatValue = atof(ai.arg1);
								var->type = FLOAT;
								printf("Float assigned: %f\n", var->value.floatValue);
								break;
							case 3:
								var->type = STRING;
								// Remove quotes from arg1
								{
									size_t len = strlen(ai.arg1);
									if (len >= 2 && ai.arg1[0] == '"' && ai.arg1[len-1] == '"') {
										strncpy(var->value.stringValue, ai.arg1+1, len-2);
										var->value.stringValue[len-2] = '\0';
									} 
									else {
										strncpy(var->value.stringValue, ai.arg1, sizeof(var->value.stringValue) - 1);
										var->value.stringValue[sizeof(var->value.stringValue) - 1] = '\0';
									}
								}
								printf("String assigned: %s\n", var->value.stringValue);
								break;
							case 4:
								var->type = CHAR;
								var->value.charValue[0] = ai.arg1[1]; // middle character
								var->value.charValue[1] = '\0';
								printf("Char assigned: %s\n", var->value.charValue);
								break;
                            
                            default:
								printf("Unrecognized type.\n");
                                break;
                            }

							addVar(var);
						} 
						
						else if (ai.arg_count == 2) {
							printf("ARGS: '%s','%s' OP='%c'\n", ai.arg1, ai.arg2, ai.op);
							// Handle two-argument assignments with an operator
							bool arg1_is_int = is_int(ai.arg1);
							bool arg2_is_int = is_int(ai.arg2);
							bool arg1_is_float = !arg1_is_int && is_float(ai.arg1);
							bool arg2_is_float = !arg2_is_int && is_float(ai.arg2);
							if ((arg1_is_int || arg1_is_float) && (arg2_is_int || arg2_is_float)) {
								// Both are numbers
								double left = arg1_is_int ? atoi(ai.arg1) : atof(ai.arg1);
								double right = arg2_is_int ? atoi(ai.arg2) : atof(ai.arg2);
								switch (ai.op) {
									case '+':
										var->value.floatValue = left + right;
										break;
									case '-':
										var->value.floatValue = left - right;
										break;
									case '*':
										var->value.floatValue = left * right;
										break;
									case '/':
										if (right != 0) {
											var->value.floatValue = left / right;
										} else {
											printf("Error: Division by zero\n");
										}
										break;
									default:
										printf("Unsupported operator '%c'\n", ai.op);
										break;
								}
								// If both are int and operator is not division, store as int
								if (arg1_is_int && arg2_is_int && ai.op != '/') {
									var->type = INT;
									var->value.intValue = (long)var->value.floatValue;
									printf("Result (int): %ld\n", var->value.intValue);
								} else {
									var->type = FLOAT;
									printf("Result (float): %f\n", var->value.floatValue);
								}
								addVar(var);
						}
						
						else {
							VarType arg1Type = getVarType(ai.arg1);
							VarType arg2Type = getVarType(ai.arg2);
							if (arg1Type != UNDEFINED && arg2Type != UNDEFINED) {
								switch(ai.op) {
									case '+':
										if ((arg1Type == INT || arg1Type == FLOAT) && (arg2Type == INT || arg2Type == FLOAT)) {
											double left = (arg1Type == INT) ? getVarInt(ai.arg1) : getVarFloat(ai.arg1);
											double right = (arg2Type == INT) ? getVarInt(ai.arg2) : getVarFloat(ai.arg2);
											var->type =	 FLOAT;
											var->value.floatValue = left + right;
											printf("Result (numeric add): %f\n", var->value.floatValue);
											addVar(var);
										} else if (arg1Type == INT && arg2Type == INT) {
											var->type = INT;
											var->value.intValue = (long)(getVarInt(ai.arg1) + getVarInt(ai.arg2));
											printf("Result (int add): %ld\n", var->value.intValue);
											addVar(var);
										} else {
											printf("Error: Unsupported types for '+' operator\n");
										}
										break;
									case '*':
										if (arg1Type == INT && arg2Type == INT) {
											var->type = INT;
											var->value.intValue = (long)(getVarInt(ai.arg1) * getVarInt(ai.arg2));
											printf("Result (int multiply): %ld\n", var->value.intValue);
											addVar(var);
										} else if ((arg1Type == INT || arg1Type == FLOAT) && (arg2Type == INT || arg2Type == FLOAT)) {
											double left = (arg1Type == INT) ? getVarInt(ai.arg1) : getVarFloat(ai.arg1);
											double right = (arg2Type == INT) ? getVarInt(ai.arg2) : getVarFloat(ai.arg2);
											var->type =	 FLOAT;
											var->value.floatValue = left * right;
											printf("Result (numeric multiply): %f\n", var->value.floatValue);
											addVar(var);
										} else {
											printf("Error: Unsupported types for '*' operator\n");
										}
										break;
									case '-':
										if ((arg1Type == INT || arg1Type == FLOAT) && (arg2Type == INT || arg2Type == FLOAT)) {
											double left = (arg1Type == INT) ? getVarInt(ai.arg1) : getVarFloat(ai.arg1);
											double right = (arg2Type == INT) ? getVarInt(ai.arg2) : getVarFloat(ai.arg2);
											var->type =	 FLOAT;
											var->value.floatValue = left - right;
											printf("Result (numeric subtract): %f\n", var->value.floatValue);
											addVar(var);
										} else if (arg1Type == INT && arg2Type == INT) {
											var->type = INT;
											var->value.intValue = (long)(getVarInt(ai.arg1) - getVarInt(ai.arg2));
											printf("Result (int subtract): %ld\n", var->value.intValue);
											addVar(var);
										} else {
											printf("Error: Unsupported types for '-' operator\n");
										}
										break;
									case '/':
										if(arg1Type == INT && arg2Type == INT){
											int left = getVarInt(ai.arg1);
											int right = getVarInt(ai.arg2);
											if(right != 0){
												var->type = INT;
												var->value.intValue = left / right;
												printf("Result (int divide): %ld\n", var->value.intValue);
												addVar(var);
											}
											else{
												printf("Error: Division by zero\n");
											}
										}
										else if ((arg1Type == INT || arg1Type == FLOAT) && (arg2Type == INT || arg2Type == FLOAT)) {
											double left = (arg1Type == INT) ? getVarInt(ai.arg1) : getVarFloat(ai.arg1);
											double right = (arg2Type == INT) ? getVarInt(ai.arg2) : getVarFloat(ai.arg2);
											if (right != 0) {
												var->type =	 FLOAT;
												var->value.floatValue = left / right;
												printf("Result (numeric divide): %f\n", var->value.floatValue);
												addVar(var);
											} else {
												printf("Error: Division by zero\n");
											}
										} else {
											printf("Error: Unsupported types for '/' operator\n");
										}
										break;
									default:
										printf("Unsupported operator '%c' for variable types\n", ai.op);
										break;
								}

							} else if(arg1Type != UNDEFINED && arg2Type == UNDEFINED) {
								double left = getVarInt(ai.arg1);
								int right = findVarType(ai.arg2);


								switch(ai.op) {
									
									
									case '+':
										if (arg1Type == INT) {	
											if(right == 1) {
												right = atoi(ai.arg2);
												var->type = INT;
												var->value.intValue = (long)(left + right);
											} else if (right == 2) {
												right = atof(ai.arg2);
												var->type = FLOAT;
												var->value.floatValue = left + right;
											} else {
												printf("Error: Unsupported type for second argument\n");
												break;
											}
											addVar(var);
										} else if (arg1Type == FLOAT) {
											var->type = FLOAT;
											var->value.floatValue = getVarFloat(ai.arg1) + atof(ai.arg2);
											printf("Result (float add): %f\n", var->value.floatValue);
											addVar(var);
										} else {
											printf("Error: Unsupported types for '+' operator\n");
										}
										break;
									case '*':
										if (arg1Type == INT) {	
											if(right == 1) {
												right = atoi(ai.arg2);
												var->type = INT;
												var->value.intValue = (long)(left * right);
											} else if (right == 2) {
												right = atof(ai.arg2);
												var->type = FLOAT;
												var->value.floatValue = left * right;
											} else {
												printf("Error: Unsupported type for second argument\n");
												break;
											}
											addVar(var);
										} else if (arg1Type == FLOAT) {
											var->type = FLOAT;
											var->value.floatValue = getVarFloat(ai.arg1) * atof(ai.arg2);
											printf("Result (float add): %f\n", var->value.floatValue);
											addVar(var);
										} else {
											printf("Error: Unsupported types for '+' operator\n");
										}
										break;
									case '-':
										if (arg1Type == INT) {	
											if(right == 1) {
												right = atoi(ai.arg2);
												var->type = INT;
												var->value.intValue = (long)(left - right);
											} else if (right == 2) {
												right = atof(ai.arg2);
												var->type = FLOAT;
												var->value.floatValue = left - right;
											} else {
												printf("Error: Unsupported type for second argument\n");
												break;
											}
											addVar(var);
										} else if (arg1Type == FLOAT) {
											var->type = FLOAT;
											var->value.floatValue = getVarFloat(ai.arg1) - atof(ai.arg2);
											printf("Result (float subtract): %f\n", var->value.floatValue);
											addVar(var);
										} else {
											printf("Error: Unsupported types for '+' operator\n");
										}
									case '/':
										if (arg1Type == INT) {	
											if(right == 1) {
												right = atoi(ai.arg2);
												var->type = INT;
												var->value.intValue = (long)(left / right);
											} else if (right == 2) {
												right = atof(ai.arg2);
												var->type = FLOAT;
												var->value.floatValue = left / right;
											} else {
												printf("Error: Unsupported type for second argument\n");
												break;
											}
											addVar(var);
										} else if (arg1Type == FLOAT) {
											var->type = FLOAT;
											var->value.floatValue = getVarFloat(ai.arg1) / atof(ai.arg2);
											printf("Result (float divide): %f\n", var->value.floatValue);
											addVar(var);
										} else {
											printf("Error: Unsupported types for '/' operator\n");
										}
										break;
									default:
										printf("Unsupported operator '%c' for variable types\n", ai.op);
										break;
									}
								} 
								else if(arg1Type == UNDEFINED && arg2Type != UNDEFINED) {
									switch(ai.op) {
										double right = getVarInt(ai.arg2);
										double left = findVarType(ai.arg1);
										case '+':
											if (arg2Type == INT) {	
												if(left == 1) {
													left = atoi(ai.arg1);
													var->type = INT;
													var->value.intValue = (long)(left + right);
												} else if (left == 2) {
													left = atof(ai.arg1);
													var->type = FLOAT;
													var->value.floatValue = left + right;
												} else {
													printf("Error: Unsupported type for first argument\n");
													break;
												}
												addVar(var);
											} else if (arg2Type == FLOAT) {
												var->type = FLOAT;
												var->value.floatValue = atof(ai.arg1) + getVarFloat(ai.arg2);
												printf("Result (float add): %f\n", var->value.floatValue);
												addVar(var);
											} else {
												printf("Error: Unsupported types for '+' operator\n");
											}
											break;
										case '*':
											if (arg2Type == INT) {	
												if(left == 1) {
													left = atoi(ai.arg1);
													var->type = INT;
													var->value.intValue = (long)(left * right);
												} else if (left == 2) {	
													left = atof(ai.arg1);
													var->type = FLOAT;
													var->value.floatValue = left * right;
												} else {
													printf("Error: Unsupported type for first argument\n");
													break;
												}
												addVar(var);
											} else if (arg2Type == FLOAT) {
												var->type = FLOAT;
												var->value.floatValue = atof(ai.arg1) * getVarFloat(ai.arg2);
												printf("Result (float multiply): %f\n", var->value.floatValue);
												addVar(var);
											} else {
												printf("Error: Unsupported types for '*' operator\n");
											}
											break;
										case '-':
											if (arg2Type == INT) {	
												if(left == 1) {
													left = atoi(ai.arg1);
													var->type = INT;
													var->value.intValue = (long)(left - right);
												} else if (left == 2) {
													left = atof(ai.arg1);
													var->type = FLOAT;
													var->value.floatValue = left - right;
												} else {
													printf("Error: Unsupported type for first argument\n");
													break;
												}
												addVar(var);
											} else if (arg2Type == FLOAT) {
												var->type = FLOAT;
												var->value.floatValue = atof(ai.arg1) - getVarFloat(ai.arg2);
												printf("Result (float subtract): %f\n", var->value.floatValue);
												addVar(var);
											} else {
												printf("Error: Unsupported types for '-' operator\n");
											}
											break;
										case '/':
											if (arg2Type == INT) {	
												if(left == 1) {
													left = atoi(ai.arg1);
													var->type = INT;
													var->value.intValue = (long)(left / right);
												} else if (left == 2) {
													left = atof(ai.arg1);
													var->type = FLOAT;
													var->value.floatValue = left / right;
												} else {
													printf("Error: Unsupported type for first argument\n");
													break;
												}
												addVar(var);
											} else if (arg2Type == FLOAT) {
												var->type = FLOAT;
												var->value.floatValue = atof(ai.arg1) / getVarFloat(ai.arg2);
												printf("Result (float divide): %f\n", var->value.floatValue);
												addVar(var);
											} else {
												printf("Error: Unsupported types for '/' operator\n");
											}
											break;
										default:
											printf("Unsupported operator '%c' for variable types\n", ai.op);
											break;
									}
								}
								else {
									printf("(parse error)\n");
								}
							}
						}

					}
				break;
				
			
			
			case PRINT:
				{
					PrintInfo pi; 
					parse_print(cmd, &pi);
					if (pi.valid) {
						//printf("[PRINT] VAR='%s'\n", pi.var);
					} else {
						printf("[PRINT] (parse error)\n");
					}
                    print(pi.var);
				}
				break;
			case APPEND:
				{
					AppendInfo api;
					parse_append(cmd, &api);
					if (api.valid) {
						printf("[APPEND] LIST='%s' ITEM='%s'\n", api.arg1, api.arg2);
					} else {
						printf("[APPEND] (parse error)\n");
					}
				}
				break;
			default:
				printf("[UNKNOWN]\n");
				break;
		}
	}

}
}
