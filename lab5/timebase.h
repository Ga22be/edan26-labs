extern "C" {

void			init_timebase(void);
unsigned long long	tbr(void);
unsigned long long	timebase(void);
double			timebase_sec(void);
double			timebase_cycles(unsigned long long timebase_count);

}
