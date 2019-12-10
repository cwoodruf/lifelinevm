= Legacy C Application =

This is the original lifeline dialogic api C application. 
The basic backbone of the program is in the ansrvars.c file. 
This contains the basic map of states. State transitions are
handled via specialized C functions. Each state uses two 
functions: one to initiate the state - i.e. interact with 
the dialogic subsystem - and one to handle the state transitions
based on the response from the dialogic sub system.

As this was designed to work on "junk" computers of the time
(it was tested on a 286) it uses DOS as the operating system.
Needless to say, handling data entry, backing up the system
etc. required a great deal of custom work. Doing external system
backups was the most difficult part of the application to 
design as I did not have the luxury of turning everything off.
In order to avoid deadlocks - which were a problem early on - 
the backup does a quick and dirty dump of the on disk data.
This does not guarantee perfect consistency if a voice mail
user is changing their security code but ensures that the 
system won't get blocked.

Despite the fact that the C program might be considered "messy" it 
was, in practice, easy to extend given the flexible architecture of
the telephony subsystem. One major change made was to automate 
payment with the "paycode" subsystem. This simplified selling voice
mail to larger organizational customers.

This was only my 2nd C program and, as such, would have benefited
with some restructuring. I was about to do this work when asterisk 
came along. Being able to do the telephony without expensive 
custom hardware made a huge difference. The asterisk AGI system was
easy enough to use that I was able to basically replicate the C 
program in about 2 weeks. The business model was redesigned to use
a web interface that completely automated sales and billing for
organizations. 

