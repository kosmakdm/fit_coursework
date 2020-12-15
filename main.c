#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define RSIZ 10240

#define EDIT_TABLE_TYPE 0
#define ROW_SELECTION_TYPE 1
#define BEGINSWITH_SELECTION_TYPE 2
#define CONTAINS_SELECTION_TYPE 3
#define DATA_PROCESSING_TYPE 4
#define NOT_VALID_TYPE -1

typedef struct Command {
    char *name;
    int number_of_arguments;
    int type;
} command;

typedef struct CommandWithArgs {
    command command;
    char *args[2]; // number of args here is the same as
} command_with_args;

command_with_args empty = {.command = {.type = NOT_VALID_TYPE}};

// Edit table mode commands
const command irow = {.name = "irow", .number_of_arguments = 1, .type = EDIT_TABLE_TYPE}; // irow 10
const command arow = {.name = "arow", .number_of_arguments = 0, .type = EDIT_TABLE_TYPE}; // arow
const command drow = {.name = "drow", .number_of_arguments = 1, .type = EDIT_TABLE_TYPE}; // drow 10
const command drows = {.name = "drows", .number_of_arguments = 2, .type = EDIT_TABLE_TYPE}; // drows 2 4
const command icol = {.name = "icol", .number_of_arguments = 1, .type = EDIT_TABLE_TYPE}; // icol 5
const command acol = {.name = "acol", .number_of_arguments = 0, .type = EDIT_TABLE_TYPE}; // acol
const command dcol = {.name = "dcol", .number_of_arguments = 1, .type = EDIT_TABLE_TYPE}; // dcol 5
const command dcols = {.name = "dcols", .number_of_arguments = 2, .type = EDIT_TABLE_TYPE}; // dcols 4 7

// Data processing mode commands
const command cset = {.name = "cset", .number_of_arguments = 2, .type = DATA_PROCESSING_TYPE}; // cset 2 "changed column"
const command tolower_c = {.name = "tolower", .number_of_arguments = 1, .type = DATA_PROCESSING_TYPE}; // tolower 3
const command toupper_c = {.name = "toupper", .number_of_arguments = 1, .type = DATA_PROCESSING_TYPE}; // toupper 10
const command round_c = {.name = "round", .number_of_arguments = 1, .type = DATA_PROCESSING_TYPE}; // round 3
const command int_c = {.name = "int", .number_of_arguments = 1, .type = DATA_PROCESSING_TYPE}; // int 5
const command copy = {.name = "copy", .number_of_arguments = 2, .type = DATA_PROCESSING_TYPE}; // copy 4 12
const command swap = {.name = "swap", .number_of_arguments = 2, .type = DATA_PROCESSING_TYPE}; // swap 2 5
const command move = {.name = "move", .number_of_arguments = 2, .type = DATA_PROCESSING_TYPE}; // move 4 6

// Row selection mode commands
const command rows = {.name = "rows", .number_of_arguments = 2, .type = ROW_SELECTION_TYPE}; // rows 4 7
const command beginswith = {.name = "beginswith", .number_of_arguments = 2, .type = BEGINSWITH_SELECTION_TYPE}; // beginswith 4 "Beijin"
const command contains = {.name = "contains", .number_of_arguments = 2, .type = CONTAINS_SELECTION_TYPE}; // contains 4 "Beijin"

char *delimiters = " ";

typedef struct operation_result {
    int result_code;
    int type;
    char *message;
} operation_result;

operation_result result(int code, int type, char *message);
operation_result parse_commands(int argc, char **argv, command_with_args all_commands[]);
bool should_process_this_row(const command_with_args *all_commands, int curr_row, int column_count, char row[105][101]);
bool is_delimiter(char i);

