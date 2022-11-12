#include <stdio.h>
#include <unistd.h>

int main()
{
    char buf[100];
    getcwd(buf,100);
    printf("%s\n",buf);

    
}