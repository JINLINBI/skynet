#include "skynet.h"
#include "rbtree.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <locale.h>
#include <wchar.h>
#include <cJSON.h>
#include <unistd.h>

struct excel {
	int filescount;
	cJSON* excel_root;
	struct rb_cjson_root** rb_cjson_files_root;
};

struct excel *
excel_create(void) {
	struct excel * inst = skynet_malloc(sizeof(*inst));
	inst->filescount = 0;

	return inst;
}

void
excel_release(struct excel * inst) {
	for (int i = 0; i < inst->filescount; i++) {
		skynet_free(inst->rb_cjson_files_root[i]);
	}
	skynet_free(inst->rb_cjson_files_root);
	skynet_free(inst);
}

static int
excel_cb(struct skynet_context * context, void *ud, int type, int session, uint32_t source, const void * msg, size_t sz) {
	//struct excel * inst = ud;
	switch (type) {
		case PTYPE_TEXT:
			printf("excel service get msg: %s\n", (char*)msg);
			break;
	}

	return 0;
}

uint32_t create_rb_index_by_cjson(struct rb_root * root, cJSON * cjson_data) {
	uint32_t line_count = cJSON_GetArraySize(cjson_data);
	for (int i = 0; i < line_count; i++) {
		cJSON * cjson_line = cJSON_GetArrayItem(cjson_data, i);
		rb_cjson_line * line_node = skynet_malloc(sizeof(rb_cjson_line));
		line_node->index = cJSON_GetArrayItem(cjson_line, 0)->valueint;
		line_node->cjson_line_item = cjson_line;

		rb_insert_cjson_line(root, line_node->index, &line_node->rb_node);
	}
	

	return 0;
}

/*
 * 打印"红黑树"
 */
static void print_rbtree(struct rb_node *tree, uint32_t key, int direction, uint32_t parent)
{
    if (tree != NULL)
    {   
        if (direction == 0)    // tree是根节点
            printf("%2d(B) is root\n", key);
        else                // tree是分支节点
            printf("%2d(%s) is %2d's %6s child\n", key, rb_is_black(tree)? "B": "R", parent, direction == 1? "right": "left");

        if (tree->rb_left)
            print_rbtree(tree->rb_left, rb_entry(tree->rb_left, struct rb_cjson_line, rb_node)->index, -1, key);
        if (tree->rb_right)
            print_rbtree(tree->rb_right,rb_entry(tree->rb_right, struct rb_cjson_line, rb_node)->index,  1, key); 
    }   
}

void my_print(struct rb_root *root)
{
    if (root != NULL && root->rb_node != NULL)
        print_rbtree(root->rb_node, rb_entry(root->rb_node, struct rb_cjson_line, rb_node)->index, 0, -1); 
}


