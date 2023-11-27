#include <stdio.h>

int main()
{
    char buff[1024] = {0};
    char wbuff[1024] = {0};
    FILE *fp = NULL;
    FILE *fpw = NULL;

    fp = fopen("testfile2.doc", "rb");
    fpw = fopen("testfile2_dl.doc", "wb");
    int n = 100;
    int status = 1;
    while (status)
    {
        status = fread(buff, 1, 1024, fp);
        printf("\nStatus = %d\n", status);
        for (int j = 0; j < status; j++)
        {

            printf("%c", buff[j]);
        }
        fwrite(buff, 1, status, fpw);
    }
    // n = 10;
    // while (n--)
    // {
    //     fwrite(wbuff, 10, 1)
    // }
}