int main(int argc, char **argv) {
    command_with_args all_commands[100] = {empty};
    operation_result result = parse_commands(argc, argv, all_commands);

    if (result.result_code < 0) {
        printf("%s", result.message);
        return result.result_code;
    }

    char input[RSIZ] = { 0 };
    int curr_row = 1;
    int number_of_columns = 0;
    while (fgets(input, RSIZ, stdin)) {
        int column_count = 0;
        int i_column = 0;
        char row[105][101] = {0};
        for (int i = 0; i < (int)strlen(input); ++i) {
            if (!is_delimiter(input[i]) && input[i] != '\n' && input[i] != EOF) {
                row[column_count][i_column] = input[i];
                ++i_column;
            }
            else {
                ++column_count;
                i_column = 0;
            }
        }
        // if file doesn't contain EOF token and just interrupts
        if (input[strlen(input) - 1] != '\n' && input[strlen(input) - 1] != EOF) {
            ++column_count;
        }
        if (curr_row == 1) {
            number_of_columns = column_count;
        } else if (column_count != number_of_columns) {
            printf("Row #%d contains %d columns, but should contain %d", curr_row, column_count, number_of_columns);
            return -1;
        }

        if (result.type == DATA_PROCESSING_TYPE) {
            bool on_this_row = should_process_this_row(all_commands, curr_row, column_count, row);
            command_with_args cmd = all_commands[0];
            if (on_this_row && cmd.command.type != NOT_VALID_TYPE) {
                char *command_name = cmd.command.name;
                if (strcmp(command_name, "cset") == 0) {
                    int col = atoi(cmd.args[0]) - 1;
                    if (col < column_count) {
                        strcpy(row[col], cmd.args[1]);
                    }
                } else if (strcmp(command_name, "tolower") == 0) {
                    int col = atoi(cmd.args[0]) - 1;
                    if (col < column_count) {
                        for (int i = 0; row[col][i]; i++) {
                            row[col][i] = tolower(row[col][i]);
                        }
                    }
                } else if (strcmp(command_name, "toupper") == 0) {
                    int col = atoi(cmd.args[0]) - 1;
                    if (col < column_count) {
                        for (int i = 0; row[col][i]; i++) {
                            row[col][i] = toupper(row[col][i]);
                        }
                    }
                } else if (strcmp(command_name, "round") == 0) {
                    int col = atoi(cmd.args[0]) - 1;
                    if (col < column_count) {
                        char *err;
                        double number = strtod(row[col], &err);
                        if (*err != 0 && !isspace((unsigned char) *err)) {
                            fprintf(stderr, "ERROR: expected a number (row %d, column %d)\n", curr_row, col);
                            return EXIT_FAILURE;
                        }
                        int res = number >= 0 ? (int) (number + 0.5) : (int) (number - 0.5);
                        sprintf(row[col], "%d", res);
                    }
                } else if (strcmp(command_name, "int") == 0) {
                    int col = atoi(cmd.args[0]) - 1;
                    if (col < column_count) {
                        char *err;
                        double number = strtod(row[col], &err);
                        if (*err != 0 && !isspace((unsigned char) *err)) {
                            fprintf(stderr, "ERROR: expected a number (row %d, column %d)\n", curr_row, col);
                            return EXIT_FAILURE;
                        }
                        int res = (int)number;
                        sprintf(row[col], "%d", res);
                    }
                } else if (strcmp(command_name, "copy") == 0) {
                    int col_from = atoi(cmd.args[0]) - 1;
                    int col_to = atoi(cmd.args[1]) - 1;
                    if (col_from < column_count && col_to < column_count) {
                        strcpy(row[col_to], row[col_from]);
                    }
                } else if (strcmp(command_name, "swap") == 0) {
                    int col_from = atoi(cmd.args[0]) - 1;
                    int col_to = atoi(cmd.args[1]) - 1;
                    if (col_from < column_count && col_to < column_count) {
                        char temp[101] = { 0 };
                        strcpy(temp, row[col_from]);
                        strcpy(row[col_from], row[col_to]);
                        strcpy(row[col_to], temp);
                    }
                } else if (strcmp(command_name, "move") == 0) {
                    // 1 2 3 4 5 6 -> move 2 5 -> 1 3 4 2 5 6
                    // 1 2 3 4 5 6 -> move 5 2 -> 1 5 2 3 4 6
                    int col_before = atoi(cmd.args[0]) - 1;
                    int col_to = atoi(cmd.args[1]) - 1;
                    if (col_before < column_count && col_to < column_count) {
                        char temp[101] = { 0 };
                        strcpy(temp, row[col_before]);
                        if (col_before < col_to) {
                            for (int col = col_before; col < col_to - 1; col++) {
                                strcpy(row[col], row[col + 1]);
                            }
                        } else {
                            for (int col = col_before; col > col_to; col--) {
                                strcpy(row[col], row[col - 1]);
                            }
                        }
                        strcpy(row[col_to], temp);
                    }

                }

            }
            for (int i = 0; i < column_count; i++) {
                printf("%s", row[i]);
                if (i != column_count - 1) {
                    printf("%c", delimiters[0]);
                }
            }
            printf("%s", "\n");
        }

        curr_row++;
    }

}

