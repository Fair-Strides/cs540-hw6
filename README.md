# cs540-hw6

Database Management Systems Homework 6: Hash Indexing

## Introduction

In the previous assignment, we learned how to store data in external memories
using page structure. In this assignment, you'll learn to implement indexing,
specifically Hash indexing, which provides a desirable trade-off between the
time and I/O complexity of search and update/insertion/deletion operations over
a file on external storage.

## What we must do

Assume that we have a relation **Employee(id, name, bio, manager-id)**. The
values of **id** and **manager-id** are integers each with the fixed sizes
of 8 bytes. The values of **name** and **bio** are character strings and take
at most 200 and 500 bytes, respectively. _Note that as opposed to the values
of id and manager-id, the sizes of the values of name and bio are not fixed
and are between 1 to 200 (500) bytes_. **The size of each page is 4096 bytes
(4KB)**. The size of each record is less than the size of a page. Using the
provided skeleton code with this assignment, write a C++ program that creates
a hash index file for relation Employee using attribute id. Your program must
also enable users to search the created index by providing the id of a record.

- **The Input File:** The input relation is stored in a CSV file,
  i.e., each tuple is in a separate line and the fields of each record are
  separated by commas. Your program must assume that the input CSV file is in the
  current working directory, i.e., the one from which your program is running,
  and its name is **Employee.csv**. We have included an input CSV file with this
  assignment as a sample test case for your program. Your program must correctly
  create hash indexes and search for id(s) using them for other CSV files with the
  same fields as the sample file.

- **Index Page Creation:** Your program must first read the input **Employee**
  relation and build a _basic hash index for the relation using attribute id_.
  Your program must store the hash index in a file, **EmployeeIndex.dat** (Binary
  file like the previous assignment) on the current working directory. **Your index
  file must be a binary data file rather than text / csv**. You must use one of the
  methods explained in our lectures on storage management for storing variable-length
  records and the method described for storing pages of variable-length records to
  store records and pages in the new data file. They are also explained in Sections
  9.7.2 and 9.6.2 of the Cow Book, respectively. If your submitted program does not
  use these formats and page data structures to store data in the data file, it does
  not get any points.

- **Index File and Page Structure:** In each page, records are written from top
  to bottom, and the overflow page index is written at the end. If there's no
  overflow page, you should write -1. The slot directory is written to the end
  of the page before the overflow page index in reverse order. This arrangement
  allows you to follow the i-th record's offset and length in the i-th entry of
  the slot directory from the bottom of the page. Indexes and their positions are
  tracked using pageDirectory, and positions are assigned to the pages using
  nextFreePosition. You must use the hash function, h = id mod 2^8.

- **Searching the Data File:** After creating the hash-indexed file, your
  program must accept a list of Employee id values in its command line and search
  the file for all records with the given _id(s)_. As you have stored the data using
  page structures and indexed those using Hash-indexing, you must compute the hash
  value and go to the specific index of the page that may contain the desired
  record. Then you should use the slot directory information to navigate to each
  record inside the page structure and find the desired id. 

  Approaches that do not utilize hash-indexing and page data structure but rather
  read pages sequentially or read the data file line by line, like the previous
  assignments, will not get any points.

  The user of your program should be able to search for records of multiple ids,
  one id at a time (see the provided _main.cpp_ below).

- **Main Memory Limitation:** During the file creation and search, your program
  must keep up to one page plus the directory of the hash index in main memory
  at any time. The submitted solutions that use more main memory will not get any
  points.

- **Compiling your Code:** your code must compile on the Hadoop server (see
  Reference: Hadoop Server). Use the filename "main.out" when compiling.

## Submission

The assignment is to be turned in before Midnight (by 11:59pm) on May 27th. You
may turn in the source code of your program through Canvas. The assignment may
be done in groups of two students. Each group may submit only one file that
contains the full name, OSU email, and ONID of every member of the group. Code
must be submitted as a .zip file.

## Grading criteria

The programs that implement the correct algorithm, return correct answers, and
do not use more than the allowed buffers will get a perfect score.

The ones that use more buffer than allowed will not get any points. The ones
that implement the right algorithm but return partially correct answers will
get partial scores.

- 12 pts _Full marks_
- 10 pts _Minor error_
  Lookup did not return all the IDs we were searching for.
- 6 pts _Partial credit_
  EmployeeIndex created properly but Lookup implementation did not match
  assignment requirement.
- 4 pts _Major error_
  Your program does not return any queries correctly. Your EmployeeIndex.dat
  file structure shows that you are reading the EmployeeIndex file
  sequentially for Lookup. It defeats the purpose Indexing the CSV file.
- 0 pts _No marks_
  You did not meet the assignment requirements.
