# 0.1 ChangeLog

color format : 颜色格式，当前仅支持RGB565类型的CF_TRUE_COLOR,其他类型待开发

output format:  输出文件格式，当前仅支持C array

intput file path：输入图片路径，当前没有做错误检查，只测试了bmp,jpg,jpeg,png

output file path：输出文件路径，输出文件名、C文件的变量名都与输入文件保持一致，所以建议不要放同名的不同类型的图片，如black.png和black.jpg不是同一个文件，但是转换后会只有一个文件

e.g.:

xx\xx\black.png会输出xx\xx\black.c，变量名也为black



# 0.2 ChangeLog

1.新增CF_TRUE_COLOR_ALPHA颜色格式修改默认格式为CF_TRUE_COLOR_ALPHA

2.减小文件大小
