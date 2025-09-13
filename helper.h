#ifndef HELPER_H
#define HELPER_H

#include <stdbool.h>
#include "structs.h"

#define MAX_CMD_LEN 100
#define VARS_MAX 100
extern int varCount;
extern Variable *vars[VARS_MAX];

void print(const char *s);
int parse_list_index(const char *arg);
void remove_whitespace(const char *src, char *dest);
OperationType parse_command(const char *cmd);
void parse_assignment(const char *cmd, AssignmentInfo *info);
void parse_print(const char *cmd, PrintInfo *pi);
void parse_append(const char *cmd, AppendInfo *ai);
int read_command(char *buffer);
Variable *getVar(const char *varName);
VarType getVarType(const char *varName);
VarType findVarType(const char *rhs);
long getVarInt(const char *varName);
double getVarFloat(const char *varName);
char *getVarString(const char *varName);
char *getVarChar(const char *varName);
int doesVarExist(Variable *var);
void addVar(Variable *var);
bool is_int(const char *s);
bool is_float(const char *s);
listNode *copyList(Variable *var);
listNode *getListNode(const char *varName, int index);

#endif // HELPER_H
