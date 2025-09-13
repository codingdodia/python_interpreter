#ifndef STRUCTS_H
#define STRUCTS_H

typedef union
{
    long intValue;
    double floatValue;
    char charValue[1 + 1];
    char stringValue[64 + 1];
} VarValue;

typedef enum
{
    VARIABLE,
    INT,
    FLOAT,
    STRING,
    CHAR,
    LIST,
    UNDEFINED
} VarType;

typedef enum
{
    ASSIGNMENT,
    PRINT,
    APPEND,
    UNKNOWN
} OperationType;

typedef struct
{
    char lhs[64];
    char rhs[128];
    char arg1[64];
    char arg2[64];
    char op;
    int arg_count;
    int valid;
} AssignmentInfo;

typedef struct
{
    char var[64];
    int valid;
} PrintInfo;

typedef struct listNode
{
    union
    {
        long intValue;
        double floatValue;
        char charValue[1 + 1];
        char stringValue[64 + 1];
    } value;
    VarType type;
    int index;
    struct listNode *next;
} listNode;

typedef struct
{
    char name[64 + 1];
    VarType type;
    VarValue value;
    listNode *next;
} Variable;

typedef struct
{
    char arg1[64];
    char arg2[64];
    int valid;
} AppendInfo;

#endif // STRUCTS_H
