#include "skynet.h"

#include "skynet_module.h"

#include <assert.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_MODULE_TYPE 32

struct modules
{
	int count;
	int lock;
	const char * path;
	struct skynet_module m[MAX_MODULE_TYPE];
};

static struct modules * M = NULL;

//从所有动态库路径中搜索name标明的第一个动态库并将其打开，然后返回动态库的handl
static void *
_try_open(struct modules *m, const char * name)
{
	const char *l;
	const char * path = m->path;
	size_t path_size = strlen(path);
	size_t name_size = strlen(name);

	int sz = path_size + name_size;
	//search path
	void * dl = NULL;
	char tmp[sz];
	do
	{
		memset(tmp, 0, sz);
		while (*path == ';')
			path++;
		if (*path == '\0')
			break;
		l = strchr(path, ';');
		if (l == NULL)
			l = path + strlen(path);
		int len = l - path;
		int i;
		for (i = 0; path[i] != '?' && i < len; i++)
		{
			tmp[i] = path[i];
		}
		memcpy(tmp + i, name, name_size);
		if (path[i] == '?')
		{
			strncpy(tmp + i + name_size, path + i + 1, len - i - 1);
		}
		else
		{
			fprintf(stderr, "Invalid C service path\n");
			exit(1);
		}
		dl = dlopen(tmp, RTLD_NOW | RTLD_GLOBAL);
		path = l;
	} while (dl == NULL);

	if (dl == NULL)
	{
		fprintf(stderr, "try open %s failed : %s\n", name, dlerror());
	}

	return dl;
}

//在全局模块管理器中查询name标明的模块，返回该模块的指针，没有找到返回NULL
static struct skynet_module *
_query(const char * name)
{
	int i;
	for (i = 0; i < M->count; i++)
	{
		if (strcmp(M->m[i].name, name) == 0)
		{
			return &M->m[i];
		}
	}
	return NULL;
}

//查找动态库中xx_create、xx_init、xx_release、xx_signal四个函数的地址，并存入mod中对应的字段
static int _open_sym(struct skynet_module *mod)
{
	size_t name_size = strlen(mod->name);
	char tmp[name_size + 9]; // create/init/release/signal , longest name is release (7)
	memcpy(tmp, mod->name, name_size);
	strcpy(tmp + name_size, "_create");
	mod->create = dlsym(mod->module, tmp);
	strcpy(tmp + name_size, "_init");
	mod->init = dlsym(mod->module, tmp);
	strcpy(tmp + name_size, "_release");
	mod->release = dlsym(mod->module, tmp);
	strcpy(tmp + name_size, "_signal");
	mod->signal = dlsym(mod->module, tmp);

	return mod->init == NULL;
}

//在全局模块管理器中查询name标明的模块，返回模块的指针。如果没找到就从磁盘加载该动态库并存入全局模块管理器中
struct skynet_module *
skynet_module_query(const char * name)
{
	struct skynet_module * result = _query(name);
	if (result)
		return result;

	while (__sync_lock_test_and_set(&M->lock, 1))
	{
	}

	result = _query(name); // double check

	if (result == NULL && M->count < MAX_MODULE_TYPE)
	{
		int index = M->count;
		void * dl = _try_open(M, name);
		if (dl)
		{
			M->m[index].name = name;
			M->m[index].module = dl;

			if (_open_sym(&M->m[index]) == 0)
			{
				M->m[index].name = skynet_strdup(name);
				M->count++;
				result = &M->m[index];
			}
		}
	}

	__sync_lock_release(&M->lock);

	return result;
}

//将模块插入到模块管理器的尾部（插入前会检查该模块是否已经存在）
void skynet_module_insert(struct skynet_module *mod)
{
	while (__sync_lock_test_and_set(&M->lock, 1))
	{
	}

	struct skynet_module * m = _query(mod->name);
	assert(m == NULL && M->count < MAX_MODULE_TYPE);
	int index = M->count;
	M->m[index] = *mod;
	++M->count;
	__sync_lock_release(&M->lock);
}

//调用该模块对应的create函数
void *
skynet_module_instance_create(struct skynet_module *m)
{
	if (m->create)
	{
		return m->create();
	}
	else
	{
		return (void *) (intptr_t)(~0);
	}
}

//调用该模块对应的init函数，inst、ctx、parm依次为参数
int skynet_module_instance_init(struct skynet_module *m, void * inst,
		struct skynet_context *ctx, const char * parm)
{
	return m->init(inst, ctx, parm);
}

//调用该模块对应的release函数
void skynet_module_instance_release(struct skynet_module *m, void *inst)
{
	if (m->release)
	{
		m->release(inst);
	}
}

//调用该模块对应的信号处理函数
void skynet_module_instance_signal(struct skynet_module *m, void *inst,
		int signal)
{
	if (m->signal)
	{
		m->signal(inst, signal);
	}
}

//初始化全局模块管理器M，并设置so文件的路径
void skynet_module_init(const char *path)
{
	struct modules *m = skynet_malloc(sizeof(*m));
	m->count = 0;
	m->path = skynet_strdup(path);
	m->lock = 0;

	M = m;
}
