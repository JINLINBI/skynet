#include "skynet.h"
#include "rbtree.h"
#include "lua-seri.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <cJSON.h>
#include <unistd.h>
#define FILENAME_MAXLEN 1024

static char s_temp_str[FILENAME_MAXLEN + 1];
struct excel {
	int filescount;
	cJSON* excel_root;
	const char* excel_path;
	struct rb_cjson_root** rb_cjson_files_root;
};

struct read_block {
	char * buffer;
	int len;
	int ptr;
};

static void *
rb_read(struct read_block *rb, int sz) {
	if (rb->len < sz) {
		return NULL;
	}

	int ptr = rb->ptr;
	rb->ptr += sz;
	rb->len -= sz;
	return rb->buffer + ptr;
}

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
	#define TYPE_SHORT_STRING 4
	// hibits 0~31 : len
	#define TYPE_LONG_STRING 5
	switch (type) {
		case PTYPE_TEXT:
			printf("excel service get msg: %s\n", (char*)msg);
			struct read_block rb = {(char*)msg, sz, 0};

			uint8_t type = 0;
			uint8_t *t = rb_read(&rb, sizeof(type));
			if (t == NULL)
				break;
			type = *t;

			// push_value(L, &rb, type & 0x7, type>>3);
			switch(type & 0x7) {
				case TYPE_SHORT_STRING: {
					printf("get short string\n");
					break;
				}
				case TYPE_LONG_STRING: {
					printf("get long string\n");
					break;
				}
				default: {
					printf("get unknow type\n");
					break;
				}
			}
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
// parse json description struct files
// open json config directories like ...
// ../excel (excel_path )
// ../excel/json/*.json
// ../excel/json/***.json
// locate excel txt files below
// ../excel/item/item_list.txt
// ../excel/item/item_type.txt
void create_excel_root_json_struct(struct excel* inst) {
	// create excel root cjson obj
	inst->excel_root = cJSON_CreateObject();

	snprintf(s_temp_str, FILENAME_MAXLEN, "%s/json", inst->excel_path);
	DIR* dir = opendir(s_temp_str);
	struct dirent* dirt;
	
	if (dir != NULL) {
		while ((dirt = readdir(dir)) != NULL){
			if (strcmp(strrchr(dirt->d_name, '.'), ".json") == 0 ) {
				// json excel-desctription files save like this:
				// {
				//     "fields": [
				//         {"name": "id", "type": "number"},
				//         {"name": "name", "type": "string"},
				//			...
				//     ],
				//     "files": [
				//         "item/item_list"
				//     ]
				// }
				// parse data into memory json struct like this:
				// excel_root = 
				// {
				//     "item_list" : {
				//         	"fields": [
				//				... just like above json struct, copy them 
				//         	],
				//			"files": [
				//			    ... just like above json struct, copy them 
				//			],
				//			"data" : [[1, "gift", ...], [2, "weapon", ...], ...]
				//     }, ...
				// }
				cJSON* excel_txt_cjson = cJSON_CreateObject();
				sscanf(dirt->d_name, "%[^.]", s_temp_str);
				cJSON_AddItemToObject(inst->excel_root, s_temp_str, excel_txt_cjson);

				snprintf(s_temp_str, FILENAME_MAXLEN, "%s/json/%s", inst->excel_path, dirt->d_name);
				FILE* fp = fopen(s_temp_str, "r");
				if (fp == NULL) {
					goto fileout;
				}

				// filedata length
				fseek(fp, 0L, SEEK_END); /* 定位到文件末尾 */  
				size_t flen = ftell(fp);
				fseek(fp, 0L, SEEK_SET);

				char* file_data = skynet_malloc(flen);
				fread(file_data, flen, 1, fp);
				cJSON* json_file = cJSON_Parse(file_data);
				
				cJSON* fields = cJSON_GetObjectItem(json_file, "fields");
				cJSON* files = cJSON_GetObjectItem(json_file, "files");
				if (cJSON_IsNull(json_file) || cJSON_IsNull(fields) || cJSON_IsNull(files)) {
					goto out;
				}

				cJSON_AddItemReferenceToObject(excel_txt_cjson, "fields", fields);
				cJSON_AddItemReferenceToObject(excel_txt_cjson, "files", files);
				cJSON_AddItemReferenceToObject(excel_txt_cjson, "data", cJSON_CreateArray());

				cJSON_DetachItemFromObject(json_file, "fields");
				cJSON_DetachItemFromObject(json_file, "files");

out:
				cJSON_Delete(json_file);
				skynet_free(file_data);
fileout:
				fclose(fp);
			}
		}
	}
	inst->filescount = cJSON_GetArraySize(inst->excel_root);

	closedir(dir);
}


void parse_excel_root_json_data(struct excel* inst) {
	// rbtree index
	inst->rb_cjson_files_root = skynet_malloc(sizeof(struct rb_cjson_root*) * inst->filescount);
	memset(inst->rb_cjson_files_root, 0, sizeof(struct rb_cjson_root*) * inst->filescount);

	for (int i = 0; i < inst->filescount; i++) {
		cJSON* excel_txt_cjson = cJSON_GetArrayItem(inst->excel_root, i);
		if (cJSON_IsNull(excel_txt_cjson)) {
			continue;
		}

		cJSON* fields = cJSON_GetObjectItem(excel_txt_cjson, "fields");
		cJSON* files = cJSON_GetObjectItem(excel_txt_cjson, "files");
		cJSON* data = cJSON_GetObjectItem(excel_txt_cjson, "data");

		for (int j = 0; j < cJSON_GetArraySize(files); j++) {
			cJSON* files_item = cJSON_GetArrayItem(files, j);
			snprintf(s_temp_str, FILENAME_MAXLEN, "%s/%s.txt", inst->excel_path, files_item->valuestring);
			FILE* fp = fopen(s_temp_str, "r");
			if (fp == NULL) {
				continue;
			}

			char* line = NULL;
			size_t len;

			// skip first line which is note
			ssize_t read = getline(&line, &len, fp);
			while ((read = getline(&line, &len, fp)) != -1) {
				cJSON * excel_line_item = cJSON_CreateArray();
				cJSON_AddItemToArray(data, excel_line_item);
				char * token = strtok(line, "\t");
				for (int col = 0; col < cJSON_GetArraySize(fields); col++) {
					cJSON* col_item = cJSON_GetArrayItem(fields, col);
					cJSON* new_item = NULL;
					if (!strcmp(cJSON_GetObjectItem(col_item, "type")->valuestring, "string")) {
						new_item = cJSON_CreateString(token);
					}
					else if (!strcmp(cJSON_GetObjectItem(col_item, "type")->valuestring, "number")) {
						new_item = cJSON_CreateNumber(strtold(token, NULL));
					}
					else if (!strcmp(cJSON_GetObjectItem(col_item, "type")->valuestring, "int[]")) {
						new_item = cJSON_CreateArray();
						char* tmp = skynet_strdup(token);
						for (char* t = strsep(&tmp, "*"); t != NULL; t = strsep(&tmp, "*")){
							cJSON_AddItemToArray(new_item, cJSON_CreateNumber(strtold(t, NULL)));
						}
						skynet_free(tmp);
					}
					token = strtok(NULL, "\t");
					cJSON_AddItemToArray(excel_line_item, new_item);
				}
			}

			// create rbtree index for excel data
			inst->rb_cjson_files_root[i] = skynet_malloc(sizeof(struct rb_cjson_root));
			memset(inst->rb_cjson_files_root[i], 0, sizeof(struct rb_cjson_root));
			inst->rb_cjson_files_root[i]->name = skynet_strdup(excel_txt_cjson->string);
			create_rb_index_by_cjson(&inst->rb_cjson_files_root[i]->rb_root, files_item);
		}
	}
}


int excel_init(struct excel* inst, struct skynet_context* ctx, const char* excel_path) {
	inst->excel_root = cJSON_CreateObject();
	skynet_command(ctx, "REG", NULL);

	cJSON_Hooks cjson_hooks = {skynet_malloc, skynet_free};
	cJSON_InitHooks(&cjson_hooks);
	if (excel_path) {
		inst->excel_path = skynet_strdup(excel_path);
		create_excel_root_json_struct(inst);
		parse_excel_root_json_data(inst);
	}

	printf("%s", cJSON_Print(inst->excel_root));
	skynet_callback(ctx, inst, excel_cb);

	return 0;
}

