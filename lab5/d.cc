#include <cstdio>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "timebase.h"

class spin_lock {
	std::atomic_flag flag = ATOMIC_FLAG_INIT;

	public:

		void lock() {
			while (flag.test_and_set(std::memory_order_acquire));
		}

		void unlock(){
			flag.clear(std::memory_order_release);
		}
};


class worklist_t {
	int*			a;
	size_t			n;
	size_t			total;	// sum a[0]..a[n-1]
	spin_lock		lock;
	
public:
	worklist_t(size_t max)
	{
		n = max+1;
		total = 0;

		a = (int*) calloc(n, sizeof(a[0]));
		if (a == NULL) {
			fprintf(stderr, "no memory!\n");
			abort();
		}
	}

	~worklist_t()
	{	
		free(a);
	}

	void reset()
	{
		total = 0;
		memset(a, 0, n*sizeof a[0]);
	}

	void put(int num)
	{
		lock.lock();
		a[num] += 1;
		total += 1;
		lock.unlock();
	}

	int get()
	{
		int				i;
		int				num;
		
		lock.lock();
		while (total < 1) {
			lock.unlock();
			std::this_thread::yield();
			lock.lock();
		}

		for (i = 1; i <= n; i += 1)
			if (a[i] > 0)
				break;

		if (i <= n) {
			a[i] -= 1;
			total -= 1;
		} else if (a[0] == 0) {
			fprintf(stderr, "corrupt data at line %d!\n", __LINE__);
			abort();
		} else
			i = 0;
		lock.unlock();
		return i;
	}
};

static worklist_t*		worklist;
static std::atomic<unsigned long long>	sum;
static std::mutex		sum_mtx;
static std::condition_variable	sum_cdt;
static int			iterations;
static int			max;

static void produce()
{
	int		i;
	int		n;

	for (i = 0; i < iterations; i += 1)
		for (n = 1; n <= max; n += 1)
			worklist->put(n);

	worklist->put(0);
}

static unsigned long long factorial(unsigned long long n)
{
	return n <= 1 ? 1 : n * factorial(n - 1);
}

static void consume()
{
	int			n;
	unsigned long long	f;

	while ((n = worklist->get()) > 0) {
		f = factorial(n);
//		std::unique_lock<std::mutex> lck(sum_mtx);
		sum += f;
//		lck.unlock();
	}
}

static void work()
{
	sum = 0;
	worklist->reset();

	std::thread p(produce);
//	p.join();
	std::thread a(consume);
	std::thread b(consume);
	std::thread c(consume);
	std::thread d(consume);

	p.join();
	a.join();
	b.join();
	c.join();
	d.join();
}

int main(void)
{
	double			begin;
	double			end;
	unsigned long long	correct;
	int			i;
	
	printf("mutex/condvar and mutex for sum\n");

	init_timebase();

	iterations	= 100000;
	max		= 12;
	correct		= 0;

	for (i = 1; i <= max; i += 1)
		correct += factorial(i);
	correct *= iterations;

	worklist = new worklist_t(max);

	for (i = 1; i <= 10; i += 1) {
		begin = timebase_sec();
		work();
		end = timebase_sec();

		if (sum != correct) {
			fprintf(stderr, "wrong output! %llu vs %llu\n", sum.load(std::memory_order_relaxed), correct);
			abort();
		}

		printf("T = %1.2lf s\n", end - begin);
	}

	delete worklist;

	return 0;
}
