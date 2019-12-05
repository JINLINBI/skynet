#include "skynet.h"

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

struct excel {
	FILE * handle;
	char * filename;
	int close;
	cJSON* excel_root;
};

struct excel *
excel_create(void) {
	struct excel * inst = skynet_malloc(sizeof(*inst));
	inst->handle = NULL;
	inst->close = 0;
	inst->filename = NULL;

	return inst;
}

void
excel_release(struct excel * inst) {
	if (inst->close) {
		fclose(inst->handle);
	}
	skynet_free(inst->filename);
	skynet_free(inst);
}

static int
excel_cb(struct skynet_context * context, void *ud, int type, int session, uint32_t source, const void * msg, size_t sz) {
	struct excel * inst = ud;
	switch (type) {
	case PTYPE_SYSTEM:
		if (inst->filename) {
			inst->handle = freopen(inst->filename, "a", inst->handle);
		}
		break;
	case PTYPE_TEXT:
		fprintf(inst->handle, "[:%08x] ",source);
		fwrite(msg, sz , 1, inst->handle);
		fprintf(inst->handle, "\n");
		fflush(inst->handle);
		break;
	}

	return 0;
}

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
		printf("file length is 0");
		return NULL;
	}

	int file = open(filename, O_RDONLY);
	char* data = skynet_malloc(length);

	memset(data, 0, length + 1);

	int ret = read(file, data, length);
	if (ret < 0){
		printf("read file failed.");
		return NULL;
	}
		
	printf("returning %s file data.\n", filename);
	return (void*)data;
}

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
	printf("col_count is %d\n", col_count);
	
	// 开始解析excel表格
	read = getline(&line, &len, fp);	// feed first line
	while((read = getline(&line, &len, fp)) != -1){
		//printf("retrieved line of length %zu: \n", read);
		//printf("\e[0;32mget line: %s\e[0m\n", line);
		
		cJSON * line_item = cJSON_CreateArray();
		cJSON * item;
		char *type;
		int col_index = 0;
		char * token = strtok(line, "\t");
		while (token != NULL){
			// name = cJSON_GetObjectItem(cJSON_GetArrayItem(conf, col_index), "name")->valuestring;
			
			type = cJSON_GetObjectItem(cJSON_GetArrayItem(conf, col_index), "type")->valuestring;
			if (!strcmp(type, "string")){
				item = cJSON_CreateString(token);
			}
			else if (!strcmp(type, "number")){
				item = cJSON_CreateNumber(strtoull(token, NULL, 10));
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
					cJSON_AddItemToArray(item, cJSON_CreateNumber(strtoull(t, NULL, 10)));
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
				printf("temp is %s\n", temp);
				cJSON_AddItemToArray(json_files, cJSON_CreateString(temp));
			}
		}


		// 广度优先遍历文件夹,开始解析json文件
		cJSON * json_file = NULL;
		char excel_path_name[128];
		for (int i = 0; i < cJSON_GetArraySize(json_files); i++){
			cJSON* array_file_name = cJSON_GetArrayItem(json_files, i);
			sprintf(files_dir, "%s/json/%s.json", excel_path, array_file_name->valuestring);
			printf("%s \n", files_dir);
			char* data = (char*)read_file(files_dir);
			printf("%s\n", data);
			json_file = cJSON_Parse(data);
			skynet_free(data);

			if (cJSON_IsNull(json_file) || cJSON_IsInvalid(json_file)){
				printf("\e[0;31m %s file might has mistakes.\e[0m\n", files_dir);

			}
			cJSON * files = cJSON_GetObjectItem(json_file, "files");
			cJSON * fields = cJSON_GetObjectItem(json_file, "fields");

			// 遍历json规则对应下的所有excel文件
			int file_count = cJSON_GetArraySize(files);
			printf("%s file count is %d\n", array_file_name->valuestring, file_count);
			char* excel_filename;
			cJSON* ret = NULL;
			for (int i = 0; i < file_count; i++){
				excel_filename = cJSON_GetArrayItem(files, i)->valuestring;
				if (excel_filename == NULL){
					printf("parse json(%s) files(%s) failed...", array_file_name->valuestring, excel_filename);
					continue;
				}
				snprintf(excel_path_name, 256, "%s/%s.txt", excel_path, excel_filename);
				printf("parsing excel file: %s\n", excel_path_name);

				ret = parse_excel(excel_path_name, fields, ret);
				if ( ret == NULL || cJSON_IsNull(ret)){
					printf("parse excel file: %s failed.", excel_path_name);
				}
				else {
					printf("parse excel file: %s suc.", excel_path_name);
				}
			}

			cJSON* file_item = cJSON_CreateObject();
			cJSON_AddItemToObject(inst->excel_root, array_file_name->valuestring, file_item);
			cJSON_AddItemReferenceToObject(file_item, "fields", fields);
			cJSON_AddItemReferenceToObject(file_item, "data", ret);
			
			cJSON_DetachItemFromObject(json_file, "fields");
			cJSON_Delete(json_file);
		}

		printf("%s \n", cJSON_Print(inst->excel_root));
		closedir(dir);
		cJSON_Delete(json_files);
	}

	skynet_callback(ctx, inst, excel_cb);

	return 0;
}

