# 42 Hackersprint x AnyDesk
## This repo is a fork from A Hackersprint organized in the 42Heilbronn on 11.122.2024 by the awesome team from AnyDesk.

The challenge was to build a simple pixel parser that looks for a certain pattern of pixelvalues in a .bmp file and extracts a string message from it.
The goal was to build the fastest executing algorithm and the incentive was to use the SIMD capabilities of the moderns cpus.
Eventhough didnt have enough time to read into was SIMD was, and also only used 1 thread, i got it working pretty fast, since we coded in C.
Right when the time was over, i was able to thimk about optimizing the prgram using a different search pattern and maybe even multithreading. I had several options in my mind but didnt come to implementing them.
Still it was a lot of fun, and i was happy to come in 4th in the final ranking. 

⬇️ this part was provided by the original repository.
# Example of using SIMD

```c
typedef struct
{
    u32 size;
    u32 capacity;
    const char *data;
} MyString;
/*
    Function that uses intrinsics to read 32 bytes by 32 bytes
    until it finds the newline character  - '\n' or the end of string ('\0')
    and advances the str pointer to that character.
*/
static char *get_next_line(MyString buffer, char *str)
{
    /* If we are on the newline character we try to find the next one. */
    if (*str == '\n' || *str == '\r')
        str = str + 1;
    /* We first load 32 bytes of newline characters into the Carriage
     * 256 bits register. */
    __m256i Carriage = _mm256_set1_epi8('\n'); 
    __m256i Zero = _mm256_set1_epi8(0);

    /* If the string is smaller than 32 bytes, we can't use the intrinsics,
        and load 32 bytes at a time. That's why we go byte by byte.       */
    while (buffer.size - (str - (char *)buffer.data) >= 32)
    {
        /* We then load 32 bytes from string into the Batch 256 bits register*/
        __m256i Batch = _mm256_loadu_si256((__m256i *)str); 

        /* We then check if there are any newline characters in the first
         * string by comparing 32 bytes at a time. The result */
        __m256i TestC = _mm256_cmpeq_epi8(Batch, Carriage);
        __m256i TestZ = _mm256_cmpeq_epi8(Batch, Zero);
        /* We check if either the '\n' character or '\0' character were found*/
        __m256i Test = _mm256_or_si256(TestC, TestZ); 

        /* We store the results of the check into a int,
            transforming the mask from 256 bits, into a 1bit mask */
        s32 Check = _mm256_movemask_epi8(Test);
        if (Check)
        {
            /* The _tzcnt_u32 func counts 
             * the numbers of zeros inside the parameter.
             * In our case it's going to count 
             * how many characters different than '\n' there are
             */

            s32 Advance = _tzcnt_u32(Check);
            str += Advance;
            if (*str == '\r')
                str++;
            return (str);
        }
        str += 32;
    }

    if (buffer.size - (str - (char *)buffer.data) < 32)
    {
        while (*str != '\n' && *str != '\0')
            str++;
    }
    if (*str == '\r')
        str++;
    return (str);
}

```