bool should_process_this_row(const command_with_args *all_commands, int curr_row, int column_count, char row[105][101]) {
    bool on_this_row = true;
    if (all_commands[1].command.type == ROW_SELECTION_TYPE) {
        on_this_row = curr_row >= atoi(all_commands[1].args[0]) && curr_row <= atoi(all_commands[1].args[1]);
    } else if (all_commands[1].command.type == BEGINSWITH_SELECTION_TYPE) {
        int col = atoi(all_commands[1].args[0]) - 1;
        on_this_row = col <= column_count && strncmp(all_commands[1].args[1], row[col], strlen(all_commands[1].args[1])) == 0;
    } else if (all_commands[1].command.type == CONTAINS_SELECTION_TYPE) {
        int col = atoi(all_commands[1].args[0]) - 1;
        on_this_row = col <= column_count && strstr(row[col], all_commands[1].args[1]) != NULL;
    }
    return on_this_row;
}

operation_result parse_commands(int argc, char **argv, command_with_args all_commands[]) {
    int index = 1;

    // parse delimiter
    if (strcmp(argv[index], "-d") == 0) {
        if (argc < 3) {
            return result(-1, NOT_VALID_TYPE, "-d argument doesn't contain delimiter.");
        }
        delimiters = argv[index + 1];
        index += 2;
    }

    if (index == argc) {
        return result(0, NOT_VALID_TYPE, "Ok");
    }

    int type = NOT_VALID_TYPE;

    command edit_commands[] = {irow, arow, drow, drows, icol, acol, dcol, dcols};
    command data_processing[] = {cset, tolower_c, toupper_c, round_c, int_c, swap, move, copy};
    command row_selection[] = {rows, beginswith, contains};

    char *cmd = argv[index];
    for (int i = 0; i < sizeof(row_selection); i++) {
        command curr_cmd = row_selection[i];
        if (strcmp(curr_cmd.name, cmd) == 0) {
            index++;
            type = DATA_PROCESSING_TYPE;

            if (argc < index + curr_cmd.number_of_arguments) {
                return result(-1, NOT_VALID_TYPE, "Too few arguments");
            }
            char *args[2] = {0};
            for (int j = 0; j < curr_cmd.number_of_arguments; j++) {
                args[j] = argv[index];
                index++;
            }
            char **test;
            test = args;
            command_with_args row_cmd = {.command = curr_cmd, .args = {args[0], args[1]}};
            all_commands[1] = row_cmd;
            break;
        }
    }

    if (index == argc) {
        return result(0, NOT_VALID_TYPE, "Ok");
    }

    cmd = argv[index];
    for (int i = 0; i < sizeof(data_processing); i++) {
        command curr_cmd = data_processing[i];
        if (strcmp(curr_cmd.name, cmd) == 0) {
            index++;
            type = DATA_PROCESSING_TYPE;

            if (argc < index + curr_cmd.number_of_arguments) {
                return result(-1, NOT_VALID_TYPE, "Too few arguments");
            }
            char* args[2] = {0};
            for (int j = 0; j < curr_cmd.number_of_arguments; j++) {
                args[j] = argv[index];
                index++;
            }
            command_with_args row_cmd = {.command = curr_cmd, .args = {args[0], args[1]}};
            all_commands[0] = row_cmd;
            break;
        }
    }

    if (type != NOT_VALID_TYPE) {

    }

    return result(0, type, "OK");
}

bool is_delimiter(char i) {
    for (int j = 0; j < strlen(delimiters); j++) {
        if (delimiters[j] == i) {
            return true;
        }
    }
    return false;
}

operation_result result(int code, int type, char *msg) {
    operation_result result;
    result.result_code = code;
    result.message = msg;
    result.type = type;
    return result;
}