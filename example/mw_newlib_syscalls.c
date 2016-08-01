/**
 * @file   mw_syscalls.c
 * @date   19.07.2016
 * @author Klemens D. Morgenstern
 *
 * Published under [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)
  <pre>
    /  /|  (  )   |  |  /
   /| / |   \/    | /| /
  / |/  |  / \    |/ |/
 /  /   | (   \   /  |
               )
 </pre>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

typedef enum mw_func_t
{
    mw_func_chown = 0,
    mw_func_close,
    mw_func_fstat,
    mw_func_isatty,
    mw_func_link,
    mw_func_lseek,
    mw_func_open,
    mw_func_read,
    mw_func_stat,
    mw_func_sysmlink,
    mw_func_unlink,
    mw_func_write
} mw_func;

int mw_func_stub(mw_func func_type __attribute__((unused)),
                 const char* arg1  __attribute__((unused)),
                 const char* arg2  __attribute__((unused)),
                 int arg3          __attribute__((unused)),
                 int arg4          __attribute__((unused)),
                 int arg5          __attribute__((unused)),
                 char* arg6        __attribute__((unused)),
                 struct stat* arg7 __attribute__((unused)))
{
    return -1;
}

int _open (char* file, int flags, int mode);
int _read (int file, char* ptr, int len);
int _write(int file, char* ptr, int len);
int _lseek(int file, int ptr, int dir);
int _close(int);


int _fstat(int fildes, struct stat* st)
{
    return mw_func_stub(mw_func_fstat,
                0, // const char* arg1,
                0, // const char* arg2
                fildes, // int arg3
                0, // int arg4
                0, // int arg5
                0, // char* arg6
                st // struct stat* arg7
                );
}

int _isatty(int file)
{
    return mw_func_stub(mw_func_isatty,
                0, // const char* arg1,
                0, // const char* arg2
                file, // int arg3
                0, // int arg4
                0, // int arg5
                0, // char* arg6
                0 // struct stat* arg7
                );
}


int _link(char* existing, char* _new)
{
    return mw_func_stub(mw_func_link,
                existing, // const char* arg1,
                _new, // const char* arg2
                0, // int arg3
                0, // int arg4
                0, // int arg5
                0, // char* arg6
                0  // struct stat* arg7
                );
}


#ifndef MW_GCOV_BUFFER_SIZE
#define MW_GCOV_BUFFER_SIZE 0x400
#endif

//write buffer
static int write_fd  = -1;
static int write_pos = 0;
static char write_buf[MW_GCOV_BUFFER_SIZE];

static void flush_write()
{
    if (write_pos > 0)
    {
        _write(write_fd, write_buf, write_pos);
        write_fd = -1;
        write_pos = 0;
    }
}

static int _buffered_write(int file, char* ptr, int len __attribute__((unused)))
{
    if ((file != write_fd) || (write_pos == MW_GCOV_BUFFER_SIZE))
    {
        flush_write();
        write_fd = file;
    }

    write_buf[write_pos++] = *ptr;

    return 1;
}


//read buffer
static int read_fd = -1;
static int read_pos = 0;
static int read_end = 0;
static char *read_buf = 0;

void read_clear()
{
    free(read_buf);
    read_pos = 0;
    read_end = 0;
    read_fd = -1;
}

void _read_init_buffer(int fd)
{
    _lseek(fd, (size_t)0, SEEK_CUR); //set to zero.
    read_end = _lseek(fd, (size_t)0, SEEK_END);
    _lseek(fd, (size_t)0, SEEK_CUR);

    read_buf = (char*)malloc(read_end);

    _read(read_fd, read_buf, read_end);
}

int _read_buffered(char* ptr, int len)
{
    if (!read_buf)
        return -1;
    int i = 0;
    for (; ((read_pos + i) < read_end) && (i<len); i++) //read current buffer
        ptr[i] = read_buf[read_pos+i];
    return i;
}

int _lseek_buffered(int file __attribute__((unused)), int ptr, int dir )
{
    switch (dir)
    {
    case SEEK_CUR:
        read_pos = ptr + read_pos;
    case SEEK_END:
        read_pos = ptr + read_end;
    case SEEK_SET:
        read_pos = ptr;
    }
    return read_pos;
}


int _open(char* file, int flags, int mode)
{
    return mw_func_stub(mw_func_open,
                file, // const char* arg1
                0, // const char* arg2
                flags, // int arg3
                mode, // int arg4
                0, // int arg5
                0, // char* arg6
                0  // struct stat* arg7
                );
}


int _close(int fildes)
{
    flush_write();
    if (read_fd == fildes)
        read_clear();
    return mw_func_stub(mw_func_close,
                0, // const char* arg1,
                0, // const char* arg2
                fildes, // int arg3
                0, // int arg4
                0, // int arg5
                0, // char* arg6
                0 // struct stat* ar7B
                );
}

int _lseek(int file, int ptr, int dir)
{
    if (file == read_fd)
        return _lseek_buffered(file, ptr, dir);
    else
        return mw_func_stub(mw_func_lseek,
                0, // const char* arg1,
                0, // const char* arg2
                file, // int arg3
                ptr, // int arg4
                dir, //int arg5
                0, // char * arg6
                0  // struct stat* arg7
                );
}


int _read(int file, char* ptr, int len)
{
    if ((file != -1) && (file == read_fd))
        return _read_buffered(ptr, len);
    else if ((read_fd == -1) && (len == 1))
    {
       _read_init_buffer(file);
       _read_buffered(ptr, len);
    }
    return mw_func_stub(mw_func_read,
                0,    // const char* arg1,
                0,    // const char* arg2
                file, // int arg3
                len,  // int arg4
                0,    // int arg5
                ptr,  // char* arg6
                0     // struct stat* arg7
                );
}

int _write(int file, char* ptr, int len)
{
    if (len == 1)
        return _buffered_write(file, ptr, len);
    else
    {

        return mw_func_stub(mw_func_write,
                0,    // const char* arg1
                0,    // const char* arg2
                file, // int arg3
                len,  // int arg4
                0,    // int arg5
                ptr,  // char* arg6
                0     // struct stat* arg7
                );
    }
}

int _stat(const char* file, struct stat* st)
{
    return mw_func_stub(mw_func_stat,
                file, // const char* arg1
                0,    // const char* arg2
                0,    // int arg3
                0,    // int arg4
                0,    // int arg5
                0,    // char* arg6
                st    // struct stat* arg7
                );
}

int _symlink(const char* path1, const char* path2)
{
    return mw_func_stub(mw_func_sysmlink,
                path1, // const char* arg1,
                path2, // const char* arg2
                0,     // int arg3
                0,     // int arg4
                0,     // int arg5
                0,     // char* arg6
                0      // struct stat* arg7
                );
}

int _unlink(char* name)
{
    return mw_func_stub(mw_func_unlink,
                0,    // const char* arg1
                0,    // const char* arg2
                0,    // int arg3
                0,    // int arg4
                0,    // int arg5
                name, // char* arg6
                0     // struct stat* arg7
                );
}

