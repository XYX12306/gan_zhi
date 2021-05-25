/**
 * The MIT License
 *
 * Copyright (c) 2019 Illusion Lee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>
#include "gan_zhi.h"

/* 同时需要调用solar_terms软件包中的获取节气函数 */
extern int get_solar_terms(int year, int month);

/* 当年2月末日的日干支序数表 nianshu_table[0]的bit[23:16]数据对应的起始年份 */
#define NIANSHU_TABLE_START_YEAR 1900
/* 春节离当年元旦的天数表 lunar_calendar_part_table[0]的bit[23:16]数据对应的起始年份 */
#define LUNAR_CALENDAR_PART_TABLE_START_YEAR 1900

/* 十天干名称 */
const char *tian_gan_const[] = {"甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"};
/* 十二地支名称 */
const char *di_zhi_const[] = {"子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"};
/* 前一个月的总天数 */
const unsigned int month_sum_table[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
/* 日数表，使用的是任红岩作者的推算 */
const unsigned char rishu_table[] = {6, 37, 0, 31, 1, 32, 2, 33, 4, 34, 5, 35};
/* 年数表，当年2月末日的日干支序数，使用的是任红岩作者的推算 */
const unsigned int nianshu_table[] =
    {
        /*1900-1929*/
        0x090e13, 0x181e23, 0x282d33, 0x380106, 0x0c1116, 0x1b2126, 0x2b3036, 0x3b0409, 0x0f1419, 0x1e2429,
        /*1930-1959*/
        0x2e3339, 0x02070c, 0x12171c, 0x21272c, 0x313600, 0x050a0f, 0x151a1f, 0x242a2f, 0x343903, 0x080d12,
        /*1960-1989*/
        0x181d22, 0x272d32, 0x370006, 0x0b1015, 0x1b2025, 0x2a3035, 0x3a0309, 0x0e1318, 0x1e2328, 0x2d3338,
        /*1990-2019*/
        0x01060c, 0x11161b, 0x21262b, 0x30363b, 0x04090f, 0x14191e, 0x24292e, 0x333902, 0x070c12, 0x171c21,
        /*2020-2049*/
        0x272c31, 0x360005, 0x0a0f15, 0x1a1f24, 0x2a2f34, 0x390308, 0x0d1218, 0x1d2227, 0x2d3237, 0x00060b,
        /*2050-2079*/
        0x10151b, 0x20252a, 0x30353a, 0x03090e, 0x13181e, 0x23282d, 0x333801, 0x060c11, 0x161b21, 0x262b30,
        /*2080-2100*/
        0x363b04, 0x090f14, 0x191e24, 0x292e33, 0x390207, 0x0c1217, 0x1c2126};
/* lunar_calendar_part_table[]使用了lunar_calendar软件包lunar_calendar_table[]里关于春节离当年元旦的天数的数据 */
unsigned int lunar_calendar_part_table[] =
    {
        /*1900-1929*/
        0xBF5348, 0xBD5044, 0xB94D42, 0xB64ABE, 0x5246BA, 0x4E4337, 0x4BC154, 0x48BC50, 0x45B84D, 0x42B74A,
        /*1930-1959*/
        0x3E5146, 0xBA4E44, 0x384BBF, 0x53483B, 0x4F45B9, 0x4D42B6, 0x4ABD51, 0x46BB4E, 0x43B84C, 0x3F5248,
        /*1960-1989*/
        0x3C4F45, 0x394D42, 0x3549BE, 0x5146BB, 0x4F43B7, 0x4BBF52, 0x473C50, 0x45B94D, 0xC25449, 0x3D5146,
        /*1990-2019*/
        0xBB4F44, 0x374ABF, 0x5347BC, 0x5045B8, 0x4C41B6, 0x49BD52, 0x47BA4E, 0x43374A, 0xBF5348, 0xBC5045,
        /*2020-2049*/
        0xB94C41, 0xB64A3D, 0x51463A, 0x4D4337, 0x4BBF53, 0x48BC4F, 0x44384C, 0x41B64A, 0xBE5146, 0xBA4E42,
        /*2050-2079*/
        0x374BC1, 0x53483C, 0x4F4438, 0x4C4235, 0x49BD51, 0x45BA4E, 0x43B74B, 0xBF5347, 0x3B4F45, 0x384C42,
        /*2080-2100*/
        0xB649BD, 0x5146BA, 0x4E4338, 0x4A3E52, 0x473B4F, 0x45B94C, 0x41B549};
/**
 * 公历年月日转换为当日所在的农历年份。
 *
 * @param year 公历年份 [1900-2099]。
 * @param month 公历月份 [1-12]。
 * @param day 公历日 [1-31]。
 *
 * @return -1 失败；其它：当日所在的农历年份。
 * 注：该函数参考了lunar_calendar软件包的lunar_calendar()函数中关于获取当日所在的农历年份的计算
 */
int get_lunar_year(int year, int month, int day)
{
    int lunar_newyear; /* 春节离当年元旦的天数 */
    int sun_newyear;   /* 阳历日离当年元旦的天数 */
    unsigned char month_days_max;

    if ((year < LUNAR_CALENDAR_PART_TABLE_START_YEAR) || (year > (LUNAR_CALENDAR_PART_TABLE_START_YEAR + sizeof(lunar_calendar_part_table) / sizeof(int) * 3) - 2) || month < 1 || month > 12)
        return -1;
    /* 查找当月的最大天数 */
    month_days_max = ((month == 2) && (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))) ? (month_sum_table[month] - month_sum_table[month - 1] + 1) : (month_sum_table[month] - month_sum_table[month - 1]);
    /* 判断输入的日是否合法 */
    if (month_days_max - day < 0)
        return -1;
    if ((((lunar_calendar_part_table[(unsigned char)((year - LUNAR_CALENDAR_PART_TABLE_START_YEAR) / 3)] >> (16 - (((year - LUNAR_CALENDAR_PART_TABLE_START_YEAR) % 3) << 3))) & 0x60) >> 5) == 1)
    {
        lunar_newyear = ((lunar_calendar_part_table[(unsigned char)((year - LUNAR_CALENDAR_PART_TABLE_START_YEAR) / 3)] >> (16 - (((year - LUNAR_CALENDAR_PART_TABLE_START_YEAR) % 3) << 3))) & 0x1F) - 1;
    }
    else
    {
        lunar_newyear = ((lunar_calendar_part_table[(unsigned char)((year - LUNAR_CALENDAR_PART_TABLE_START_YEAR) / 3)] >> (16 - (((year - LUNAR_CALENDAR_PART_TABLE_START_YEAR) % 3) << 3))) & 0x1F) - 1 + 31;
    }
    sun_newyear = month_sum_table[month - 1] + day - 1;
    if ((!(year % 4)) && (month > 2))
        sun_newyear++;
    /* 判断阳历日在春节前还是春节后 */
    if (sun_newyear < lunar_newyear)
    {
        year--;
    }
    return year;
}

