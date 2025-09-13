#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "structs.h"
#include "helper.h"

void print(const char *s)

{ // Takes the char as input and then parses it to see if its a string or variablable.
    if (s == NULL)
    {
        printf("(null)\n");
        return;
    }
    const char print;
    if (strncmp(s, "\"", 1) == 0)
    {
        for (int i = 0; i < strlen(s); i++)
        {
            if (s[i] == '\"')
                continue;
            printf("%c", s[i]);
        }
        printf("\n");
        return;
    }
    else
    {

        for (int i = 0; i < VARS_MAX; i++)
        {
            if (vars[i] == NULL)
                break;
           
            if (vars[i] && strcmp(vars[i]->name, s) == 0)
            {
                switch (vars[i]->type)
                {
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
                    listNode *current = vars[i]->next;
                    if (current == NULL)
                    {
                        printf("List is empty.\n");
                        return;
                    }
                    int count = 0;
                    printf("[");
                    while (current != NULL)
                    {
                        switch (current->type)
                        {
                        case INT:
                            printf("%d", current->value.intValue);
                            break;
                        case FLOAT:
                            printf("%lf", current->value.floatValue);
                            break;
                        case STRING:
                            printf("\"%s\"", current->value.stringValue);
                            break;
                        case CHAR:
                            printf("\'%c\'", current->value.charValue[0]);
                            break;
                        default:
                            break;
                        }
                        current = current->next;
                        if (current != NULL)
                            printf(", ");
                    }
                    printf("]\n");
                default:
                    break;
                }
            }
        }
        return;
    }
}

// Parses the index from a list syntax string (e.g., "a[2]")
int parse_list_index(const char *arg)
{
    if (!arg)
        return -1;
    char *bracket_open = strchr(arg, '[');
    char *bracket_close = strchr(arg, ']');
    if (bracket_open && bracket_close && bracket_close > bracket_open)
    {
        char index_str[16];
        size_t len = bracket_close - bracket_open - 1;
        if (len > 0 && len < sizeof(index_str))
        {
            strncpy(index_str, bracket_open + 1, len);
            index_str[len] = '\0';
            return atoi(index_str);
        }
    }
    return -1;
}

// Remove all whitespace from a string
void remove_whitespace(const char *src, char *dest)
{
    while (*src)
    {
        if (!isspace((unsigned char)*src))
        {
            *dest++ = *src;
        }
        src++;
    }
    *dest = '\0';
}

OperationType parse_command(const char *cmd)
{
    char cleaned[256];
    remove_whitespace(cmd, cleaned);
    if (strncmp(cleaned, "print(", 6) == 0)
    {
        return PRINT;
    }
    else if (strncmp(cleaned, "append(", 7) == 0)
    {
        return APPEND;
    }
    else
    {
        const char *eq = strchr(cleaned, '=');
        if (eq)
        {
            int pos = (int)(eq - cleaned);
            if (pos > 0 && isalpha((unsigned char)cleaned[0]))
            {
                return ASSIGNMENT;
            }
        }
    }
    return UNKNOWN;
}

