# led - a lightweight Line EDitor for Linux(R) and Lanos
This is a work licensed under the terms of GPLv2.<br>
*ABSOLUTELY NO WARRANTY!*<br>
Created by eXerigumo Clanjor (哆啦比猫/兰威举), 2010-2012.

## Have a try
### Clone
Clone this repo by using:

	git clone https://github.com/cjxgm/led.git

### Compile and run
You can build led by using:

	make

Or, just type:

	make debug

The latter one will build led and run it.

If you want to clean up the source code directory, you can try:

	make clean

or

	make cleanall

The latter will do the same thing as the former, but with executable
removed.

### Install
Just

	make install

### Uninstall
If you don't like led, you can uninstall it by:

	make uninstall

## How to use led?
Once you fire up led, you will see the led logo, then press any key
to get into the main interface.

In the main interface, you can use led commands to operate on the text.
### Commands
Commands are all one-character ones.
'+' means finished.

	c	+	create a new file
	l	+	load a file
	s	+	save to a file
	v	+	view all
	n	+	next line
	p	+	previous line
	f		find in the file
	a	+	append to current line
	d	+	delete current line
	q	+	quit
	h	+	head
	t	+	tail
	g		go to line

	?		help
	`		repeat

	<bs>+	<backspace>, remove the last character of current line
	<cr>+	<return>, new line next to current line.

### About the prompt
If you tried the command 'a', then you will see the prompt ':'.
There are 2 types of prompts in led. They are:

	:		You should input a string, then press RETURN key.
			You can also just press the RETURN key to cancel.
	>		You should input a integer number, then press RETURN key.
			You can also just press the RETURN key to cancel.


<!-- vim: ts=4 sw=4 sts=0 noet fenc=utf-8
-->

