#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void vec_expand(char **data, int *length, int *capacity, int memsz)
{
    if (*length + 1 > *capacity)
    {
        if (*capacity == 0)
            *capacity = 1;
        else
            *capacity <<= 1;
        
        *data = realloc(*data, (size_t)(*capacity * memsz));
    }
}

static void vec_splice(char **data, int *length, int *capacity, int memsz, int start, int count )
{
    (void) capacity;
    memmove(*data + start * memsz,
            *data + (start + count) * memsz,
            (*length - start - count) * memsz);
}


#define Vec(T)\
    struct { T *data; int length, capacity; }


#define vec_unpack(v)\
    (char**)&(v)->data, &(v)->length, &(v)->capacity, sizeof(*(v)->data)


#define vec_init(v)\
    memset((v), 0, sizeof(*(v)))


#define vec_deinit(v)\
    free((v)->data)


#define vec_clear(v)\
    ((v)->length = 0)


#define vec_push(v, val)\
    ( vec_expand(vec_unpack(v)),\
    (v)->data[(v)->length++] = (val) )

#define vec_pop(v)\
    ( assert((v)->length > 0),\
    (v)->data[ -- (v)->length] )


#define vec_get(v, pos)\
    (assert(pos < (v)->length),\
    (v)->data[pos])


#define vec_splice(v, start, count)\
    ( vec_splice(vec_unpack(v), start, count),\
    (v)->length -= (count) )


/* @test */
int main(void)
{
    Vec(int) array;
    vec_init(&array);
    vec_push(&array, 34);
    vec_push(&array, 12);
    vec_push(&array, 'c');

    printf("total length = %d\n", array.length);

    int inum;
    printf("array get %c\n", vec_get(&array, 2));
    inum = vec_pop(&array);
    printf("inum = %d\n", inum);
    printf("array length = %d\n", array.length);

    int fnum;
    fnum = vec_pop(&array);
    printf("fnum = %d\n", fnum);
    printf("array length = %d\n", array.length);

    char ch;
    ch = vec_pop(&array);
    printf("ch = %d\n", ch);
    printf("array length = %d\n", array.length);


    vec_deinit(&array);
}
