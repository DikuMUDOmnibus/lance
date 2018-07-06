typedef struct jobs_data JOBS_DATA;

struct jobs_data
{
    JOBS_DATA*	next;
    char*	title; /* tada! Job position title */
    int 	spaces; /* # of positions open */
}

