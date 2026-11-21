   10 dev=peek(186)
   20 print"menu mode: ";
   30 if peek(57344)=15 then print"vic20":load"vic",dev
   40 if peek(44)=16 and peek(52)<65 then print"c16":load"c16",8
   50 if peek(44)=16 then print"plus4":load"+4",8
   60 poke 53311,1:poke 53248,1:if peek(53312)<>1 then poke 53311,0: print"dtv":load"dtv",dev
   70 print"c64":load"c64",dev