/**
 * 公历年月日时转换相对应的天干地支。
 *
 * @param year 公历年份 [1900-2099]。
 * @param month 公历月份 [1-12]。
 * @param day 公历日 [1-31]。
 * @param hour 时 [0-23]。
 *
 * @return -1 失败；其它：*tian_gan_const[]、*di_zhi_const[]序号。
 *                  bit[31:28]：年天干；bit[27:24]：年地支；bit[23:20]：月天干；bit[19:16]：月地支；
 *                  bit[15:12]：日天干；bit[11:8]：日地支；bit[7:4]：时天干；bit[3:0]：时地支。
 */
int get_gan_zhi(int year, int month, int day, int hour)
{
    int lunar_year, month_solar_terms, solar_terms_year_tian, gan_zhi_day, gan_zhi_hour, gan_zhi_result = 0;
    unsigned char month_days_max;

    if ((year < NIANSHU_TABLE_START_YEAR) || (year > (NIANSHU_TABLE_START_YEAR + sizeof(nianshu_table) / sizeof(int) * 3) - 2) || month < 1 || month > 12 || hour < 0 || hour > 23)
        return -1;
    /* 查找当月的最大天数 */
    month_days_max = ((month == 2) && (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))) ? (month_sum_table[month] - month_sum_table[month - 1] + 1) : (month_sum_table[month] - month_sum_table[month - 1]);
    /* 判断输入的日是否合法 */
    if (month_days_max - day < 0)
        return -1;
    /* 获得当天所在的农历年 */
    lunar_year = get_lunar_year(year, month, day);
    /* 年的天干地支 */
    gan_zhi_result = ((((lunar_year % 10) + 6) % 10) << 28) & 0xF0000000;
    gan_zhi_result |= ((((lunar_year % 12) + 8) % 12) << 24) & 0x0F000000;
    /* 月的天干地支 */
    if (day < (unsigned char)(get_solar_terms(year, month) >> 16))
    {
        month_solar_terms = ((month - 1 + 10) % 12) + 1;
    }
    else
    {
        month_solar_terms = ((month - 1 + 11) % 12) + 1;
    }
    /* 判断立春是在农历新年前后 */
    if ((month_solar_terms == 1) && (lunar_year < year))
    {
        solar_terms_year_tian = (year + 6) % 10;
    }
    else
    {
        solar_terms_year_tian = ((lunar_year % 10) + 6) % 10;
    }
    switch (solar_terms_year_tian)
    {
    case 0:
    case 5:
        gan_zhi_result |= (((month_solar_terms - 1 + 2) % 10) << 20) & 0x00F00000;
        break;
    case 1:
    case 6:
        gan_zhi_result |= (((month_solar_terms - 1 + 4) % 10) << 20) & 0x00F00000;
        break;
    case 2:
    case 7:
        gan_zhi_result |= (((month_solar_terms - 1 + 6) % 10) << 20) & 0x00F00000;
        break;
    case 3:
    case 8:
        gan_zhi_result |= (((month_solar_terms - 1 + 8) % 10) << 20) & 0x00F00000;
        break;
    case 4:
    case 9:
        gan_zhi_result |= (((month_solar_terms - 1) % 10) << 20) & 0x00F00000;
        break;
    default:
        break;
    }
    gan_zhi_result |= (((month_solar_terms - 1 + 2) % 12) << 16) & 0x000F0000;
    /* 日的天干地支 */
    if (month == 1 || month == 2)
    {
        year--;
    }
    gan_zhi_day = (unsigned char)(nianshu_table[(unsigned char)((year - NIANSHU_TABLE_START_YEAR) / 3)] >> (16 - (((year - NIANSHU_TABLE_START_YEAR) % 3) << 3))) + rishu_table[month - 1] + day;
    if (gan_zhi_day > 60)
    {
        gan_zhi_day -= 60;
    }
    year++;
    gan_zhi_result |= (((gan_zhi_day + 9) % 10) << 12) & 0x0000F000;
    gan_zhi_result |= (((gan_zhi_day + 11) % 12) << 8) & 0x00000F00;
    /* 时的天干地支 */
    switch (hour)
    {
    case 0:
    case 23:
        gan_zhi_hour = 0;
        break;
    default:
        gan_zhi_hour = ((hour % 2) == 0 ? (hour >> 1) : ((hour + 1) >> 1));
        break;
    }
    switch ((gan_zhi_day + 9 + ((hour < 23) ? 0 : 1)) % 10)
    {
    case 0:
    case 5:
        gan_zhi_result |= ((gan_zhi_hour % 10) << 4) & 0x000000F0;
        break;
    case 1:
    case 6:
        gan_zhi_result |= (((gan_zhi_hour + 2) % 10) << 4) & 0x000000F0;
        break;
    case 2:
    case 7:
        gan_zhi_result |= (((gan_zhi_hour + 4) % 10) << 4) & 0x000000F0;
        break;
    case 3:
    case 8:
        gan_zhi_result |= (((gan_zhi_hour + 6) % 10) << 4) & 0x000000F0;
        break;
    case 4:
    case 9:
        gan_zhi_result |= (((gan_zhi_hour + 8) % 10) << 4) & 0x000000F0;
        break;
    default:
        break;
    }
    gan_zhi_result |= gan_zhi_hour & 0x0000000F;
    /* bit[31:28]：年天干；bit[27:24]：年地支；bit[23:20]：月天干；bit[19:16]：月地支；
       bit[15:12]：日天干；bit[11:8]：日地支；bit[7:4]：时天干；bit[3:0]：时地支。*/
    return (gan_zhi_result);
}