struct rb_cjson_line * excel_find_cb(struct excel* inst, const char * name, uint32_t index) {
	for (int i = 0; i < inst->filescount; i++) {
		rb_cjson_root * root = inst->rb_cjson_files_root[i];
		if (strcmp(root->name, name) == 0) {
			return rb_search_cjson_line(&root->rb_root, index);
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////

unsigned long get_file_size(const char *path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	if (stat(path, &statbuff) < 0){
		return filesize;
	}
	else{
		filesize = statbuff.st_size;
	}
	return filesize;
}


void* read_file(char* filename){
	int length = get_file_size(filename);
	if (length <= 0){
		return NULL;
	}

	int file = open(filename, O_RDONLY);
	char* data = skynet_malloc(length);

	memset(data, 0, length + 1);

	int ret = read(file, data, length);
	if (ret < 0){
		return NULL;
	}

	return (void*)data;
}


// 单个配置文件解析，返回解析的cjson结构信息
cJSON* parse_excel(char * excel, cJSON * conf, cJSON * last){
	if (conf == NULL || cJSON_IsInvalid(conf) || excel == NULL){
		return NULL;
	}

	cJSON * ret = last;
	if (ret == NULL)
		ret = cJSON_CreateArray();

	FILE* fp;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	fp = fopen(excel, "r");
	if (fp == NULL)
		return ret;

	// 获取excel文件共有多少列
	int col_count = cJSON_GetArraySize(conf);
	
	// 开始解析excel表格
	read = getline(&line, &len, fp);	// feed first line
	while((read = getline(&line, &len, fp)) != -1){
		cJSON * line_item = cJSON_CreateArray();
		cJSON * item;
		char *type;
		int col_index = 0;
		char * token = strtok(line, "\t");
		while (token != NULL){
			type = cJSON_GetObjectItem(cJSON_GetArrayItem(conf, col_index), "type")->valuestring;
			if (!strcmp(type, "string")){
				item = cJSON_CreateString(token);
			}
			else if (!strcmp(type, "number")){
				item = cJSON_CreateNumber(strtoll(token, NULL, 10));
			}
			else if (!strcmp(type, "float")){
				item = cJSON_CreateNumber(strtof(token, NULL));
			}
			else if (!strcmp(type, "int[]")){
				item = cJSON_CreateArray();
				
				char * tmp = skynet_strdup(token);
				char * t;
				for (t = strsep(&tmp, "*"); t != NULL; t = strsep(&tmp, "*")){
					// while (t != NULL){
					cJSON_AddItemToArray(item, cJSON_CreateNumber(strtoll(t, NULL, 10)));
				}
				skynet_free(tmp);
			}
			else{
				item = cJSON_CreateBool(0);
			}

			cJSON_AddItemToArray(line_item, item);

			token = strtok(NULL, "\t");
			col_index++;
		}

		if (cJSON_IsInvalid(item)){
			printf("item is invalid.");
		}
		
		if (col_index != col_count){
			printf("excel file line: %d col count(%d) dont't match conf's!\n", cJSON_GetArraySize(line_item), cJSON_GetArraySize(ret) + 1);
			continue;
		}

		cJSON_AddItemToArray(ret, line_item);
	}

	if (line)
		free(line);

	return ret;
}


int
excel_init(struct excel * inst, struct skynet_context *ctx, const char * excel_path) {
	struct dirent* dirt;
	DIR * dir;
	inst->excel_root = cJSON_CreateObject();

	skynet_command(ctx, "REG", NULL);
	if (excel_path) {
		char files_dir[256] = {0};
		sprintf(files_dir, "%s/json", excel_path);

		dir = opendir(files_dir);		// 打开json文件夹
		cJSON* json_files = cJSON_CreateArray();
		if (dir != NULL){
			while ((dirt = readdir(dir)) != NULL){
				if (strcmp(dirt->d_name, ".") == 0 || strcmp(dirt->d_name, "..") == 0 || strcmp(dirt->d_name, "json") == 0)
					continue;
				char temp[256];
				sscanf(dirt->d_name, "%[^.]", temp);
				cJSON_AddItemToArray(json_files, cJSON_CreateString(temp));
			}
		}

		// 广度优先遍历文件夹,开始解析json文件
		cJSON * json_file = NULL;
		char excel_path_name[128];
		inst->filescount = cJSON_GetArraySize(json_files);


		// 红黑树索引文件数组内存
		inst->rb_cjson_files_root = skynet_malloc(sizeof(struct rb_cjson_root*) * inst->filescount);
		memset(inst->rb_cjson_files_root, 0, sizeof(struct rb_cjson_root*) * inst->filescount);
		for (int i = 0; i < inst->filescount; i++){
			cJSON* array_file_name = cJSON_GetArrayItem(json_files, i);
			sprintf(files_dir, "%s/json/%s.json", excel_path, array_file_name->valuestring);
			char* json_file_data = (char*)read_file(files_dir);
			json_file = cJSON_Parse(json_file_data);
			skynet_free(json_file_data);

			if (cJSON_IsNull(json_file) || cJSON_IsInvalid(json_file)){
				printf("\e[0;31m %s file might has errors.\e[0m\n", files_dir);
				continue;
			}
			
			cJSON * files = cJSON_GetObjectItem(json_file, "files");
			cJSON * fields = cJSON_GetObjectItem(json_file, "fields");

			// 遍历json规则对应下的所有分块excel文件
			int file_count = cJSON_GetArraySize(files);
			char* excel_filename;
			cJSON* ret = NULL;
			for (int i = 0; i < file_count; i++){
				excel_filename = cJSON_GetArrayItem(files, i)->valuestring;
				if (excel_filename == NULL){
					printf("parse json(%s) files(%s) failed...", array_file_name->valuestring, excel_filename);
					continue;
				}
				snprintf(excel_path_name, 256, "%s/%s.txt", excel_path, excel_filename);

				ret = parse_excel(excel_path_name, fields, ret);
			}

			cJSON* file_item = cJSON_CreateObject();
			cJSON_AddItemToObject(inst->excel_root, array_file_name->valuestring, file_item);
			cJSON_AddItemReferenceToObject(file_item, "fields", fields);
			cJSON_AddItemReferenceToObject(file_item, "data", ret);
			
			cJSON_DetachItemFromObject(json_file, "fields");
			cJSON_Delete(json_file);

			inst->rb_cjson_files_root[i] = skynet_malloc(sizeof(struct rb_cjson_root));
			memset(inst->rb_cjson_files_root[i], 0, sizeof(struct rb_cjson_root));
			inst->rb_cjson_files_root[i]->name = skynet_strdup(array_file_name->valuestring);
			create_rb_index_by_cjson(&inst->rb_cjson_files_root[i]->rb_root, ret);
		}

		closedir(dir);
		cJSON_Delete(json_files);
	}

	printf("%s", cJSON_Print(inst->excel_root));
	skynet_callback(ctx, inst, excel_cb);

	return 0;
}