// Parse an assignment (supports optional single '+')
void parse_assignment(const char *cmd, AssignmentInfo *info)
{
    memset(info, 0, sizeof(*info));
    char cleaned[256];
    remove_whitespace(cmd, cleaned);
    const char *eq = strchr(cleaned, '=');
    if (!eq)
        return;
    size_t lhs_len = (size_t)(eq - cleaned);
    if (lhs_len == 0 || lhs_len >= sizeof(info->lhs))
        return;
    strncpy(info->lhs, cleaned, lhs_len);
    info->lhs[lhs_len] = '\0';
    const char *rhs_start = eq + 1;
    if (*rhs_start == '\0')
        return;
    strncpy(info->rhs, rhs_start, sizeof(info->rhs) - 1);
    info->rhs[sizeof(info->rhs) - 1] = '\0';
    // Check for list assignment or list indexing (e.g., a[0])
    char *bracket_open = strchr(info->rhs, '[');
    char *bracket_close = strchr(info->rhs, ']');
    if (bracket_open && bracket_close && bracket_close > bracket_open)
    {
        info->op = '[';
        info->arg_count = 2;
        // Extract list name
        size_t list_name_len = bracket_open - info->rhs;
        if (list_name_len >= sizeof(info->arg1))
            list_name_len = sizeof(info->arg1) - 1;
        strncpy(info->arg1, info->rhs, list_name_len);
        info->arg1[list_name_len] = '\0';
        // Extract index
        size_t index_len = bracket_close - bracket_open - 1;
        if (index_len >= sizeof(info->arg2))
            index_len = sizeof(info->arg2) - 1;
        strncpy(info->arg2, bracket_open + 1, index_len);
        info->arg2[index_len] = '\0';
        info->valid = 1;
    }
    // ...existing code...
    // Look for operator: +, -, *, /
    const char *op_ptr = NULL;
    char ops[] = "+-*/";
    for (int i = 0; ops[i]; i++)
    {
        op_ptr = strchr(info->rhs, ops[i]);
        if (op_ptr)
        {
            info->op = ops[i];
            break;
        }
    }
    if (op_ptr)
    {
        size_t a_len = (size_t)(op_ptr - info->rhs);
        if (a_len == 0 || a_len >= sizeof(info->arg1))
            return;
        strncpy(info->arg1, info->rhs, a_len);
        info->arg1[a_len] = '\0';
        const char *second = op_ptr + 1;
        if (*second == '\0')
            return;
        strncpy(info->arg2, second, sizeof(info->arg2) - 1);
        info->arg2[sizeof(info->arg2) - 1] = '\0';
        info->arg_count = 2;
    }
    else
    {
        strncpy(info->arg1, info->rhs, sizeof(info->arg1) - 1);
        info->arg1[sizeof(info->arg1) - 1] = '\0';
        info->arg_count = 1;
    }
    info->valid = 1;
}

void parse_print(const char *cmd, PrintInfo *pi)
{
    memset(pi, 0, sizeof(*pi));
    char cleaned[256];
    remove_whitespace(cmd, cleaned);
    if (strncmp(cleaned, "print(", 6) != 0)
        return;
    const char *inside = cleaned + 6; // after 'print('
    const char *end = strchr(inside, ')');
    if (!end || end == inside)
        return;
    size_t len = (size_t)(end - inside);
    if (len >= sizeof(pi->var))
        len = sizeof(pi->var) - 1;
    strncpy(pi->var, inside, len);
    pi->var[len] = '\0';
    pi->valid = 1;
}

void parse_append(const char *cmd, AppendInfo *ai)
{
    memset(ai, 0, sizeof(*ai));
    char cleaned[256];
    remove_whitespace(cmd, cleaned);
    if (strncmp(cleaned, "append(", 7) != 0)
        return;
    const char *inside = cleaned + 7; // after 'append('
    const char *end = strchr(inside, ')');
    if (!end || end == inside)
        return;
    const char *comma = strchr(inside, ',');
    if (!comma || comma > end)
        return;
    size_t len1 = (size_t)(comma - inside);
    size_t len2 = (size_t)(end - (comma + 1));
    if (len1 == 0 || len2 == 0)
        return;
    // Parse arg1 for list indexing (e.g., a[0])
    char *bracket_open1 = strchr(inside, '[');
    char *bracket_close1 = strchr(inside, ']');
    if (bracket_open1 && bracket_close1 && bracket_close1 < comma)
    {
        size_t list_name_len = bracket_open1 - inside;
        if (list_name_len >= sizeof(ai->arg1))
            list_name_len = sizeof(ai->arg1) - 1;
        strncpy(ai->arg1, inside, list_name_len);
        ai->arg1[list_name_len] = '\0';
        // Optionally, you can store the index in a separate field if needed
    }
    else
    {
        if (len1 >= sizeof(ai->arg1))
            len1 = sizeof(ai->arg1) - 1;
        strncpy(ai->arg1, inside, len1);
        ai->arg1[len1] = '\0';
    }
    // Parse arg2 for list indexing (e.g., b[1])
    char *bracket_open2 = strchr(comma + 1, '[');
    char *bracket_close2 = strchr(comma + 1, ']');
    if (bracket_open2 && bracket_close2 && bracket_close2 < end)
    {
        size_t var_name_len = bracket_open2 - (comma + 1);
        if (var_name_len >= sizeof(ai->arg2))
            var_name_len = sizeof(ai->arg2) - 1;
        strncpy(ai->arg2, comma + 1, var_name_len);
        ai->arg2[var_name_len] = '\0';
        // Optionally, you can store the index in a separate field if needed
    }
    else
    {
        if (len2 >= sizeof(ai->arg2))
            len2 = sizeof(ai->arg2) - 1;
        strncpy(ai->arg2, comma + 1, len2);
        ai->arg2[len2] = '\0';
    }
    ai->valid = 1;
}

