batcher
=======

(c) 1994 Dr. Andrew C.R. Martin
-------------------------------

batcher - Create a repetitive file

`batcher` is a simple command language for creating repetitive batch 
files, etc. Each command is introduced by a | character and variables 
may also be created using this character. The language has only 9 
basic commands:
   
```
   |SARRAY
   |SET
   |INC
   |DEC
   |STEP
   |REPEAT
   |FORMAT
   |FILE
   |END
   |EXIT
```
and two associated commands
```
   |STRING
   |NUMBER
```

The simplest of these is the `|REPEAT` `|END` pair. `|REPEAT` is followed
by a variable name and two values. This is formally equvalent to the
`DO` loop of FORTRAN or the `for()` loop of C. The `|END` command ends the
set of lines to be repeated.

Note that `|REPEAT` `|END` pairs may *NOT* be nested.

For example:
```   
   |REPEAT |I 1 5
   $WRITE SYS$OUTPUT "Hello |I "
   |END
   |EXIT
```

would create a VAX DCL batch file thus:
```
   $WRITE SYS$OUTPUT "Hello 1"
   $WRITE SYS$OUTPUT "Hello 2"
   $WRITE SYS$OUTPUT "Hello 3"
   $WRITE SYS$OUTPUT "Hello 4"
   $WRITE SYS$OUTPUT "Hello 5"
```   

Had the `|STEP` command been issued before the `|REPEAT` command, the 
variable would have been incremented by this value. The effect of 
`|STEP` is removed once the `|END` command is found.
e.g.

```
   |STEP 2
   |REPEAT |I 1 5
   $WRITE SYS$OUTPUT "Hello |I "
   |END
   |EXIT
```
would create a VAX DCL batch file thus:
```
   $WRITE SYS$OUTPUT "Hello 1"
   $WRITE SYS$OUTPUT "Hello 3"
   $WRITE SYS$OUTPUT "Hello 5"
```

The alternative to the `|REPEAT` `|END` pair is the `|FILE` `|END` pair. 
The effect of this is similar except that variables are read from
a file and the construct is repeated until the file ends. Before 
using `|FILE` `|END`, the `|FORMAT` command must be given to describe the
format of a line in the file and associate values with variables
e.g.
```
   |FORMAT |STRING |A |NUMBER |B |NUMBER |C
```
would expect each line of the file to contain a string followed by two
numbers. These would be assigned to the variables |A, |B and |C
resepctively.

Suppose a file exists called `batch.dat` as follows:
```
   A 53 2
   B 47 5
   C 20 15
```
The following commands would be used to create a batch file:
```
   |FORMAT |STRING |A |NUMBER |B |NUMBER |C
   |FILE batch.dat
   $WRITE sys$output "String|A  Values |B , |C "
   |END
   |EXIT
```

This would result in the following batch file:
```
   $WRITE sys$output "StringA Values 53, 2"
   $WRITE sys$output "StringB Values 47, 5"
   $WRITE sys$output "StringC Values 20, 15"
```

The `|SARRAY` command is used to create a string array:
```
   |SARRAY |MyArray
   Hello
   Goodbye World!
   |END
```

Items in the array are numbered from 1 and accessed with a number or 
numeric variable:
```
   |MyArray |a
   |MyArray 2
```

Usage:
------

```
batcher [ -i inputfile ...] outputFile
```

