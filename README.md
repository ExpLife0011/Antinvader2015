# Antinvader2015

A minifilter driver source code on Windows, hack from https://github.com/Coxious/Antinvader, It's can work on VS 2015 now.

Now, default target platform is Windows 7 Desktop.

modified by: GuoXiongHui(wokss@163.com)

Antinvader
=============

## What is Antinvader

"Antinvader" is a project of a transparent encryptor running on Windows which I started several years ago (Say 2010 or even earlier? I do not quite remember...) and it has been continuously developed for years as a training project to improve my skills of Windows kernel programming. It is a File System Minifilter Driver, and it can already successfully encrypt some simple program like Notepad with xor 0x77 but still have many problems on more complex program...

As I become more and more busy and it seems no time for me to continue developing it, rather than keep it a secret and let it buried with time, I decided to publish it as a free software and hope it may help someone who is interested in Windows kernel development.

Also if you have interests to continue my work, I'll be glad that you fork this repository and improve it.

## How to use it

Use the inf file to load the driver then there is a demo program here showing how to control the driver. you may have a peek on it.

## About the language

At the time I starting writing this project , I'm a student just step in to an high school and my English is not very well. So there might be some spelling error and the comments were written in Chinese. If this project can help some of you may not know Chinese, I'll be glad to pay some time translating it.