// Reads up to MAX_CMD_LEN characters (excluding null terminator) until newline or EOF.
// Returns length read, or -1 on EOF with no characters read.
int read_command(char *buffer)
{
    int idx = 0;
    int c;
    while (idx < MAX_CMD_LEN && (c = getchar()) != EOF)
    {
        if (c == '\n')
            break;
        buffer[idx++] = (char)c;
    }
    if (c == EOF && idx == 0)
        return -1; // EOF before any data
    buffer[idx] = '\0';
    // If line longer than MAX_CMD_LEN, discard remainder until newline or EOF
    if (c != '\n')
    {
        while (c != '\n' && c != EOF)
        {
            c = getchar();
        }
    }
    return idx;
}
Variable *getVar(const char *varName)
{
    for (int i = 0; i < VARS_MAX; i++)
    {
        if (vars[i] == NULL)
            break;
        if (strcmp(vars[i]->name, varName) == 0)
        {
            return vars[i]; // Success
        }
    }
    Variable *undefinedVar = (Variable *)malloc(sizeof(Variable));
    undefinedVar->type = UNDEFINED;
    return undefinedVar; // Not found
}
// Checks the type of var if it exists. Used for arguments inputed
VarType getVarType(const char *varName)
{
    for (int i = 0; i < VARS_MAX; i++)
    {
        if (vars[i] == NULL)
            break;
        ;
        if (strcmp(vars[i]->name, varName) == 0)
        {

            return vars[i]->type;
        }
    }
    return UNDEFINED; // Not found
}

VarType findVarType(const char *rhs)
{

    if (rhs == NULL || *rhs == '\0')
        return UNDEFINED;
    if (rhs[0] == '\"' && rhs[strlen(rhs) - 1] == '\"')
    {

        return STRING;
    }
    if (strlen(rhs) == 3 && rhs[0] == '\'' && rhs[2] == '\'')
    {

        return CHAR;
    }

    if (rhs[strlen(rhs) - 1] == ']')
    {

        return LIST;
    }
    if (doesVarExist(getVar(rhs)) >= 0)
    {

        return VARIABLE;
    }

    int dotCount = 0;
    for (int i = 0; i < strlen(rhs); i++)
    {
        if (strncmp(&rhs[i], ".", 1) == 0)
        {
            dotCount++;
        }
    }
    if (dotCount == 0)
    {

        return INT;
    } // Integer
    else if (dotCount == 1)
    {

        return FLOAT;
    } // Float

    if (!isdigit((unsigned char)rhs[0]))
    {

        return 0;
    }
    return -1; // More than one dot is invalid
}
// Gets the var type in from the variable list if it exists

