# puzzle
A program to solve "WordBrain" puzzle word grids

usage: puzzle height width letters hint [ hint ... ]
    height & width are the dimensions of the grid
    letters are the letters that form the grid in left-to-right, top-to-bottom order
    hint is the word hint, either a number describing the word length, a full word or a partial word right padded with spaces
    
eg. to solve a puzzle grid like this

  o e t h i
  s n d c r
  a o r o c
  f e k e s
  m l v o h

You'd enter...
    
    $ puzzle 5 5 oethisndcraorocfekesmlvoh 4 4 6 6 5
    
If you knew that the 3rd word started with the letters "sh" then you'd enter..

    $ puzzle 5 5 oethisndcraorocfekesmlvoh 4 4 "sh    " 6 5

Notice how the word is space-padded to fill it out to the right sized word. spaces anywhere else in the word are invalid, i.e.
"s h   " and "   sh " would be invalud and unpredictable results will be obtained.

if you know a complete word then you'd simply type the whole word out as a hint, i.e.

    $ puzzle 5 5 oethisndcraorocfekesmlvoh 4 4 shovel 6 5
    
Good Puzzling!!! :o)
