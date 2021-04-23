#include <rtthread.h>
#include <gan_zhi.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
#else
#include <stdlib.h>
#include <stdio.h>
#endif

static const char *usage = "        \n\
Welcome in showgan_zhi.             \n\
Usage:                              \n\
    showgan_zhi year month day hour \n\
e.g:                                \n\
    showgan_zhi 2021 4 23 15        \n\
\n";

static int showgan_zhi(int argc, char const *argv[])
{
    if (argc == 5)
    {
        char str[32] = {'\0'};
        if (0 == sun_hour2gan_zhi(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), str))
        {
            printf("%s\n", str);
        }
        else
        {
            printf("输入日期或时有误，请重新输入\n");
        }
    }
    else
    {
        printf("%s\n", usage);
    }
    return 0;
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(showgan_zhi, showgan_zhi generator: showgan_zhi[year][month][day][hour]);
#endif /* FINSH_USING_MSH */
