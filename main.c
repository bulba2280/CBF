#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *FileContent = NULL;

// проверка файла (.bf)
int CheckExtension(const char *name) {
    size_t len = strlen(name);
    if (len < 3 || strcmp(name + len - 3, ".bf") != 0) {
        fprintf(stderr, "error>> File must have .bf extension\n");
        return 1;
    }
    return 0;
}

// чтение файла
int ReadFile(const char *name) {
    FILE *file = fopen(name, "r");
    if (!file) {
        fprintf(stderr, "error>> Can't open file: %s\n", name);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size < 0) {
        fprintf(stderr, "error>> Can't determine file size\n");
        fclose(file);
        return 1;
    }

    FileContent = malloc(size + 1);
    fread(FileContent, 1, size, file);
    FileContent[size] = '\0';
    fclose(file);

    return 0;
}

// Проверка синтексав
int CheckSyntax() {
    int depth = 0;
    for (size_t i = 0; i < strlen(FileContent); i++) {
        if (FileContent[i] == '[') depth++;
        if (FileContent[i] == ']') depth--;
        
        if (depth < 0) {
            fprintf(stderr, "error>> Syntax error: unmatched ']' at position %zu\n", i);
            return 1;
        }
    }
    if (depth != 0) {
        fprintf(stderr, "error>> Syntax error: unmatched '['\n");
        return 1;
    }
    return 0;
}

// 4. BF в C-lang
int TranspileToC(const char *out_filename) {
    FILE *out = fopen(out_filename, "w");
    if (!out) {
        fprintf(stderr, "error>> Can't create output file: %s\n", out_filename);
        return 1;
    }

    // Начало файла
    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "#include <string.h>\n\n");
    fprintf(out, "int main() {\n");
    fprintf(out, "    char arr[30000];\n");
    fprintf(out, "    memset(arr, 0, sizeof(arr));\n");
    fprintf(out, "    int i = 0;\n\n");

    // перевод символов (bf в C-lang)
    for (size_t j = 0; j < strlen(FileContent); j++) {
        switch (FileContent[j]) {
            case '>': fprintf(out, "    i++;\n"); break;
            case '<': fprintf(out, "    i--;\n"); break;
            case '+': fprintf(out, "    arr[i]++;\n"); break;
            case '-': fprintf(out, "    arr[i]--;\n"); break;
            case '.': fprintf(out, "    putchar(arr[i]);\n"); break;
            case ',': fprintf(out, "    arr[i] = getchar();\n"); break;
            case '[': fprintf(out, "    while(arr[i]) {\n"); break;
            case ']': fprintf(out, "    }\n"); break;
            default:  break; // пропускаем другие буквы
        }
    }

    // Закрываем main
    fprintf(out, "\n    return 0;\n}\n");
    fclose(out);
    
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s FILE.bf\n", argv[0]);
        return 1;
    }

    printf("[1] Checking file: %s\n", argv[1]);
    if (CheckExtension(argv[1]) != 0) return 1;
    if (ReadFile(argv[1])       != 0) return 1;
    
    printf("[2] Checking syntax...\n");
    if (CheckSyntax() != 0) {
        free(FileContent);
        return 1;
    }

    const char *temp_c_file = "temp_transpiled.c";
    const char *temp_bin_file = "temp_transpiled_bin";

    printf("[3] Transpiling to C code...\n");
    if (TranspileToC(temp_c_file) != 0) {
        free(FileContent);
        return 1;
    }

    printf("[4] Compiling with GCC...\n");
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd), "gcc %s -o %s", temp_c_file, temp_bin_file);
    
    int compile_status = system(compile_cmd);
    if (compile_status != 0) {
        fprintf(stderr, "error>> Compilation failed!\n");
        remove(temp_c_file);
        free(FileContent);
        return 1;
    }

    printf("[5] Running program...\n\n");
    char run_cmd[256];
    snprintf(run_cmd, sizeof(run_cmd), "./%s", temp_bin_file);
    system(run_cmd);

    printf("\n[6] Cleaning up...\n");

    remove(temp_c_file);
    remove(temp_bin_file);
    
    free(FileContent);
    printf("Done :D\n");
    return 0;
}