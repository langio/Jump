#include "skynet.h"
#include "skynet_mq.h"
#include "skynet_handle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define DEFAULT_QUEUE_SIZE 64
#define MAX_GLOBAL_MQ 0x10000

// 0 means mq is not in global mq.
// 1 means mq is in global mq , or the message is dispatching.

#define MQ_IN_GLOBAL 1
#define MQ_OVERLOAD 1024

struct message_queue
{
	uint32_t handle;
	int cap;
	int head;
	int tail;
	int lock;
	int release;
	int in_global;
	int overload;
	int overload_threshold;
	struct skynet_message *queue;
	struct message_queue *next;
};

struct global_queue
{
	struct message_queue *head;
	struct message_queue *tail;
	int lock;
};

static struct global_queue *Q = NULL;

#define LOCK(q) while (__sync_lock_test_and_set(&(q)->lock,1)) {}
#define UNLOCK(q) __sync_lock_release(&(q)->lock);

//将消息队列入队到全局队列Q的尾部
void skynet_globalmq_push(struct message_queue * queue)
{
	struct global_queue *q = Q;

	LOCK(q)
	assert(queue->next == NULL);
	if (q->tail)
	{
		q->tail->next = queue;
		q->tail = queue;
	}
	else
	{
		q->head = q->tail = queue;
	}
	UNLOCK(q)
}


//从全局队列Q的头部弹出一个消息队列
struct message_queue *
skynet_globalmq_pop()
{
	struct global_queue *q = Q;

	LOCK(q)
	struct message_queue *mq = q->head;
	if (mq)
	{
		q->head = mq->next;
		if (q->head == NULL)
		{
			assert(mq == q->tail);
			q->tail = NULL;
		}
		mq->next = NULL;
	}
	UNLOCK(q)

	return mq;
}

//创建1个消息队列，并将handle保存到队列信息中
struct message_queue *
skynet_mq_create(uint32_t handle)
{
	struct message_queue *q = skynet_malloc(sizeof(*q));
	q->handle = handle;
	q->cap = DEFAULT_QUEUE_SIZE;
	q->head = 0;
	q->tail = 0;
	q->lock = 0;
	// When the queue is create (always between service create and service init) ,
	// set in_global flag to avoid push it to global queue .
	// If the service init success, skynet_context_new will call skynet_mq_force_push to push it to global queue.
	q->in_global = MQ_IN_GLOBAL;
	q->release = 0;
	q->overload = 0;
	q->overload_threshold = MQ_OVERLOAD;
	q->queue = skynet_malloc(sizeof(struct skynet_message) * q->cap);
	q->next = NULL;

	return q;
}

//释放消息队列中元素的内存并释放消息队列
static void _release(struct message_queue *q)
{
	assert(q->next == NULL);
	skynet_free(q->queue);
	skynet_free(q);
}

//获取消息队列保存的handle
uint32_t skynet_mq_handle(struct message_queue *q)
{
	return q->handle;
}

//获取消息队列中元素的个数
int skynet_mq_length(struct message_queue *q)
{
	int head, tail, cap;

	LOCK(q)
	head = q->head;
	tail = q->tail;
	cap = q->cap;
	UNLOCK(q)

	if (head <= tail)
	{
		return tail - head;
	}
	return tail + cap - head;
}

//队列没有过载返回0，否则返回过载数量名设置过载为0
int skynet_mq_overload(struct message_queue *q)
{
	if (q->overload)
	{
		int overload = q->overload;
		q->overload = 0;
		return overload;
	}
	return 0;
}

//弹出一个消息，到message中，成功返回0，否则返回1
int skynet_mq_pop(struct message_queue *q, struct skynet_message *message)
{
	int ret = 1;
	LOCK(q)

	if (q->head != q->tail)
	{
		*message = q->queue[q->head++];
		ret = 0;
		int head = q->head;
		int tail = q->tail;
		int cap = q->cap;

		if (head >= cap)
		{
			q->head = head = 0;
		}
		int length = tail - head;
		if (length < 0)
		{
			length += cap;
		}
		while (length > q->overload_threshold)
		{
			q->overload = length;
			q->overload_threshold *= 2;
		}
	}
	else
	{
		// reset overload_threshold when queue is empty
		q->overload_threshold = MQ_OVERLOAD;
	}

	if (ret)
	{
		q->in_global = 0;
	}

	UNLOCK(q)

	return ret;
}

//将消息队列的容量扩大到原来的2倍
static void expand_queue(struct message_queue *q)
{
	struct skynet_message *new_queue = skynet_malloc(
			sizeof(struct skynet_message) * q->cap * 2);
	int i;
	for (i = 0; i < q->cap; i++)
	{
		new_queue[i] = q->queue[(q->head + i) % q->cap];
	}
	q->head = 0;
	q->tail = q->cap;
	q->cap *= 2;

	skynet_free(q->queue);
	q->queue = new_queue;
}

//将消息入队到消息队列的尾部，若消息队列慢了，则进行扩容，若消息队列不在全局队列中，则把消息队列入队到全局队列的尾部
void skynet_mq_push(struct message_queue *q, struct skynet_message *message)
{
	assert(message);
	LOCK(q)

	q->queue[q->tail] = *message;
	if (++q->tail >= q->cap)
	{
		q->tail = 0;
	}

	if (q->head == q->tail)
	{
		expand_queue(q);
	}

	if (q->in_global == 0)
	{
		q->in_global = MQ_IN_GLOBAL;
		skynet_globalmq_push(q);
	}

	UNLOCK(q)
}

//初始化全局消息队列
void skynet_mq_init()
{
	struct global_queue *q = skynet_malloc(sizeof(*q));
	memset(q, 0, sizeof(*q));
	Q = q;
}

//标记消息队列为释放状态，若消息队列不在全局队列中，则把消息队列入队到全局队列的尾部
void skynet_mq_mark_release(struct message_queue *q)
{
	LOCK(q)
	assert(q->release == 0);
	q->release = 1;
	if (q->in_global != MQ_IN_GLOBAL)
	{
		skynet_globalmq_push(q);
	}
	UNLOCK(q)
}

//对消息队列中的每个消息调用drop_func，然后释放消息队列
static void _drop_queue(struct message_queue *q, message_drop drop_func,
		void *ud)
{
	struct skynet_message msg;
	while (!skynet_mq_pop(q, &msg))
	{
		drop_func(&msg, ud);
	}
	_release(q);
}

//如果消息队列被标记为释放状态，那么就释放掉该消息队列，否则将该消息队列入队到全局队列的尾部
void skynet_mq_release(struct message_queue *q, message_drop drop_func,
		void *ud)
{
	LOCK(q)

	if (q->release)
	{
		UNLOCK(q)
		_drop_queue(q, drop_func, ud);
	}
	else
	{
		skynet_globalmq_push(q);
		UNLOCK(q)
	}
}
