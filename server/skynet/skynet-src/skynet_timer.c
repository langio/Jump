#include "skynet.h"

#include "skynet_timer.h"
#include "skynet_mq.h"
#include "skynet_server.h"
#include "skynet_handle.h"

#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(__APPLE__)
#include <sys/time.h>
#endif

typedef void (*timer_execute_func)(void *ud, void *arg);

#define LOCK(q) while (__sync_lock_test_and_set(&(q)->lock,1)) {}
#define UNLOCK(q) __sync_lock_release(&(q)->lock);

#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR-1)
#define TIME_LEVEL_MASK (TIME_LEVEL-1)

struct timer_event
{
	uint32_t handle;
	int session;
};

struct timer_node
{
	struct timer_node *next;
	uint32_t expire;
};

struct link_list
{
	struct timer_node head;
	struct timer_node *tail;
};

struct timer
{
	struct link_list near[TIME_NEAR];
	struct link_list t[4][TIME_LEVEL];
	int lock;
	uint32_t time;
	uint32_t current;
	uint32_t starttime;
	uint64_t current_point;
	uint64_t origin_point;
};

static struct timer * TI = NULL;

//返回链表的下一个节点，并清空链表
static inline struct timer_node *
link_clear(struct link_list *list)
{
	struct timer_node * ret = list->head.next;
	list->head.next = 0;
	list->tail = &(list->head);

	return ret;
}

//将一个timer_node节点插入到链表的尾部
static inline void link(struct link_list *list, struct timer_node *node)
{
	list->tail->next = node;
	list->tail = node;
	node->next = 0;
}

//将timer_node节点加入到定时器中合适的链表尾部
static void add_node(struct timer *T, struct timer_node *node)
{
	uint32_t time = node->expire;
	uint32_t current_time = T->time;

	if ((time | TIME_NEAR_MASK) == (current_time | TIME_NEAR_MASK))
	{
		link(&T->near[time & TIME_NEAR_MASK], node);
	}
	else
	{
		int i;
		uint32_t mask = TIME_NEAR << TIME_LEVEL_SHIFT;
		for (i = 0; i < 3; i++)
		{
			if ((time | (mask - 1)) == (current_time | (mask - 1)))
			{
				break;
			}
			mask <<= TIME_LEVEL_SHIFT;
		}

		link(
				&T->t[i][((time >> (TIME_NEAR_SHIFT + i * TIME_LEVEL_SHIFT))
						& TIME_LEVEL_MASK)], node);
	}
}

//将timer_node节点加入到定时器中合适的链表尾部
static void timer_add(struct timer *T, void *arg, size_t sz, int time)
{
	struct timer_node *node = (struct timer_node *) skynet_malloc(
			sizeof(*node) + sz);
	memcpy(node + 1, arg, sz);

	LOCK(T);

	node->expire = time + T->time;
	add_node(T, node);

	UNLOCK(T);
}

//把定时器中某个链表中的节点分配到其他链表中
static void move_list(struct timer *T, int level, int idx)
{
	struct timer_node *current = link_clear(&T->t[level][idx]);
	while (current)
	{
		struct timer_node *temp = current->next;
		add_node(T, current);
		current = temp;
	}
}

//
static void timer_shift(struct timer *T)
{
	LOCK(T);
	int mask = TIME_NEAR;
	uint32_t ct = ++T->time;
	if (ct == 0)
	{
		move_list(T, 3, 0);
	}
	else
	{
		uint32_t time = ct >> TIME_NEAR_SHIFT;
		int i = 0;

		while ((ct & (mask - 1)) == 0)
		{
			int idx = time & TIME_LEVEL_MASK;
			if (idx != 0)
			{
				move_list(T, i, idx);
				break;
			}
			mask <<= TIME_LEVEL_SHIFT;
			time >>= TIME_LEVEL_SHIFT;
			++i;
		}
	}
	UNLOCK(T);
}

//将以timer_node为头的到时的链表节点全部执行（生成1个消息入队到handle的消息队列中）
static inline void dispatch_list(struct timer_node *current)
{
	do
	{
		struct timer_event * event = (struct timer_event *) (current + 1);
		struct skynet_message message;
		message.source = 0;
		message.session = event->session;
		message.data = NULL;
		message.sz = PTYPE_RESPONSE << HANDLE_REMOTE_SHIFT;

		skynet_context_push(event->handle, &message);

		struct timer_node * temp = current;
		current = current->next;
		skynet_free(temp);
	} while (current);
}

