#include "../include/function.h"
int sha512(char *newkey,char *key,char *salt)
{
    char *p = crypt(key,salt);
    strcpy(newkey,p);
    return 0;
}
int salt(char *str,int size)
{
    for(int i = 0; i < size; i++)
    {
        int tmp =  rand()%64;
        if(tmp < 12)
        {
            str[i] = 46+tmp;
        }
        else if(tmp < 12 + 26)
        {
            str[i] = 65 + tmp -12;
        }
        else
        {
            str[i] = 97 + tmp -12 -26;
        }
    }
    str[size] = '\0';
    return 0;
}

