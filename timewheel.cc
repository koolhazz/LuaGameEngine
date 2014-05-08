#include "timewheel.h"

int
tw_create(time_wheel_t** t, int len)
{
	*t = (time_scale_t* (*)[15])malloc(sizeof(time_wheel_t));

	for (int i = 0; i < len; i++) {
		*((**t) + i) = new time_scale_t;
	}

	return *t == NULL ? 1 : 0;
}

time_scale_t*
tw_wheeling(time_wheel_t* t, int len)
{
	time_scale_t 	**a; 
	time_scale_t	*s, *closed;

	a = *t;
	closed = a[0];

	for (int i = 0; i < len - 1; i++) {
		s = a[i];
		a[i] = a[i + 1];
	}

	a[len - 1] = closed;

	return closed;
}

int
tw_insert(time_wheel_t* t, int len, SocketHandler* s)
{
	time_scale_t	**a;

	a = *t;

	a[len - 1]->insert(s);  //*((*t) + 14)->insert(s);

	return 0;
}

