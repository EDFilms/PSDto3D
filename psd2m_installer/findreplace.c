// ADAPTED FROM ROSETTA CODE PROJECT:
// https://rosettacode.org/wiki/Globally_replace_text_in_several_files#C


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <unistd.h>
//#include <err.h>
#include <string.h>

#include <io.h>
 
char * find_match(const char *buf, const char * buf_end, const char *pat, size_t len)
{
    ptrdiff_t i;
    const char *start = buf;
    while (start + len < buf_end) {
        for (i = 0; i < len; i++)
            if (start[i] != pat[i]) break;
 
        if (i == len) return (char *)start;
        start++;
    }
    return 0;
}
 
int replace(const char *from, const char *to, const char *in_fname, const char *out_fname)
{
    #define bail(msg,fname) { printf(msg" '%s'\n", fname); goto done; }
//#define bail(msg,fname) { warn(msg" '%s'", fname); goto done; }
    printf( "Input file [%s] \n", in_fname );
    printf( "Output file [%s] \n", out_fname );
    printf( "Replacing [%s] with [%s] \n", from, to );

    struct stat st;
    int ret = 0;
    char *buf = 0, *start, *end;
    size_t len = strlen(from), nlen = strlen(to);

    int in_fd = open(in_fname, O_RDONLY|O_BINARY);
    if (in_fd == -1) bail("Can't open input file",in_fname);
    if (fstat(in_fd, &st) == -1) bail("Can't stat",in_fname);
    if (!(buf = malloc(st.st_size))) bail("Can't alloc",in_fname);
    int file_size = st.st_size;
    int read_size = read(in_fd, buf, st.st_size);
    if (read_size != file_size) bail("Bad read",in_fname);
    if (in_fd != -1) close(in_fd);
    
    int out_fd = open(out_fname, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IREAD|S_IWRITE);
    if (out_fd == -1) bail("Can't open output file",out_fname);

    int count = 0;
    start = buf;
    end = find_match(start, buf + st.st_size, from, len);
    if (!end) { // no match found, write file unchanged and stop
      write(out_fd, start, buf + st.st_size - start);
      goto done;
    }
 
    //ftruncate(out_fd, 0);
    lseek(out_fd, 0, 0);
    do {
        count++;
        //printf( "No match between %i-%i \n", (int)(start-buf), (int)(end-buf) );
        int n = (end-start);
        if( n>0 ) write(out_fd, start, end - start);    // write content before match
        //printf( "wrote %i before match, should be %i \n", n, (int)(end - start) );
        //printf( "Match at %i, length %i \n", (int)(end-buf), (int)(nlen) );
        n = write(out_fd, to, nlen);        // write replacement of match
        //printf( "wrote %i after match, should be %i, to:%s \n", n, (int)nlen, to );
        start = end + len;        // skip to end of match
                        // find match again
        end = find_match(start, buf + st.st_size, from, len);
    } while (end);
 
    // write leftover after last match
    if (start < buf + st.st_size)
        write(out_fd, start, buf + st.st_size - start);
 
done:
    printf( "Matches found: %i \n", count );
    if (out_fd != -1) close(out_fd);
    if (buf) free(buf);
    return ret;
}
 
int main( int argc, char** argv )
{
    if( argc!=5 )
        printf("usage: findreplace find_string replace_string in_filename out_filename\n" );
    else
    {
        const char *from = argv[1];
        const char *to   = argv[2];
        replace(from, to, argv[3], argv[4]);
    }
 
    return 0;
}