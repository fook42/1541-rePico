   10 dev=peek(186)
   20 print"menu mode: ";
   30 if peek(57344)=15 then print"vic20":load"v",dev
   40 if peek(44)=16 and peek(52)<64 then print"c16":load"c",8
   50 if peek(44)=16 then print"plus4":load"+",8
   60 if peek(678)=0 then print"c64 ntsc":load"n",dev
   70 print"c64 pal":load"p",dev
