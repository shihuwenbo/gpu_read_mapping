#include "suffix_sort.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// helper function, given a letter, return its rank in the alphabet
int alpha_rank(char l)
{
    // rank alphabetically $<a<c<g<t
    switch(l)
    {
        case '$': return 0;
        case 'a': return 1;
        case 'c': return 2;
        case 'g': return 3;
        case 't': return 4;
    }
    return -1;
}

// struct for naive suffix sort algorithm
typedef struct str_int
{
    char* str;
    int idx;
}
str_int_t;

// comparator for qsort used in suffx sorting algorithm
int suffix_compare(const void* lhs, const void* rhs)
{
    str_int_t* lhs_sit = (str_int_t*) lhs;
    str_int_t* rhs_sit = (str_int_t*) rhs;
    return strcmp(lhs_sit->str, rhs_sit->str);
}

// naive suffix sorting algorithm
int* suffix_sorting_0(char* str, int len)
{
    // generate the suffixes
    str_int_t* suffixes = (str_int_t*) malloc(len*sizeof(str_int_t));
    for(int i = 0; i < len; i++)
    {
        suffixes[i].str = str++;
        suffixes[i].idx = i;
    }

    // sort the suffixes
    qsort(suffixes, len, sizeof(str_int_t), suffix_compare);

    // generate the suffix array
    int* sa = (int*) malloc(len*sizeof(int));
    for(int i = 0; i < len; i++)
        sa[i] = suffixes[i].idx;

    // return the result
    return sa;
}


// suffix sorting algorithm by manber and myers
int* suffix_sorting_1(char* str, int len)
{
/* allocating memory for arrays required for sorting */

    int* pos = (int*) malloc(len * sizeof(int));
    int* prm = (int*) malloc(len * sizeof(int));
    int* bh = (int*) malloc(len * sizeof(int));
    int* b2h = (int*) malloc(len * sizeof(int));
    int* count = (int*) malloc(len * sizeof(int));

/* first stage: radix sorting according to the first letter */

    // initialize the bin for counting sort
    int bin[ALPHA_SIZE + 1];
    for(int i = 0; i < ALPHA_SIZE + 1; i++)
        bin[i] = 0;

    // count the number of each letter
    for(int i = 0; i < len; i++)
        bin[alpha_rank(str[i])]++;

    // compute offset of each letter
    int offset[ALPHA_SIZE + 1];
    offset[0] = 0;
    for(int i = 1; i < ALPHA_SIZE + 1; i++)
        offset[i] = offset[i-1] + bin[i-1];

    // use the offset to compute the first stage suffix array
    for(int i = 0; i < len; i++)
        pos[offset[alpha_rank(str[i])]++] = i;

    // compute the first stage prm
    for(int i = 0; i < len; i++)
        prm[pos[i]] = i;

    // compute the first stage bh
    bh[0] = 1;
    for(int i = 1; i < len; i++)
        bh[i] = str[pos[i]] != str[pos[i-1]] ? 1 : 0;

/* inductive stages: sort pos incrementally in n*log(n) time */

    for(int H = 1; H < len; H <<= 1)
    {

/* reset prm such that prm[i] points to the left most bucket */

        // find how many bins and zero
        int num_bin = 0, num_zero = 0;
        for(int i = 0; i < len; i++)
        {
            if(bh[i] == 1)
                num_bin++;
            else
                num_zero++;
        }

        // break if the number of bins is less or equal to H
        if(num_zero <= H)
            break;

        // count how many elements in each bin
        int* bin_size = (int*) malloc(num_bin*sizeof(int));
        bin_size[0] = 1;
        for(int i = 1; i < num_bin; i++)
            bin_size[i] = 0;
        for(int i = 1, j = 0; i < len; i++)
        {
            if(bh[i] == 1)
                j++;
            bin_size[j]++;
        }

        // compute offset
        int* bin_offset = (int*) malloc(num_bin*sizeof(int));
        bin_offset[0] = 0;
        for(int i = 1; i < num_bin; i++)
            bin_offset[i] = bin_offset[i-1] + bin_size[i-1];

        // find the left most position of each bin
        int* left_most = (int*) malloc(num_bin*sizeof(int));
        for(int i = 0; i < num_bin; i++)
            left_most[i] = 2147483647u;
        for(int i = 0; i < len; i++)
        {
            int j;
            for(j = 0; prm[i] >= bin_offset[j] && j < num_bin; j++)
                continue;
            j--;
            if(prm[i] < left_most[j])
                left_most[j] = prm[i];
        }

        // set prm[i] to point to the left most in each bin
        for(int i = 0; i < len; i++)
        {
            int j;
            for(j = 0; prm[i] >= bin_offset[j] && j < num_bin; j++)
                continue;
            j--;
            prm[i] = left_most[j];
        }

        // initialize count
        for(int i = 0; i < len; i++)
            count[i] = 0;

        // initialize b2h
        for(int i = 0; i < len; i++)
            b2h[i] = 0;

        // scan bucket
        int l = 0, r = 0, n = 0;
        while(l < len)
        {
            // find left and right boundary of a bucket
            for(r = l + 1; r < len; r++)
            {
                if(bh[r] == 1)
                {
                    r = r -1;
                    break;
                }
            }
            if(r >= len)
                r = r - 1;
            // printf("l: %d, r: %d\n", l, r);

            // increment count, set prm, set b2h
            for(int i = l; i <= r; i++)
            {
                int Ti = pos[i] - H;
                if(Ti >= 0)
                {
                    count[prm[Ti]]++;
                    prm[Ti] += count[prm[Ti]] - 1;
                    b2h[prm[Ti]] = 1;
                    // printf("setting b2h[%d] to 1\n", prm[Ti]);
                }
            }

            // update b2h
            for(int i = l; i <= r; i++)
            {
                int Ti = pos[i] - H;
                if(Ti >= 0)
                {
                    if(b2h[prm[Ti]] == 1)
                    {
                        int j;
                        for(j = prm[Ti] + 1; bh[j]==0 && b2h[j]==1; j++)
                            continue;
                        for(int k = prm[Ti] + 1; k < j; k++)
                        {
                            // printf("setting b2h[%d] to 0\n", k);
                            b2h[k] = 0;
                        }
                    }
                }
            }

            // update left side boundary
            l = r + 1;
            n++;
        }

        // update pos
        for(int i = 0; i < len; i++)
            pos[prm[i]] = i;

        // copy b2h to bh
        for(int i = 0; i < len; i++)
            bh[i] = b2h[i];
        bh[0] = 1;

        // free temp dynamically allocated memory
        free(bin_size);
        free(bin_offset);
        free(left_most);
    }

/* free dynamically allocated memory */

    free(prm);
    free(bh);
    free(b2h);
    free(count);

/* return suffix array */

    return pos;
}
