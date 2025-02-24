#include "map.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {

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

  // defining output file
  size_t file_in_len = strlen(file_in);
  char file_out[file_in_len + 2];
  strncpy(file_out, file_in, file_in_len - 3);
  strcat(file_out, "hack");
  FILE *output = fopen(file_out, "w");

  // working with lines
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

    // A-instructions
    if (line[0] == '@') {
      fprintf(output, "%.16b\n", atoi(line + 1));
      continue;
    }

    // C-instructions
    char *eq_pos = strchr(line, '=');
    char *sc_pos = strchr(line, ';');
    char *dest_bin = "000";
    char *jump_bin = "000";
    char *comp_start;
    char *comp_end;
    char *comp_bin;

    // dest
    if (eq_pos != NULL) {
      size_t diff = eq_pos - line;
      char *asm_str = malloc(diff + 1);
      strncpy(asm_str, line, diff);
      asm_str[diff] = '\0';
      dest_bin = lookupKey(map_dest, asm_str);
      free(asm_str);
      comp_start = eq_pos + 1;
    } else
      comp_start = line;

    // jump
    if (sc_pos != NULL) {
      size_t diff = line + strlen(line) - sc_pos - 1;
      char *asm_str = malloc(diff + 1);
      strncpy(asm_str, sc_pos + 1, diff);
      asm_str[diff] = '\0';
      jump_bin = lookupKey(map_jump, asm_str);
      free(asm_str);
      comp_end = sc_pos;
    } else
      comp_end = line + strlen(line) + 1;

    // comp
    size_t diff = comp_end - comp_start;
    char *asm_str = malloc(diff + 1);
    strncpy(asm_str, comp_start, diff);
    asm_str[diff] = '\0';
    comp_bin = lookupKey(map_comp, asm_str);
    free(asm_str);

    // write C-instruction
    fprintf(output, "111%s%s%s\n", comp_bin, dest_bin, jump_bin);
  }

  // free
  free(line);
  freeMap(map_dest);
  freeMap(map_comp);
  freeMap(map_jump);

  return 0;
}
