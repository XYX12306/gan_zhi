# gan_zhi

## 1、介绍

**gan_zhi** 是一个根据公历日期、时得到相应的天干地支的工具软件包。

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| examples | 例子目录 |
| inc  | 头文件目录 |
| src  | 源代码目录 |

### 1.2 许可证

`gan_zhi` package 遵循 `MIT` 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread 3.0+

## 2、如何打开 gan_zhi

使用 gan_zhi package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    tools packages --->
        [*] gan_zhi: A tool package to get tiangan and dizhi informations according to the date and time.
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 gan_zhi

在使用 gan_zhi 软件包时首先要定义一个`char`数组来存储返回年、月、日、时的天干地支，如：

```c
char str[32] = {'\0'};
```

调用的函数接口为：

```c
int get_lunar_year(int year, int month, int day);
int get_gan_zhi(int year, int month, int day, int hour);
signed char sun_hour2gan_zhi(int year, int month, int day, int hour, char *str_gan_zhi);
```

比如：

```c
static int showgan_zhi(int argc, char const *argv[]) {
    if (argc == 5) {
        char str[32] = {'\0'};
        sun_hour2gan_zhi(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), str);
        printf("%s\n", str);
    }
    return 0;
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(showgan_zhi, showgan_zhi generator: showgan_zhi[year][month][day][hour]);
#endif /* FINSH_USING_MSH */
```

在 MSH 中输入命令 `showgan_zhi 2021 4 23 15`，可以在串口助手上看到输出了距离最近的下一个节气信息。

```
msh />showgan_zhi 2021 4 23 15
辛丑年壬辰月辛丑日丙申时
```

在打开 gan_zhi package 示例后，当进行 bsp 编译时，它会被加入到 bsp 工程中进行编译。

## 4、注意事项

- 当前可以生成天干地支信息对应的阳历范围为 1900-1-1 到2099-12-31。
- 文件统一使用UTF-8编码。

## 5、联系方式 & 感谢

* 维护：[XYX12306](https://github.com/XYX12306)
* 主页：https://github.com/XYX12306/gan_zhi
