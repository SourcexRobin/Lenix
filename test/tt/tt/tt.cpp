// tt.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "stdio.h"

#define FS_IS_PREFIX(c)             ( (c) == ' ' || (c) == '/' )
#define FS_TRIM_PREFIX(p)           do{ while(*(p) && FS_IS_PREFIX(*(p))) \
                                      (p)++;}while(0);
#define FS_TRIM_SPACE(p)            do{ while(*(p) && *(p) == ' ') (p)++; \
                                      }while(0)
#define FS_TRIM_PATH_SEPARATOR(p)   do{ while(*(p) && *(p) == '/') (p)++; \
                                      }while(0)
const char* Fs_get_subpath(const char * path,char * subpath)
{
    const char * p          = path;
    char       * sp         = subpath;
    char       * lastspace  = NULL;
    
    FS_TRIM_PATH_SEPARATOR(p);
    while( *p && *p != '/' )
    {
        if( *p == ' ' )
        {
            if( lastspace == NULL )
                lastspace = sp;
        }
        else
            lastspace = NULL;
        *sp++ = *p++;
    }
    *sp = 0;
    if( lastspace )
        *lastspace = 0;
    
    return 0 == *p ? NULL : p;
}

int         _strmatch   (const char * str,const char * mode)
{
    const char * m = mode;

    while( *m && *str)
    {
        switch(*mode)
        {
        case '*':
            while( *m && (*m == '*' || *mode == '?') )
                m++;
            if( *m == 0 )
                return 1;
            for( ; *m != *str && *str ; str++ )
                ;
            m++;
            break;
        case '?':
            /*  跳过一个字符*/
            m++;
            str++;
            break;
        default:
            if( *m != *str )
            break;
        }
    }

    return *mode == 0 ? 1 : 0 ;
}

char * str[16] = {
    "fdsajkl;fdst",
    "clapkdnci",
    "fhulskmcj",
    "hiojklsd125789",
    "fdsajk23ddst",
    "fdsajidsf823yuif",
    "vjhkukoaskld",
    "vchuinsdjn",
    "fdsajklfd;fdst",
    "fhckjsd",
    "jhklpiomvcbuio",
    "cvy89jkqwjnkcji",
    "fdsajkl;ffdsfadsdst",
    "cxvjhioqwaklncj",
    "vbhjjiowejkld",
    "jfhdkjklafjkl",
};
int _tmain(int argc, _TCHAR* argv[])
{
    int i = 0; 
    char * mode = "*a*aj*";
    char * test = "78akaakakaddajc";
    for( i = 0 ; i < 16 ; i++)
    {
        if( _strmatch(str[i],mode) )
            printf("string match!\t");
        else
            printf("string not match!\t");
        printf("mode: %s  str: %s\n",mode,str[i]);
    }

	return 0;
}

