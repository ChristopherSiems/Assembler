#include "map.h"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct instruct instruct;
struct instruct {
  char *str;
  instruct *next;
};

char *bin(int cursor) {
  /*
   * transforms the given integer to a 15-bit binary string
   *
   * cursor: the integer to convert
   *
   * returns: cursor as a binary string
   */

  // create address
  char *addr = malloc(16);
  addr[15] = '\0';

  // define address
  for (int i = 14; i >= 0; i--) {
    addr[i] = (cursor & 1) ? '1' : '0';
    cursor >>= 1;
  }

  return addr;
}

int main(int argc, char **argv) {

  // define symbol table
  map symbols = createMap(10000);
  insertKey(symbols, "R0", "000000000000000");
  insertKey(symbols, "R1", "000000000000001");
  insertKey(symbols, "R2", "000000000000010");
  insertKey(symbols, "R3", "000000000000011");
  insertKey(symbols, "R4", "000000000000100");
  insertKey(symbols, "R5", "000000000000101");
  insertKey(symbols, "R6", "000000000000110");
  insertKey(symbols, "R7", "000000000000111");
  insertKey(symbols, "R8", "000000000001000");
  insertKey(symbols, "R9", "000000000001001");
  insertKey(symbols, "R10", "000000000001010");
  insertKey(symbols, "R11", "000000000001011");
  insertKey(symbols, "R12", "000000000001100");
  insertKey(symbols, "R13", "000000000001101");
  insertKey(symbols, "R14", "000000000001110");
  insertKey(symbols, "R15", "000000000001111");
  insertKey(symbols, "SCREEN", "100000000000000");
  insertKey(symbols, "KBD", "110000000000000");
  insertKey(symbols, "SP", "000000000000000");
  insertKey(symbols, "LCL", "000000000000001");
  insertKey(symbols, "ARG", "000000000000010");
  insertKey(symbols, "THIS", "000000000000011");
  insertKey(symbols, "THAT", "000000000000100");

  // define dest mappings
  map map_dest = createMap(8);
  insertKey(map_dest, "M", "001");
  insertKey(map_dest, "D", "010");
  insertKey(map_dest, "DM", "011");
  insertKey(map_dest, "MD", "011");
  insertKey(map_dest, "A", "100");
  insertKey(map_dest, "AM", "101");
  insertKey(map_dest, "AD", "110");
  insertKey(map_dest, "ADM", "111");

  // define comp mappings
  map map_comp = createMap(28);
  insertKey(map_comp, "0", "0101010");
  insertKey(map_comp, "1", "0111111");
  insertKey(map_comp, "-1", "0111010");
  insertKey(map_comp, "D", "0001100");
  insertKey(map_comp, "A", "0110000");
  insertKey(map_comp, "!D", "0001101");
  insertKey(map_comp, "!A", "0110011");
  insertKey(map_comp, "-D", "0001111");
  insertKey(map_comp, "-A", "0001111");
  insertKey(map_comp, "D+1", "0011111");
  insertKey(map_comp, "A+1", "0110111");
  insertKey(map_comp, "D-1", "0001110");
  insertKey(map_comp, "A-1", "0110010");
  insertKey(map_comp, "D+A", "0000010");
  insertKey(map_comp, "D-A", "0010011");
  insertKey(map_comp, "A-D", "0000111");
  insertKey(map_comp, "D&A", "0000000");
  insertKey(map_comp, "D|A", "0010101");
  insertKey(map_comp, "M", "1110000");
  insertKey(map_comp, "!M", "1110001");
  insertKey(map_comp, "-M", "1110011");
  insertKey(map_comp, "M+1", "1110111");
  insertKey(map_comp, "M-1", "1110010");
  insertKey(map_comp, "D+M", "1000010");
  insertKey(map_comp, "D-M", "1010011");
  insertKey(map_comp, "M-D", "1000111");
  insertKey(map_comp, "D&M", "1000000");
  insertKey(map_comp, "D|M", "1010101");

  // define jump mapping
  map map_jump = createMap(7);
  insertKey(map_jump, "JGT", "001");
  insertKey(map_jump, "JEQ", "010");
  insertKey(map_jump, "JGE", "011");
  insertKey(map_jump, "JLT", "100");
  insertKey(map_jump, "JNE", "101");
  insertKey(map_jump, "JLE", "110");
  insertKey(map_jump, "JMP", "111");

  // variables for input file
  char *line = NULL;
  size_t len = 0;
  char *file_in = argv[1];
  FILE *input = fopen(file_in, "r");
  instruct *instruct_head = NULL;
  instruct *instruct_curr;
  unsigned short int rom_cursor = -1;
  unsigned short int mem_cursor = 16;

  // symbol table pass
  while (getline(&line, &len, input) != -1) {

    // find start of the content of the line
    size_t start = 0;
    while (isspace(line[start]))
      start++;
    if (line[start] == '\0')
      continue;

    // find end of the content of the line
    size_t end = start;
    while (!isspace(line[end]) && line[end] != '/' && line[end] != '\0')
      end++;
    end--;
    if (start >= end)
      continue;

    // moving content to front of string
    size_t term = end - start + 1;
    memmove(line, line + start, term);
    line[term] = '\0';
    if (line[0] == '\0')
      continue;

    // line length variables
    size_t line_len = strlen(line);
    size_t line_str_len = line_len - 1;

    // checking for labels
    if (line[0] == '(') {

      // finding label symbol
      char *label = malloc(line_str_len);
      strncpy(label, line + 1, line_str_len - 1);
      label[line_str_len] = '\0';

      // assigning label address
      char *rom_cursor_bin = bin(rom_cursor + 1);

      // adding new entry
      if (containsKey(symbols, label) == -1)
        insertKey(symbols, label, rom_cursor_bin);
      else {

        // finding existing entry
        for (unsigned short int i = 0; i < symbols->mapSize; i++) {
          stringpair pair = symbols->pairs[i];
          if (pair.key == label) {

            // setting new value
            char *val = pair.value;
            free(val);
            val = rom_cursor_bin;
            break;
          }
        }
      }

      // next line
      continue;
    }

    // next ROM address
    rom_cursor++;

    // A-instructions
    if (line[0] == '@') {

      // finding number or variable
      char *tail = malloc(line_len);
      strncpy(tail, line + 1, line_str_len);
      tail[line_str_len] = '\0';

      // checking if value is already assigned
      if (containsKey(symbols, tail) == -1) {

        // determining if value is number or variable
        bool num = true;
        for (unsigned short int i = 0; i < line_str_len; i++)
          if (!isdigit(tail[i])) {
            num = false;
            break;
          }

        // assigning address
        char *addr;
        if (num)
          addr = bin(atoi(tail));
        else {
          addr = bin(mem_cursor);
          mem_cursor++;
        }
        insertKey(symbols, tail, addr);
      } else

        // freeing tail if unused
        free(tail);
    }

    // record instruction
    instruct *new = (instruct *)malloc(sizeof(instruct));
    new->str = malloc(line_len + 1);
    strcpy(new->str, line);
    new->next = NULL;

    // checking if this is first instruction or not
    if (instruct_head == NULL) {
      printf("head\n");
      printf("%s\n", line);
      instruct_head = new;
      instruct_curr = new;
    } else {
      printf("%s\n", line);
      instruct_curr->next = new;
      instruct_curr = new;
    }
  }

  // free line
  free(line);

  // close input file
  fclose(input);

  // defining output file
  size_t file_in_len = strlen(file_in);
  char *file_out = malloc(file_in_len + 2);
  size_t len_pruned = file_in_len - 3;
  strncpy(file_out, file_in, len_pruned);
  file_out[len_pruned] = '\0';
  strcat(file_out, "hack");
  FILE *output = fopen(file_out, "w");
  free(file_out);

  // parsing pass
  instruct_curr = instruct_head;
  char *instruct_str = NULL;
  while (instruct_curr != NULL) {

    // the current instruction
    instruct_str = instruct_curr->str;
    printf("%s", instruct_str);

    // A-instructions
    if (instruct_str[0] == '@') {
      char *a_val = instruct_str + 1;
      fprintf(output, "0%s\n", lookupKey(symbols, a_val));

      // next instruction
      instruct_curr = instruct_curr->next;
      continue;
    }

    // C-instructions
    char *eq_pos = strchr(instruct_str, '=');
    char *sc_pos = strchr(instruct_str, ';');
    char *dest_bin = "000";
    char *jump_bin = "000";
    char *comp_start;
    char *comp_end;
    char *comp_bin;

    // dest
    if (eq_pos != NULL) {

      // dest string
      size_t diff = eq_pos - instruct_str;
      char *asm_str = malloc(diff + 1);
      strncpy(asm_str, instruct_str, diff);
      asm_str[diff] = '\0';

      // dest binary
      dest_bin = lookupKey(map_dest, asm_str);
      free(asm_str);
      comp_start = eq_pos + 1;
    } else
      comp_start = instruct_str;

    // jump
    if (sc_pos != NULL) {

      // jump string
      size_t diff = instruct_str + strlen(instruct_str) - sc_pos - 1;
      char *asm_str = malloc(diff + 1);
      strncpy(asm_str, sc_pos + 1, diff);
      asm_str[diff] = '\0';

      // jump binary
      jump_bin = lookupKey(map_jump, asm_str);
      free(asm_str);
      comp_end = sc_pos;
    } else
      comp_end = instruct_str + strlen(instruct_str) + 1;

    // comp string
    size_t diff = comp_end - comp_start;
    char *asm_str = malloc(diff + 1);
    strncpy(asm_str, comp_start, diff);
    asm_str[diff] = '\0';

    // comp binary
    comp_bin = lookupKey(map_comp, asm_str);
    free(asm_str);

    // write C-instruction
    fprintf(output, "111%s%s%s\n", comp_bin, dest_bin, jump_bin);

    // next instruction
    instruct_curr = instruct_curr->next;
  }

  // free pointers
  free(instruct_str);

  // free maps
  freeMap(map_dest);
  freeMap(map_comp);
  freeMap(map_jump);
  freeMap(symbols);

  // free instruction list
  instruct_curr = instruct_head;
  while (instruct_curr != NULL) {
    instruct *temp = instruct_curr;
    instruct_curr = instruct_curr->next;
    free(temp);
  }

  // close output
  fclose(output);

  return 0;
}