// Gets the int value of a variable if it exists and is of type INT
long getVarInt(const char *varName)
{
    long outValue = 0;
    for (int i = 0; i < VARS_MAX; i++)
    {
        if (vars[i] == NULL)
            break;
        if (strcmp(vars[i]->name, varName) == 0)
        {
            if (vars[i]->type == INT)
            {
                outValue = vars[i]->value.intValue;
                return outValue; // Success
            }
            else
            {
                return -1; // Type mismatch
            }
        }
    }
    return -2; // Not found
}
// Gets the float value of a variable if it exists and is of type FLOAT
double getVarFloat(const char *varName)
{
    double outValue = 0.0;
    for (int i = 0; i < VARS_MAX; i++)
    {
        if (vars[i] == NULL)
            break;
        if (strcmp(vars[i]->name, varName) == 0)
        {
            if (vars[i]->type == FLOAT)
            {
                outValue = vars[i]->value.floatValue;
                return outValue; // Success
            }
            else
            {
                return -1.0; // Type mismatch
            }
        }
    }
    return -2.0; // Not found
}

// Gets the string value of a variable if it exists and is of type STRING
char *getVarString(const char *varName)
{
    for (int i = 0; i < VARS_MAX; i++)
    {
        if (vars[i] == NULL)
            break;
        if (strcmp(vars[i]->name, varName) == 0)
        {
            if (vars[i]->type == STRING)
            {
                return vars[i]->value.stringValue; // Success
            }
            else
            {
                return NULL; // Type mismatch
            }
        }
    }
    return NULL; // Not found
}

// Gets the char value of a variable if it exists and is of type CHAR
char *getVarChar(const char *varName)
{
    for (int i = 0; i < VARS_MAX; i++)
    {
        if (vars[i] == NULL)
            break;
        if (strcmp(vars[i]->name, varName) == 0)
        {
            if (vars[i]->type == CHAR)
            {
                return vars[i]->value.charValue; // Success
            }
            else
            {
                return NULL; // Type mismatch
            }
        }
    }
    return NULL; // Not found
}

int doesVarExist(Variable *var)
{
    for (int i = 0; i < VARS_MAX; i++)
    {
        if (vars[i] == NULL)
        {
            return -2;
        }
        if (strcmp(var->name, vars[i]->name) == 0)
        {
            return i; // Returns then index of the var;
        }
    }
    return -1; // Array full
}

void addVar(Variable *var)
{
    int check = doesVarExist(var);
    if (check <= -1)
    { // Does not exist or array full
        vars[0 + varCount] = var;

        varCount++;
    }
    else
    { // Exists update at the current index
        vars[check] = var;
    }
}

bool is_int(const char *s)
{
    if (*s == '-' || *s == '+')
        s++; // Skip sign
    if (!*s)
        return 0; // Empty string after sign
    while (*s)
    {
        if (!isdigit((unsigned char)*s))
            return false;
        s++;
    }
    return true;
}

bool is_float(const char *s)
{
    char *endptr;
    strtod(s, &endptr);
    return *endptr == '\0' && endptr != s;
}

listNode *copyList(Variable *var)
{
    if (var == NULL || var->type != LIST)
    {
        return NULL;
    }

    listNode *current = var->next;
    listNode *newHead = (listNode *)malloc(sizeof(listNode));
    while (current != NULL)
    { // Copying the entire node
        newHead->value = current->value;
        newHead->type = current->type;
        newHead->index = current->index;
        if (current->next == NULL)
        {
            break;
        }
        newHead->next = (listNode *)malloc(sizeof(listNode));
        newHead = newHead->next;
        current = current->next;
    }
    newHead->next = NULL;
    return newHead;
}

listNode *getListNode(const char *varName, int index) // Used to get a specific index from a list variable
{

    Variable *var = getVar(varName);
    if (var == NULL || var->type != LIST)
    {
        return NULL;
    }
    listNode *current = var->next;
    while (current != NULL)
    {
        if (current->index == index)
        {
            return current; // Found the node at the specified index
        }
        current = current->next;
    }
    return NULL; // Index not found
}