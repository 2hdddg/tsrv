#include <stdio.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "test.h"
#include "receivebuffer.h"

static int _open_with_unlim_data()
{
    return open("/dev/urandom", O_RDONLY);
}

static int _open_no_data()
{
    return open("/dev/null", O_RDONLY);
}

static int _open_with_seq(char max)
{
    FILE *file = tmpfile();
    int fd = fileno(file);
    char c;

    for (c = 0; c <= max; c++){
        write(fd, &c, 1);
    }
    fseek(file, 0, SEEK_SET);

    return fd;
}

static void alloc()
{
    START();

    /* Bad input */
    RHandle h = receivebuffer_alloc(1, -1);
    ASSERT(!h);

    /* Happy path */
    h = receivebuffer_alloc(1, 10);
    ASSERT(h != NULL);
    receivebuffer_free(h);
}

static void prepare()
{
    START();
    fd_set set;
    int fd = 5;
    RHandle h = receivebuffer_alloc(fd, 10);
    int highest = fd -1;

    /* highest is low */
    FD_ZERO(&set);
    receivebuffer_prepare(h, &set, &highest);
    ASSERT(FD_ISSET(fd, &set));
    ASSERT(highest == fd);

    /* highest is high */
    FD_ZERO(&set);
    highest = fd + 1;
    receivebuffer_prepare(h, &set, &highest);
    ASSERT(FD_ISSET(fd, &set));
    ASSERT(highest > fd);

    receivebuffer_free(h);
}

static void receive()
{
    START();
    fd_set set;
    int fd;
    RHandle h;
    enum received received;

    /* Illegal filedescriptor -> error */
    FD_ZERO(&set);
    fd = -1;
    FD_SET(fd, &set);
    h = receivebuffer_alloc(fd, 10);
    received = receivebuffer_receive(h, &set);
    ASSERT(received == received_error);
    receivebuffer_free(h);

    /* Not set -> nothing */
    FD_ZERO(&set);
    fd = _open_no_data();
    h = receivebuffer_alloc(fd, 10);
    received = receivebuffer_receive(h, &set);
    ASSERT(received == received_nothing);
    receivebuffer_free(h);
    close(fd);

    /* Set but no data -> end of file */
    FD_ZERO(&set);
    fd = _open_no_data();
    FD_SET(fd, &set);
    h = receivebuffer_alloc(fd, 10);
    received = receivebuffer_receive(h, &set);
    ASSERT(received == received_endoffile);
    receivebuffer_free(h);
    close(fd);

    /* Set with data -> data */
    FD_ZERO(&set);
    fd = _open_with_unlim_data();
    FD_SET(fd, &set);
    h = receivebuffer_alloc(fd, 10);
    received = receivebuffer_receive(h, &set);
    ASSERT(received == received_data);
    receivebuffer_free(h);
    close(fd);

    /* Read twice without pop -> error (full) */
    FD_ZERO(&set);
    fd = _open_with_unlim_data();
    FD_SET(fd, &set);
    h = receivebuffer_alloc(fd, 10);
    receivebuffer_receive(h, &set);
    received = receivebuffer_receive(h, &set);
    ASSERT(received == received_error);
    receivebuffer_free(h);
    close(fd);

    /* Read twice with pop -> data */
    FD_ZERO(&set);
    fd = _open_with_unlim_data();
    FD_SET(fd, &set);
    h = receivebuffer_alloc(fd, 10);
    receivebuffer_receive(h, &set);
    receivebuffer_pop(h, 5);
    received = receivebuffer_receive(h, &set);
    ASSERT(received == received_data);
    receivebuffer_free(h);
    close(fd);
}

void get()
{
    START();
    fd_set set;
    int fd;
    RHandle h;
    int size;
    char *buffer;
    const int buffersize = 3;
    const int sequencesize = 4;

    fd = _open_with_seq(sequencesize - 1);
    FD_ZERO(&set);
    FD_SET(fd, &set);

    /* Fill buffer and get -> correct data */
    h = receivebuffer_alloc(fd, buffersize);
    receivebuffer_receive(h, &set);
    size = receivebuffer_get(h, &buffer);
    ASSERT(size == buffersize);
    ASSERT(buffer[0] == 0);
    ASSERT(buffer[1] == 1);
    ASSERT(buffer[2] == 2);

    /* Pop all and get -> no data */
    receivebuffer_pop(h, buffersize);
    size = receivebuffer_get(h, &buffer);
    ASSERT(size == 0);
    ASSERT(buffer == NULL);

    /* Read rest of sequence -> correct data */
    receivebuffer_receive(h, &set);
    size = receivebuffer_get(h, &buffer);
    ASSERT(size == 1);
    ASSERT(buffer[0] == 3);

    receivebuffer_free(h);
    close(fd);
}

int main()
{
    alloc();
    prepare();
    receive();
    get();

    return REPORT();
}
