# SimpleShell

## Control flow:  
  * Display the prompt sign “>” and take a string from user  
  * Parse the string into a program name and arguments  
  * Fork a child process  
  * Have the child process execute the program  
  * Wait until the child terminates  
  * Go to the first step  
  
## example:
  * clear
  * ls -l
  * cp a.txt b.txt
  * cat c.txt &
