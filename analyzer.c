//gcc ast_jhg.c cJSON.c -o ast_jhg
// ast_jhg.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

//파일 읽어오는 함수
char* read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("파일 열기 실패");
        return NULL;
    }

    //파일 크기 저장
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);

    return buffer;
}

//if문 발견 될때마다 *count +1 
void count_if_statements(cJSON *node, int *count) {
    if (!node) return;

    cJSON *current = NULL;
    cJSON_ArrayForEach(current, node) {
        cJSON *nodetype = cJSON_GetObjectItem(current, "_nodetype");
        if (nodetype && strcmp(nodetype->valuestring, "If") == 0) {
            (*count)++;
        }

        //재귀
        count_if_statements(current, count);
    }
}

//타입 출력
void print_type(cJSON *type_node) {
    if (!type_node) return;

    cJSON *type_type = cJSON_GetObjectItem(type_node, "type");
    cJSON *nodetype = cJSON_GetObjectItem(type_node, "_nodetype");

    if (nodetype && strcmp(nodetype->valuestring, "PtrDecl") == 0) {
        print_type(type_type);
        printf(" *");
    } else {
        cJSON *names = cJSON_GetObjectItem(type_node, "names");
        if (names && cJSON_IsArray(names)) {
            cJSON *first = cJSON_GetArrayItem(names, 0);
            if (first) printf("%s", first->valuestring);
        } else {
            cJSON *name = cJSON_GetObjectItem(type_node, "name");
            if (name) printf("%s", name->valuestring);
            else if (type_type) print_type(type_type);
        }
    }
}

//함수이름, 리턴타입, 파라미터 타입, if문 개수 출력
void process_functions(cJSON *ext_array) {
    int func_count = 0;
    int total_if_count = 0;

    cJSON *element = NULL;
    cJSON_ArrayForEach(element, ext_array) {
        cJSON *nodetype = cJSON_GetObjectItem(element, "_nodetype");
        if (nodetype && strcmp(nodetype->valuestring, "FuncDef") == 0) {
            func_count++;
            cJSON *decl = cJSON_GetObjectItem(element, "decl");
            cJSON *decl_type = cJSON_GetObjectItem(decl, "type");

            printf("%d번째 리턴 타입 : ", func_count);
            print_type(decl_type);
            printf("\n");

            cJSON *name = cJSON_GetObjectItem(decl, "name");
            printf("%d번째 함수 이름 : %s\n", func_count, name->valuestring);

            cJSON *args = cJSON_GetObjectItem(decl_type, "args");
            if (!args) {
                printf("%d번째 함수 파라미터 : X\n", func_count);
            } else {
                cJSON *params = cJSON_GetObjectItem(args, "params");
                int param_index = 1;
                cJSON *param = NULL;
                cJSON_ArrayForEach(param, params) {
                    cJSON *param_name = cJSON_GetObjectItem(param, "name");
                    printf("%d번째 함수 - %d번째 파라미터 변수 명 : %s, 타입 : ", func_count, param_index++, param_name->valuestring);
                    print_type(param);
                    printf("\n");
                }
            }

            cJSON *body = cJSON_GetObjectItem(element, "body");
            int if_count = 0;
            count_if_statements(body, &if_count);
            printf("%d번째 함수 if 개수 : %d\n\n", func_count, if_count);

            total_if_count += if_count;
        }
    }

    printf("모든 if의 개수 : %d\n", total_if_count);
}

int main() {
    char *json_text = read_file("ast.json");
    if (!json_text) return 1;

    cJSON *root = cJSON_Parse(json_text);
    if (!root) {
        printf("JSON 파싱 실패\n");
        free(json_text);
        return 1;
    }

    cJSON *ext = cJSON_GetObjectItem(root, "ext");
    if (!ext || !cJSON_IsArray(ext)) {
        printf("ext 배열 없음\n");
        cJSON_Delete(root);
        free(json_text);
        return 1;
    }

    //printf("모든 함수 개수: %d\n\n", cJSON_GetArraySize(ext));
    int actual_func_count = 0;

    cJSON *element = NULL;
    cJSON_ArrayForEach(element, ext) {
        cJSON *nodetype = cJSON_GetObjectItem(element, "_nodetype");
        if (nodetype && strcmp(nodetype->valuestring, "FuncDef") == 0) {
            actual_func_count++;
        }
    }

    printf("모든 함수 개수: %d\n\n", actual_func_count);
    
    process_functions(ext);

    cJSON_Delete(root);
    free(json_text);
    return 0;
}
