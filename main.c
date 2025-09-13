
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "structs.h"
#include "helper.h"

// Array of pointers to Variable structs
int varCount = 0;
Variable *vars[VARS_MAX] = {0};
int main()
{
	char cmd[MAX_CMD_LEN + 1];

	while (1)
	{
		printf(">>> ");
		fflush(stdout);
		int len = read_command(cmd);
		if (len < 0)
		{ // EOF
			printf("\n");
			break;
		}
		if (len == 0)
		{
			continue;
		}
		OperationType op = parse_command(cmd);
		switch (op)
		{
		case ASSIGNMENT:
		{
			AssignmentInfo ai;
			parse_assignment(cmd, &ai);
			if (ai.valid)
			{
				// printf("[ASSIGNMENT] LHS='%s' RHS='%s' ", ai.lhs, ai.rhs);
				Variable *var = (Variable *)malloc(sizeof(Variable)); // Allocate memory for new variable
				strncpy(var->name, ai.lhs, sizeof(var->name) - 1);
				var->name[sizeof(var->name) - 1] = '\0';

				if (ai.op == '[') // Checks for list assignment or indexing
				{
					if (strcmp(ai.rhs, "[]") == 0)
					{
						// printf("Empty list initialized: %s\n", ai.lhs);
						var->type = LIST;
						addVar(var);
					}
				}
				else if (ai.arg_count == 1) // Single Assignment
				{

					switch (findVarType(ai.arg1))
					{
					case 0:
						printf("Variable assignment not implemented yet.\n");
						break;
					case 1:
						var->value.intValue = atoi(ai.arg1);
						var->type = INT;
						break;
					case 2:
						var->value.floatValue = atof(ai.arg1);
						var->type = FLOAT;
						;
						break;
					case 3:
						var->type = STRING;
						// Remove quotes from arg1
						{
							size_t len = strlen(ai.arg1);
							if (len >= 2 && ai.arg1[0] == '"' && ai.arg1[len - 1] == '"')
							{
								strncpy(var->value.stringValue, ai.arg1 + 1, len - 2);
								var->value.stringValue[len - 2] = '\0';
							}
							else
							{
								strncpy(var->value.stringValue, ai.arg1, sizeof(var->value.stringValue) - 1);
								var->value.stringValue[sizeof(var->value.stringValue) - 1] = '\0';
							}
						}
						break;
					case 4:
						var->type = CHAR;
						var->value.charValue[0] = ai.arg1[1]; // middle character
						var->value.charValue[1] = '\0';
						break;

					default:
						printf("Unrecognized type.\n");
						break;
					}

					addVar(var);
				}

				else if (ai.arg_count == 2)
				{
					// Handle two-argument assignments with an operator
					VarType arg1Type = findVarType(ai.arg1);
					VarType arg2Type = findVarType(ai.arg2);
					bool arg1_is_int = is_int(ai.arg1);
					bool arg2_is_int = is_int(ai.arg2);
					bool arg1_is_float = !arg1_is_int && is_float(ai.arg1);
					bool arg2_is_float = !arg2_is_int && is_float(ai.arg2);
					if ((arg1_is_int || arg1_is_float) && (arg2_is_int || arg2_is_float))
					{
						// Both are numbers
						double left = arg1_is_int ? atoi(ai.arg1) : atof(ai.arg1);
						double right = arg2_is_int ? atoi(ai.arg2) : atof(ai.arg2);
						switch (ai.op)
						{
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
							if (right != 0)
							{
								var->value.floatValue = left / right;
							}
							else
							{
								printf("Error: Division by zero\n");
							}
							break;
						default:
							printf("Unsupported operator '%c'\n", ai.op);
							break;
						}
						// If both are int and operator is not division, store as int
						if (arg1_is_int && arg2_is_int && ai.op != '/')
						{
							var->type = INT;
							var->value.intValue = (long)var->value.floatValue;
						}
						else
						{
							var->type = FLOAT;
						}
						addVar(var);
					}

					else if (arg1Type == VARIABLE || arg2Type == VARIABLE)
					{
						VarType arg1Type = getVarType(ai.arg1);
						VarType arg2Type = getVarType(ai.arg2);
						if (arg1Type != UNDEFINED && arg2Type != UNDEFINED)
						{
							switch (ai.op)
							{
							case '+':
								if ((arg1Type == INT || arg1Type == FLOAT) && (arg2Type == INT || arg2Type == FLOAT))
								{
									double left = (arg1Type == INT) ? getVarInt(ai.arg1) : getVarFloat(ai.arg1);
									double right = (arg2Type == INT) ? getVarInt(ai.arg2) : getVarFloat(ai.arg2);
									var->type = FLOAT;
									var->value.floatValue = left + right;

									addVar(var);
								}
								else if (arg1Type == INT && arg2Type == INT)
								{
									var->type = INT;
									var->value.intValue = (long)(getVarInt(ai.arg1) + getVarInt(ai.arg2));

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '+' operator\n");
								}
								break;
							case '*':
								if (arg1Type == INT && arg2Type == INT)
								{
									var->type = INT;
									var->value.intValue = (long)(getVarInt(ai.arg1) * getVarInt(ai.arg2));

									addVar(var);
								}
								else if ((arg1Type == INT || arg1Type == FLOAT) && (arg2Type == INT || arg2Type == FLOAT))
								{
									double left = (arg1Type == INT) ? getVarInt(ai.arg1) : getVarFloat(ai.arg1);
									double right = (arg2Type == INT) ? getVarInt(ai.arg2) : getVarFloat(ai.arg2);
									var->type = FLOAT;
									var->value.floatValue = left * right;

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '*' operator\n");
								}
								break;
							case '-':
								if ((arg1Type == INT || arg1Type == FLOAT) && (arg2Type == INT || arg2Type == FLOAT))
								{
									double left = (arg1Type == INT) ? getVarInt(ai.arg1) : getVarFloat(ai.arg1);
									double right = (arg2Type == INT) ? getVarInt(ai.arg2) : getVarFloat(ai.arg2);
									var->type = FLOAT;
									var->value.floatValue = left - right;

									addVar(var);
								}
								else if (arg1Type == INT && arg2Type == INT)
								{
									var->type = INT;
									var->value.intValue = (long)(getVarInt(ai.arg1) - getVarInt(ai.arg2));

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '-' operator\n");
								}
								break;
							case '/':
								if (arg1Type == INT && arg2Type == INT)
								{
									int left = getVarInt(ai.arg1);
									int right = getVarInt(ai.arg2);
									if (right != 0)
									{
										var->type = INT;
										var->value.intValue = left / right;

										addVar(var);
									}
									else
									{
										printf("Error: Division by zero\n");
									}
								}
								else if ((arg1Type == INT || arg1Type == FLOAT) && (arg2Type == INT || arg2Type == FLOAT))
								{
									double left = (arg1Type == INT) ? getVarInt(ai.arg1) : getVarFloat(ai.arg1);
									double right = (arg2Type == INT) ? getVarInt(ai.arg2) : getVarFloat(ai.arg2);
									if (right != 0)
									{
										var->type = FLOAT;
										var->value.floatValue = left / right;

										addVar(var);
									}
									else
									{
										printf("Error: Division by zero\n");
									}
								}
								else
								{
									printf("Error: Unsupported types for '/' operator\n");
								}
								break;
							default:
								printf("Unsupported operator '%c' for variable types\n", ai.op);
								break;
							}
						}
						else if (arg1Type != UNDEFINED && arg2Type == UNDEFINED)
						{
							double left = getVarInt(ai.arg1);
							int right = findVarType(ai.arg2);

							switch (ai.op)
							{

							case '+':
								if (arg1Type == INT)
								{
									if (right == 1)
									{
										right = atoi(ai.arg2);
										var->type = INT;
										var->value.intValue = (long)(left + right);
									}
									else if (right == 2)
									{
										right = atof(ai.arg2);
										var->type = FLOAT;
										var->value.floatValue = left + right;
									}
									else
									{
										printf("Error: Unsupported type for second argument\n");
										break;
									}
									addVar(var);
								}
								else if (arg1Type == FLOAT)
								{
									var->type = FLOAT;
									var->value.floatValue = getVarFloat(ai.arg1) + atof(ai.arg2);

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '+' operator\n");
								}
								break;
							case '*':
								if (arg1Type == INT)
								{
									if (right == 1)
									{
										right = atoi(ai.arg2);
										var->type = INT;
										var->value.intValue = (long)(left * right);
									}
									else if (right == 2)
									{
										right = atof(ai.arg2);
										var->type = FLOAT;
										var->value.floatValue = left * right;
									}
									else
									{
										printf("Error: Unsupported type for second argument\n");
										break;
									}
									addVar(var);
								}
								else if (arg1Type == FLOAT)
								{
									var->type = FLOAT;
									var->value.floatValue = getVarFloat(ai.arg1) * atof(ai.arg2);

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '+' operator\n");
								}
								break;
							case '-':
								if (arg1Type == INT)
								{
									if (right == 1)
									{
										right = atoi(ai.arg2);
										var->type = INT;
										var->value.intValue = (long)(left - right);
									}
									else if (right == 2)
									{
										right = atof(ai.arg2);
										var->type = FLOAT;
										var->value.floatValue = left - right;
									}
									else
									{
										printf("Error: Unsupported type for second argument\n");
										break;
									}
									addVar(var);
								}
								else if (arg1Type == FLOAT)
								{
									var->type = FLOAT;
									var->value.floatValue = getVarFloat(ai.arg1) - atof(ai.arg2);

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '+' operator\n");
								}
							case '/':
								if (arg1Type == INT)
								{
									if (right == 1)
									{
										right = atoi(ai.arg2);
										var->type = INT;
										var->value.intValue = (long)(left / right);
									}
									else if (right == 2)
									{
										right = atof(ai.arg2);
										var->type = FLOAT;
										var->value.floatValue = left / right;
									}
									else
									{
										printf("Error: Unsupported type for second argument\n");
										break;
									}
									addVar(var);
								}
								else if (arg1Type == FLOAT)
								{
									var->type = FLOAT;
									var->value.floatValue = getVarFloat(ai.arg1) / atof(ai.arg2);

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '/' operator\n");
								}
								break;
							default:
								printf("Unsupported operator '%c' for variable types\n", ai.op);
								break;
							}
						}
						else if (arg1Type == UNDEFINED && arg2Type != UNDEFINED)
						{
							double right = getVarInt(ai.arg2);
							double left = findVarType(ai.arg1);
							switch (ai.op)
							{

							case '+':
								if (arg2Type == INT)
								{
									if (left == 1)
									{
										left = atoi(ai.arg1);
										var->type = INT;
										var->value.intValue = (long)(left + right);
									}
									else if (left == 2)
									{
										left = atof(ai.arg1);
										var->type = FLOAT;
										var->value.floatValue = left + right;
									}
									else
									{
										printf("Error: Unsupported type for first argument\n");
										break;
									}
									addVar(var);
								}
								else if (arg2Type == FLOAT)
								{
									var->type = FLOAT;
									var->value.floatValue = atof(ai.arg1) + getVarFloat(ai.arg2);

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '+' operator\n");
								}
								break;
							case '*':
								if (arg2Type == INT)
								{
									if (left == 1)
									{
										left = atoi(ai.arg1);
										var->type = INT;
										var->value.intValue = (long)(left * right);
									}
									else if (left == 2)
									{
										left = atof(ai.arg1);
										var->type = FLOAT;
										var->value.floatValue = left * right;
									}
									else
									{
										printf("Error: Unsupported type for first argument\n");
										break;
									}
									addVar(var);
								}
								else if (arg2Type == FLOAT)
								{
									var->type = FLOAT;
									var->value.floatValue = atof(ai.arg1) * getVarFloat(ai.arg2);

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '*' operator\n");
								}
								break;
							case '-':
								if (arg2Type == INT)
								{
									if (left == 1)
									{
										left = atoi(ai.arg1);
										var->type = INT;
										var->value.intValue = (long)(left - right);
									}
									else if (left == 2)
									{
										left = atof(ai.arg1);
										var->type = FLOAT;
										var->value.floatValue = left - right;
									}
									else
									{
										printf("Error: Unsupported type for first argument\n");
										break;
									}
									addVar(var);
								}
								else if (arg2Type == FLOAT)
								{
									var->type = FLOAT;
									var->value.floatValue = atof(ai.arg1) - getVarFloat(ai.arg2);

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '-' operator\n");
								}
								break;
							case '/':
								if (arg2Type == INT)
								{
									if (left == 1)
									{
										left = atoi(ai.arg1);
										var->type = INT;
										var->value.intValue = (long)(left / right);
									}
									else if (left == 2)
									{
										left = atof(ai.arg1);
										var->type = FLOAT;
										var->value.floatValue = left / right;
									}
									else
									{
										printf("Error: Unsupported type for first argument\n");
										break;
									}
									addVar(var);
								}
								else if (arg2Type == FLOAT)
								{
									var->type = FLOAT;
									var->value.floatValue = atof(ai.arg1) / getVarFloat(ai.arg2);

									addVar(var);
								}
								else
								{
									printf("Error: Unsupported types for '/' operator\n");
								}
								break;
							default:
								printf("Unsupported operator '%c' for variable types\n", ai.op);
								break;
							}
						}
					}
					else if (arg1Type == LIST && arg2Type == LIST)
					{
						// Parse index from arg2 (e.g., b[3])
						int index1 = parse_list_index(ai.arg1);
						int index2 = parse_list_index(ai.arg2);
						listNode *node1 = getListNode(ai.arg1, index1);
						listNode *node2 = getListNode(ai.arg2, index2);

						if (node1 != NULL && node2 != NULL)
						{
							// Perform addition if both nodes are numeric
							if ((node1->type == INT || node1->type == FLOAT) && (node2->type == INT || node2->type == FLOAT))
							{
								var->type = FLOAT;
								double val1 = (node1->type == INT) ? node1->value.intValue : node1->value.floatValue;
								double val2 = (node2->type == INT) ? node2->value.intValue : node2->value.floatValue;
								var->value.floatValue = val1 + val2;

								addVar(var);
							}
							else if (node1->type == INT && node2->type == INT)
							{
								var->type = INT;
								var->value.intValue = node1->value.intValue + node2->value.intValue;

								addVar(var);
							}
							else
							{
								printf("Error: Unsupported types for list element addition\n");
							}
						}
						else
						{
							printf("Error: One or both list indices not found\n");
						}
					}
					else if (arg1Type == LIST && arg2Type != UNDEFINED)
					{

						int index1 = parse_list_index(ai.arg1);
						const char *VarName = strtok(ai.arg1, "[");
						listNode *listNode1 = getListNode(VarName, index1);
						// Check if listNode1 is not NULL
						switch (arg2Type)
						{
						case INT: // INT
							if (listNode1->type == FLOAT)
							{
								var->type = FLOAT;
								var->value.floatValue = listNode1->value.floatValue + atoi(ai.arg2);

								addVar(var);
							}
							else if (listNode1->type == INT)
							{
								var->type = INT;
								var->value.intValue = listNode1->value.intValue + atoi(ai.arg2);

								addVar(var);
							}
							else
							{
								printf("Error: Cannot perform arithmetic between list and number\n");
							}
							break;
						case FLOAT: // FLOAT
							var->type = FLOAT;
							if (listNode1->type == INT)
							{
								var->value.floatValue = listNode1->value.intValue + atof(ai.arg2);

								addVar(var);
							}
							else if (listNode1->type == FLOAT)
							{
								var->value.floatValue = listNode1->value.floatValue + atof(ai.arg2);

								addVar(var);
							}
							else
							{
								printf("Error: Cannot perform arithmetic between list and number\n");
								break;
							}
							break;
						default:
							printf("Error: Unsupported type for second argument in list operation\n");
							break;
						}
					}
					else if (arg1Type != UNDEFINED && arg2Type == LIST)
					{
						listNode *listNode2 = getVar(ai.arg2)->next;
						switch (arg2Type)
						{
						case INT: // INT
							if (listNode2->type == FLOAT)
							{
								var->type = FLOAT;
								if (listNode2->type == INT)
								{
									var->value.floatValue = listNode2->value.intValue + atoi(ai.arg1);
								}
								else if (listNode2->type == FLOAT)
								{
									var->value.floatValue = listNode2->value.floatValue + atof(ai.arg1);
								}

								addVar(var);
							}
							else if (listNode2->type == INT)
							{
								var->type = INT;
								var->value.intValue = listNode2->value.intValue + atoi(ai.arg1);

								addVar(var);
							}
							else
							{
								printf("Error: Cannot perform arithmetic between list and number\n");
							}
						case FLOAT: // FLOAT
							var->type = FLOAT;
							if (listNode2->type == INT)
							{
								var->value.floatValue = listNode2->value.intValue + atof(ai.arg1);
							}
							else if (listNode2->type == FLOAT)
							{
								var->value.floatValue = listNode2->value.floatValue + atof(ai.arg1);
							}
							else
							{
								printf("Error: Cannot perform arithmetic between list and number\n");
								break;
							}
							break;
						default:
							printf("Error: Unsupported type for second argument in list operation\n");
							break;
						}

					case PRINT:
					{
						PrintInfo pi;
						parse_print(cmd, &pi);
						if (pi.valid)
						{
							print(pi.var);
						}
						else
						{
							printf("[PRINT] (parse error)\n");
						}
					}
					break;
					case APPEND:
					{
						AppendInfo api;
						parse_append(cmd, &api);
						if (api.valid)
						{

							// Split arg2 by comma and print all items
							char arg2_copy[64];
							strncpy(arg2_copy, api.arg2, sizeof(arg2_copy) - 1);
							arg2_copy[sizeof(arg2_copy) - 1] = '\0';
							char *token = strtok(arg2_copy, ",");

							Variable *listVar = getVar(api.arg1);

							listNode *current;

							if (listVar->type == LIST)
							{

								// printf("List variable '%s' found.\n", api.arg1);
								int index = 0;
								while (token != NULL)
								{

									listNode *newNode = (listNode *)malloc(sizeof(listNode));

									switch (findVarType(token))
									{
									case INT:
										newNode->type = INT;
										newNode->value.intValue = atoi(token);
										break;
									case FLOAT:
										newNode->type = FLOAT;
										newNode->value.floatValue = atof(token);
										break;
									case STRING:
										newNode->type = STRING;
										// Remove quotes from token
										{
											size_t len = strlen(token);
											if (len >= 2 && token[0] == '"' && token[len - 1] == '"')
											{
												strncpy(newNode->value.stringValue, token + 1, len - 2);
												newNode->value.stringValue[len - 2] = '\0';
											}
											else
											{
												strncpy(newNode->value.stringValue, token, sizeof(newNode->value.stringValue) - 1);
												newNode->value.stringValue[sizeof(newNode->value.stringValue) - 1] = '\0';
											}
										}
										break;
									case CHAR:
										newNode->type = CHAR;
										newNode->value.charValue[0] = token[1]; // middle character
										newNode->value.charValue[1] = '\0';
										break;
									case VARIABLE:
										Variable *var = getVar(token);
										if (var->type != LIST)
										{
											printf("Cannot append a variable that is not a list: %s\n", token);
											free(newNode);
										}
										else if (var->type == LIST)
										{

											newNode = copyList(var);
										}
										break;
									default:
										printf("Unrecognized type for list item: %s\n", token);
										free(newNode);
										token = strtok(NULL, ",");
										continue;
									}
									newNode->next = NULL;

									if (listVar->next != NULL) // Makes sure its not the first item
									{
										current = listVar->next;
										index = current->index;
									}
									else
									{ // Initialize first item index
										newNode->index = 0;
										listVar->next = newNode;
										current = listVar->next;

										token = strtok(NULL, ",");
										continue;
									}
									// Increment index for new item
									while (current->next != NULL) // Goes to the end of the list
									{
										current = current->next;
									}
									index = current->index + 1;

									newNode->index = index;
									current->next = newNode;
									token = strtok(NULL, ",");
								}
							}
						}
						else
						{
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
		}
	}
}