//执行到期的定时器链表
static inline void timer_execute(struct timer *T)
{
	LOCK(T);
	int idx = T->time & TIME_NEAR_MASK;

	while (T->near[idx].head.next)
	{
		struct timer_node *current = link_clear(&T->near[idx]);
		UNLOCK(T);
		// dispatch_list don't need lock T
		dispatch_list(current);
		LOCK(T);
	}

	UNLOCK(T);
}

static void timer_update(struct timer *T)
{
	// try to dispatch timeout 0 (rare condition)
	timer_execute(T);

	// shift time first, and then dispatch timer message
	timer_shift(T);

	timer_execute(T);

}

//定时器存储分配
static struct timer *
timer_create_timer()
{
	struct timer *r = (struct timer *) skynet_malloc(sizeof(struct timer));
	memset(r, 0, sizeof(*r));

	int i, j;

	for (i = 0; i < TIME_NEAR; i++)
	{
		link_clear(&r->near[i]);
	}

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < TIME_LEVEL; j++)
		{
			link_clear(&r->t[i][j]);
		}
	}

	r->lock = 0;
	r->current = 0;

	return r;
}

//将handle加入到定时器中
int skynet_timeout(uint32_t handle, int time, int session)
{
	if (time == 0)
	{
		struct skynet_message message;
		message.source = 0;
		message.session = session;
		message.data = NULL;
		message.sz = PTYPE_RESPONSE << HANDLE_REMOTE_SHIFT;

		if (skynet_context_push(handle, &message))
		{
			return -1;
		}
	}
	else
	{
		struct timer_event event;
		event.handle = handle;
		event.session = session;
		timer_add(TI, &event, sizeof(event), time);
	}

	return session;
}

// centisecond: 1/100 second
static void systime(uint32_t *sec, uint32_t *cs)
{
#if !defined(__APPLE__)
	struct timespec ti;
	clock_gettime(CLOCK_REALTIME, &ti);
	*sec = (uint32_t) ti.tv_sec;
	*cs = (uint32_t)(ti.tv_nsec / 10000000);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	*sec = tv.tv_sec;
	*cs = tv.tv_usec / 10000;
#endif
}

static uint64_t gettime()
{
	uint64_t t;
#if !defined(__APPLE__)

#ifdef CLOCK_MONOTONIC_RAW
#define CLOCK_TIMER CLOCK_MONOTONIC_RAW
#else
#define CLOCK_TIMER CLOCK_MONOTONIC
#endif

	struct timespec ti;
	clock_gettime(CLOCK_TIMER, &ti);
	t = (uint64_t) ti.tv_sec * 100;
	t += ti.tv_nsec / 10000000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (uint64_t)tv.tv_sec * 100;
	t += tv.tv_usec / 10000;
#endif
	return t;
}

void skynet_updatetime(void)
{
	uint64_t cp = gettime();
	if (cp < TI->current_point)
	{
		skynet_error(NULL, "time diff error: change from %lld to %lld", cp,
				TI->current_point);
		TI->current_point = cp;
	}
	else if (cp != TI->current_point)
	{
		uint32_t diff = (uint32_t)(cp - TI->current_point);
		TI->current_point = cp;

		uint32_t oc = TI->current;
		TI->current += diff;
		if (TI->current < oc)
		{
			// when cs > 0xffffffff(about 497 days), time rewind
			TI->starttime += 0xffffffff / 100;
		}
		int i;
		for (i = 0; i < diff; i++)
		{
			timer_update(TI);
		}
	}
}

//获取定时器开始时间
uint32_t skynet_gettime_fixsec(void)
{
	return TI->starttime;
}

//获取定时器当前时间
uint32_t skynet_gettime(void)
{
	return TI->current;
}

//初始化定时器
void skynet_timer_init(void)
{
	TI = timer_create_timer();
	systime(&TI->starttime, &TI->current);
	uint64_t point = gettime();
	TI->current_point = point;
	TI->origin_point = point;
}

