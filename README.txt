CS 485g Systems Programming Project 4
Name: Christopher Will
Files: nsh.h nsh.cpp README.txt Makefile

Description of the project:
The goal of this project was to emulate a shell, which we call new shell, or nsh, by getting input from the user
and using fork and exec to execute their given files. This further mimicked a shell by letting the user set variables
with values which they could then reference later by doing $variableName in the command prompt. The user's input was read in,
and tokenized such that a bare word, or a string of words in matching double quotes, counted as a single token. Comments were
represented by the % character, with the convention being that anything after a comment is ignored by the shell. 

Descriptions of algorithms used for each command:
For all commands the first thing I did was to read in each char of the user's input into a char vector. Then loop over
each char in this vector, looking for comments. If I found a comment then I deleted the %, and every character after the 
% from the vector. The next task was to tokenize this comment-less input. A valid token is either a single word, or a collection
of words surrounded by double quotes. So to tokenize the input I looped over each char in vector (which now has no comments in it),
and looked for either a double quote, or a non white space character. If I found a double quote then I stored each char after the quote
in a string until I found a matching double quote, or reach the end of the input. And if I found a non white space character then I added 
each char to a string until I reached the end of the input, or found whitespace. Dealing with the tokens in quotes was more nuanced, as I had 
to decide how to handle input which had an opening double quote, but no matching quote at the end. I decided to treat this as an error, so 
entering something like: "hello world, will throw an error as there is no matching quote. I throw the error and proceed to read in the next line
of input. The final thing to do was to expand any variables that might be in the user's input. This was as a simple as looping over each char in each
token and looking for the $ char. If I found this character then I stored every char after it, until I found whitespace or the end of input, and then
looked in my map to see if this variable name mapped to a value. If it did then I replaced the $variable with its value, else I gave an error message
that the variable had not yet been defined.


For all commands I also make sure that the correct number of tokens is given. This is done easily by comapring the size of my eachToken vector
with the expected number of tokens for a command

1. set
I first make sure the variable name begins with a letter, and is followed by only either numbers or letters. This is done with a for loop
and the isalpha() and isalnum() routines. If the name is valid then I just it so that the variable name maps to the given value, using my
map data structure.

2. prompt
Assuming that 2 tokens were given, I simply set the prompt variable to be equal to the 2nd token the user entered

3. dir
I just call chdir() on the 2nd token the user gave. If an error occurs then it will be caught and displayed using perror

4. procs
I use a map data structure to map the pid of a procss to the path of the file being executed. In my function where I call
execv() I just store the pid, and then when the user enters procs I can look at the status of this pid and give a relevant
message depending on whether it is still running or not

5. done
If the user enters done then I quit the program. As a convention I don't care if they enter anything after the "done". So long
as they enter "done" I quit the shell. 

6. do
I first check whether the file given, which will be the 2nd token, is in the current path. This is done by using the
":" in the PATH variable as the delimiter, and checking whether currentPath + userFile exists. And if it doesn't then
I keep checking until I have looped over each path in PATH and at that point return false as the given file must not
exist or be in the current path. If the file does exist in the path then I simply add each token to a vector and call
execv() with the path to the file and that token as arguments. Then I call waitpid() in the parent to wait for the child to return
before reading from stdin again

7. back
do is just like back, except I don't call waitpid(). This allows the program to run in the background and the user to immediately
send more input to the shell

8. tovar
For tovar I first validate that the variable name given is of the correct form. Then the only difference between this command 
and the do command is that I open a file in /tmp and redirect stdout in the child process to this /tmp file.
I can then open this file and read its contents into the variable given before closing the file and wiping its contents.


Special features/known bugs:
The "done" command will work even if more tokens come after "done". So entering: done foo bar, would not throw an error
as my program would see that the 1st token is "done" and immediately quit the program. This is similar to what bash does
when you type "exit". That is, in bash you may type: exit foo bar, and the foo and bar will be ignored. The shell will still exit
despite extraneous tokens being given.

I treat any double quote given without a matching quote as an error. I give an error message and prompt for the next command.
So giving: prompt "hello world " "foo, would be an error. As would: prompt ".  

I don't know of any bugs present in my program. 
