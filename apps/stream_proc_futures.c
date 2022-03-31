#include <xinu.h>
#include <run_cmd.h>
#include <streams.h>
#include <future.h>
#include <stdlib.h>
#include "tscdf.h"

int cons_pt_idf = SYSERR; 
int work_queue_depth_f, num_streams_f, time_window_f, output_time_f;

int stream_proc_futures(int nargs, char* args[])
{

    ulong secs, msecs, time;
    secs = clktime;
    msecs = clkticks;

    // Parse arguments
    char usage[] =  "run tscdf_fq -s <num_streams_f> -w <work_queue_depth_f> -t <time_window_f> -o <output_time_f>\n";

    int i;
    char *ch, c;
    if (nargs != 9) {
        goto fail;
    } else {
        i = nargs - 1;
        while (i > 0) {
            ch = args[i - 1];
            c = *(++ch);

            switch (c) {
                case 's':
                    num_streams_f = atoi(args[i]);
                    break;

                case 'w':
                    work_queue_depth_f = atoi(args[i]);
                    break;

                case 't':
                    time_window_f = atoi(args[i]);
                    break;

                case 'o':
                    output_time_f = atoi(args[i]);
                    break;

                default:
                    goto fail;
            }

            i -= 2;
        }
    }
    
    // Create streams
    future_t **futs =(future_t **) getmem(sizeof(future_t *) * num_streams_f);

    for (i = 0; i< num_streams_f; i++) {
        futs[i] = future_alloc(FUTURE_QUEUE, (sizeof (struct data_element)), work_queue_depth_f); 
    }

    // Create port for getting child pid's
    cons_pt_idf  = ptcreate(num_streams_f);
    if (cons_pt_idf == SYSERR)
        goto fail;

    // Create consumer processes and initialize streams
    // Use `i` as the stream id.
    for (i = 0; i < num_streams_f; i++) {
        char cons_name[16];
        sprintf(cons_name, "cons%d", i);
        resume(create(stream_consumer_future, 1024, 20, cons_name, 2, i, futs[i]));
    }

    // Parse input header file data and populate work queue
    char *a;
    int st,ts,v;
    struct data_element de;
    for (i=0; i < n_input; i++) {
        a = (char *) stream_input[i];
        st = atoi(a);
        while (*a++ != '\t');
        ts = atoi(a);
        while (*a++ != '\t');
        v = atoi(a);

        // write to message queue for given st
        de.time = ts;
        de.value = v;
        future_set(futs[st], (char *) &de);
    }

    for (i = 0; i < num_streams_f; i++) {
        future_free(futs[i]);
    }

    // Join all launched consumer processes
    for (i = 0; i < num_streams_f; i++) {
        kprintf("process %d exited\n",ptrecv(cons_pt_idf));
    }

    // Measure the time of this entire function and report it at the end
    time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
    printf("time in ms: %u\n", time);

    // Cleanup
    /* ptdelete(cons_pt_idf); */
    freemem((char*) futs, (sizeof(future_t *) * num_streams_f));
    signal(run_complete);
    return OK;
fail:
    printf("%s", usage);
    signal(run_complete);
    return SYSERR;
}

void stream_consumer_future(int32 id, future_t *f)
{

    struct data_element de;
    // Print the current id and pid
    pid32 cpid = getpid();
    kprintf("stream_consumer_future id:%d (pid:%d)\n", id, cpid);

    // Consume all values from the work queue of the corresponding stream
    struct tscdf *tc = tscdf_init(time_window_f);
    int32 *qarray;
    char output[64];
    int reached_end = 0;

    while(1){
        for(int i = 0; i < output_time_f; i++){
            future_get(f, (char *) &de);
            if (de.value == 0 && de.time == 0){
                reached_end = 1;
                break;
            }
            tscdf_update(tc, de.time, de.value);
        }
        if(reached_end)
            break;
        qarray = tscdf_quartiles(tc);

        if (qarray == NULL) {
            kprintf("tscdf_quartiles returned NULL\n");
            continue;
        }

        sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
        kprintf("%s\n", output);
        freemem((char *) qarray, (6*sizeof(int32)));
    }
    tscdf_free(tc);
    kprintf("stream_consumer_future exiting\n");
    ptsend(cons_pt_idf, cpid);
    return;
}
