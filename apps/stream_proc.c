#include <xinu.h>
#include <run_cmd.h>
#include <streams.h>
#include <stdlib.h>
#include "tscdf_input.h"
#include "tscdf.h"

int cons_pt_id = SYSERR; 
int work_queue_depth;
int num_streams, time_window, output_time;

int stream_proc(int nargs, char* args[])
{

    ulong secs, msecs, time;
    secs = clktime;
    msecs = clkticks;

    // Parse arguments
    char usage[] = "Usage: run tscdf -s <num_streams> -w <work_queue_depth> -t <time_window> -o <output_time>\n";

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
                    num_streams = atoi(args[i]);
                    break;

                case 'w':
                    work_queue_depth = atoi(args[i]);
                    break;

                case 't':
                    time_window = atoi(args[i]);
                    break;

                case 'o':
                    output_time = atoi(args[i]);
                    break;

                default:
                    goto fail;
            }

            i -= 2;
        }
    }
    
    // Create streams
    struct stream *strs = (struct stream*) getmem(sizeof(struct stream) * num_streams);
    struct stream *str = strs;
    for (i = 0; i< num_streams; i++) {
        str->spaces = semcreate(1);
        str->items  = semcreate(0);
        str->mutex  = semcreate(0);
        str->head   = 0;
        str->tail   = 0;
        str->queue  = (struct data_element*) getmem(sizeof(struct data_element) * work_queue_depth);
        str++;
    }

    // Create port for getting child pid's
    cons_pt_id  = ptcreate(num_streams);
    if (cons_pt_id == SYSERR)
        goto fail;

    // Create consumer processes and initialize streams
    // Use `i` as the stream id.
    str = strs;
    for (i = 0; i < num_streams; i++) {
        char cons_name[16];
        sprintf(cons_name, "cons%d", i);
        resume(create(stream_consumer, 1024, 20, cons_name, 2, i, str));
        str++;
    }

    // Parse input header file data and populate work queue
    char *a;
    int st,ts,v;
    struct data_element *de;
    for (i=0; i < number_inputs; i++) {
        a = (char *) stream_input[i];
        st = atoi(a);
        while (*a++ != '\t');
        ts = atoi(a);
        while (*a++ != '\t');
        v = atoi(a);

        // write to message queue for given st
        str = strs + st;
        wait(str->spaces);
        de = str->queue + str->head;
        de->time = ts;
        de->value = v;
        str->head = (str->head + 1) % work_queue_depth;
        signal(str->items);
    }

    str = strs;
    for (i = 0; i < num_streams; i++) {
        wait(str->mutex);
        str++;
    }
    // Join all launched consumer processes
    for (i = 0; i < num_streams; i++) {
        printf("process %d exited\n",ptrecv(cons_pt_id));
    }


    // Measure the time of this entire function and report it at the end
    time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
    printf("time in ms: %u\n", time);

    signal(run_complete);
    return OK;
fail:
    printf("%s", usage);
    signal(run_complete);
    return SYSERR;
}

void stream_consumer(int32 id, struct stream *str)
{

    struct data_element *de;
    // Print the current id and pid
    pid32 cpid = getpid();
    printf("stream_consumer id:%d (pid:%d)\n", id, cpid);

    // Consume all values from the work queue of the corresponding stream
    struct tscdf *tc = tscdf_init(time_window);
    int32 *qarray;
    char output[64];
    int reached_end = 0;

    while(1){
        for(int i = 0; i < output_time; i++){
            wait(str->items);
            de = str->queue + str->tail;
            if (de->value == 0 && de->time == 0){
                reached_end = 1;
                break;
            }
            /* printf("s%d: %d\n", id, de->value); */
            tscdf_update(tc, de->time, de->value);
            str->tail = (str->tail + 1) % work_queue_depth;
            signal(str->spaces);
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
    printf("stream_consumer exiting\n");
    ptsend(cons_pt_id, cpid);
    signal(str->mutex);
}
