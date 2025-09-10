#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_CMD_LEN 100

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
	char arg1[64]; // list name
	char arg2[64]; // item to append
	int valid;
} AppendInfo;


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
	const char *plus = strchr(info->rhs, '+');
	if (plus) {
		size_t a_len = (size_t)(plus - info->rhs);
		if (a_len == 0 || a_len >= sizeof(info->arg1)) return;
		strncpy(info->arg1, info->rhs, a_len);
		info->arg1[a_len] = '\0';
		const char *second = plus + 1;
		if (*second == '\0') return;
		strncpy(info->arg2, second, sizeof(info->arg2) - 1);
		info->arg2[sizeof(info->arg2) - 1] = '\0';
		info->op = '+';
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
						if (ai.arg_count == 1) {
							printf("ARGS: '%s'\n", ai.arg1);
						} else if (ai.arg_count == 2) {
							printf("ARGS: '%s','%s' OP='%c'\n", ai.arg1, ai.arg2, ai.op);
						} else {
							printf("(parse error)\n");
						}
					} else {
						printf("[ASSIGNMENT] (failed to parse details)\n");
					}
				}
				break;
			case PRINT:
				{
					PrintInfo pi; 
					parse_print(cmd, &pi);
					if (pi.valid) {
						printf("[PRINT] VAR='%s'\n", pi.var);
					} else {
						printf("[PRINT] (parse error)\n");
					}
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
	return 0;
}