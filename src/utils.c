/* compare only first characters */
int strhcmp(const char *str1, const char *str2)
{
    while(*str1 && * str2)
        if(*str1++ != *str2++) return 0;

    return 1;
}