/**
 * 对转换的天干地支解析。
 *
 * @param year 公历年份 [1900-2099]。
 * @param month 公历月份 [1-12]。
 * @param day 公历日 [1-31]。
 * @param hour 时 [0-23]。
 * @param str_gan_zhi 返回天干地支字符串。
 *
 * @return -1 失败；0：成功。
 */
signed char sun_hour2gan_zhi(int year, int month, int day, int hour, char *str_gan_zhi)
{
    unsigned int gan_zhi_result;
    unsigned char month_days_max;

    if ((year < NIANSHU_TABLE_START_YEAR) || (year > (NIANSHU_TABLE_START_YEAR + sizeof(nianshu_table) / sizeof(int) * 3) - 2) || month < 1 || month > 12 || hour < 0 || hour > 23)
        return -1;
    /* 查找当月的最大天数 */
    month_days_max = ((month == 2) && (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))) ? (month_sum_table[month] - month_sum_table[month - 1] + 1) : (month_sum_table[month] - month_sum_table[month - 1]);
    /* 判断输入的日是否合法 */
    if (month_days_max - day < 0)
        return -1;
    gan_zhi_result = get_gan_zhi(year, month, day, hour);

    strcat(str_gan_zhi, tian_gan_const[((gan_zhi_result >> 28) & 0x0F) % 10]);
    strcat(str_gan_zhi, di_zhi_const[((gan_zhi_result >> 24) & 0x0F) % 12]);
    strcat(str_gan_zhi, "年");
    strcat(str_gan_zhi, tian_gan_const[((gan_zhi_result >> 20) & 0x0F) % 10]);
    strcat(str_gan_zhi, di_zhi_const[((gan_zhi_result >> 16) & 0x0F) % 12]);
    strcat(str_gan_zhi, "月");
    strcat(str_gan_zhi, tian_gan_const[((gan_zhi_result >> 12) & 0x0F) % 10]);
    strcat(str_gan_zhi, di_zhi_const[((gan_zhi_result >> 8) & 0x0F) % 12]);
    strcat(str_gan_zhi, "日");
    strcat(str_gan_zhi, tian_gan_const[((gan_zhi_result >> 4) & 0x0F) % 10]);
    strcat(str_gan_zhi, di_zhi_const[(gan_zhi_result & 0x0F) % 12]);
    strcat(str_gan_zhi, "时");
    return 0;
}
