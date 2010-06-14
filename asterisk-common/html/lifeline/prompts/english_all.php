<?php
$prompts = array(
	"ll-en-noprompt.gsm" => "
Error: no prompt is available for this feature. 
Please call your system administrator.
",
	"ll-en-getbox.gsm" => "
(pause 2 secs)
Please enter a four digit box or extension number now.
(pause 6 secs)
Press the star key to listen to this message again.
(pause 6 secs)
",
	"ll-en-greeting.gsm" => "
Please leave a message after the tone or 
enter your four-digit security code now to access your box.
(Note: this is the default greeting. Users may record their own greetings.)
",
	"ll-en-hangup.gsm" => "
Thank you for calling. Please call again later. Good bye.
(pause 3 secs)
",
	"ll-en-invalid.gsm" => "
That number is not valid please try again.
",
	"ll-en-reenter.gsm" => "
Re-enter your four digit security code now to access your box.
(pause 3 secs)
",
	"ll-en-inuse.gsm" => "
That box is in use right now please try again later.
",
	"ll-en-full.gsm" => "
That box is full please try again later. 
To retrieve your messages enter your four digit security code now.
(pause 3 secs)
",
	"ll-en-page.gsm" => "
This is an automated call from your voice mail service.
You have new messages. 
Press a key now to access your voice mail box and listen to your messages.
(pause 3 secs)
",
	"ll-en-askpage.gsm" => "
The paging feature allows you to enter a local phone number so that 
you can be notified when someone has left you a message. Paging tries 
to contact you as soon as the message is left and will continue trying 
every hour for the next 10 hours following that message.
To set up paging enter the 10 digit telephone number to be used to contact
you now.
To clear your existing paging number and cancel paging press the number 
sign now.
To go back to the main menu without making any changes press the star key 
now.
(pause 3 secs)
",
	"ll-en-confpage.gsm" => "
To activate paging to a voice line press 1 now.
To activate paging to a numeric pager press 2 now.
To deactivate the paging feature press the number sign now.
To go back to the menu without changing your paging setup
press the star key now.
(pause 3 secs)
",
	"ll-en-pageon.gsm" => "
Paging is now activated.
",
	"ll-en-pgid.gsm" => "
When the voice mail system pages you your numeric pager will display 
(play the system phone number)
",
	"ll-en-pgrpt.gsm" => "
To repeat the voice mail system call back number press any key
(play the system phone number)
",
	"ll-en-rmpage.gsm" => "
Paging is now deactivated.
",
	"ll-en-remind.gsm" => "
Your voice mailbox subscription is almost up. 
Your voice mail box will be deactivated within 7 days.
Please contact your voice mail service provider as soon as possible. 
",
	"ll-en-pressany.gsm" => "
Press any key to skip this message.
",
	"ll-en-geninstr.gsm" => "
To listen to your messages press 1 now.
To listen to your greeting press 2 now.
To record your greeting press 3 now and start recording after the beep. 
Press any key when you are finished recording.
To add time to your voice mail subscription or to find out when your 
subscription expires press 4 now.
To change the language for this box press 6 now.
To find out more about the paging feature press 8 now.
To change your security code press the number sign now 
and enter your new four digit security code.
To go back to the start press the star key now.
To repeat these instructions press zero.
(pause 5 secs)
",
	"ll-en-listen.gsm" => "
To hear the next message press 1 now.
To hear the previous message press 2 now.
To repeat the current message press 3 now.
To go to the first message press 4 now.
To go to the last message press 5 now.
To delete or restore this message press 7 now.
To send the current message to another box on this voice mail 
system press 8 now.
To delete or restore all of your messages press 9 now.
To listen to the date and time this message was recorded press the
number sign now.
To go back to the previous menu press the star key now.
To repeat these instructions press zero.
(pause 5 secs)
",
	"ll-en-nomsgs.gsm" => "
You have no messages.
",
	"ll-en-newmsgs.gsm" => "
You have new messages.
",
	"ll-en-lastmsg.gsm" => "
You have no more messages.
",
	"ll-en-newpin.gsm" => "
Enter your new four digit security code now.
Press the star key four times to go back to the menu.
(pause 3 secs)
",
	"ll-en-delete.gsm" => "
All of your messages have been marked for deletion.
",
	"ll-en-restore.gsm" => "
All of your messages have been restored.
",
	"ll-en-delmsg.gsm" => "
Message marked for deletion.
",
	"ll-en-restmsg.gsm" => "
Message restored.
",
	"ll-en-wasdel.gsm" => "
The following message was marked for deletion.
",
	"ll-en-forward.gsm" => "
Forwarding a message will delete the message from this voice mail
box and make it appear in the messages of another box on this
voice mail system.
To forward the current message to another box on this voice mail 
system enter the four digit box number now.
To cancel forwarding press the number sign now.
To return to the menu without changing the forwarding status of the
current message press the star key now.
(pause 3 secs)
",
	"ll-en-conffwd.gsm" => "
If you want to send the current message to this box press 1 now.
To cancel forwarding of the current message press the number 
sign now.
To return to the menu without changing the forwarding status of the
current message press the star key now.
(pause 3 secs);
",
	"ll-en-willfwd.gsm" => "
The current message will be sent to box.
(plays box number)
",
	"ll-en-wontfwd.gsm" => "
Forwarding for the current message is off.
",
	"ll-en-fwdfull.gsm" => "
That box is full please try again later.
",
	"ll-en-grtintro.gsm" => "
This is your current outgoing greeting.
",
	"ll-en-recgrt.gsm" => "
Record your outgoing greeting after the beep 
and press any key when you are finished recording.
",
	"ll-en-getpay.gsm" => "
To add time to your box you must purchase voice mail payment code 
from your local voice mail service provider. 
Enter the sixteen digit payment code printed on your phone 
card now or press the star key to go back to the main menu.
(pause 6 seconds)
",
	"ll-en-paidto.gsm" => "
Your subscription is paid to the...
(Plays paid to date)
",
	"ll-en-newsub.gsm" => "
This is a new subscription.
(For when a subscriber does not have the paid to date set.)
",
	"ll-en-newpay.gsm" => "
Your subscription is now paid to the ...
(Plays paid to date)
",
	"ll-en-callinit.gsm" => "
After the beep wait for the dial tone then dial the number you wish to 
call.
",
	"ll-en-confrmpn.gsm" => "
Please reenter your security code now.
(Pause 5 secs)
",
	"ll-en-thanks.gsm" => "
Thank You. 
",
	"ll-en-lang.gsm" => "
To change the language that you will be served in press '1' now.
To go back to the menu press the star key now.
",
	"ll-en-nolang.gsm" => "
We are sorry. There are no additional languages available.
",
	"ll-en-admintro.gsm" => "
To listen to your messages press 1 now.
To listen to your greeting press 2 now.
To change your security code press the number sign now 
and enter your new four digit security code.
To record your greeting press 3 now and start recording after the beep. 
Press any key when you are finished recording.
To find out when your subscription expires press 4 now.
To change the language for this box press 6 now.
To use administrator functions press 7 now.
To find out more about the paging feature press 8 now.
To go back to the start press the star key now.
To repeat these instructions press zero.
(pause 5 secs)
",
	"ll-en-admin.gsm" => "
To change the properties of a box press 1 now.
To listen to the current reminder message press 2 now.
To record a new reminder message press 3 now.
To remind all boxes press 6 now.
Reminding all boxes will not change reminder status for individual boxes.
To enable or disable browsing press 8 now.
To go back to the previous menu press the star key now.
To repeat these instructions press zero.
(pause 5 secs)
",
	"ll-en-rembox.gsm" => "
To turn on box reminder enter box number now.
To remind all boxes press the number sign four times.
Reminding all boxes will not change reminder status for individual boxes.
Press the star key four times to go back to the menu.
(pause 3 secs)
",
	"ll-en-clrbox.gsm" => "                      
To turn off box reminder enter box number now.
If the reminder has been set for all boxes
Press the number sign four times to restore individual box reminders..
Press the star key four times to go back to the menu.
(pause 3 secs)
",
	"ll-en-rmallmnu.gsm" => "
To toggle the remind all boxes feature press 1 now.
Reminding all boxes will not change the reminder status for 
individual boxes.
To go back to the main menu press the star key now.
",
	"ll-en-rmallon.gsm" => "
Remind all boxes is now on.
",
	"ll-en-rmalloff.gsm" => "
Remind all boxes is now off.
Individual box reminders restored.
",
	"ll-en-playrmd.gsm" => "
This is your current reminder message.
",
	"ll-en-recrmd.gsm" => "
Record your reminder message after the beep 
and press any key when you are finished.
",
	"ll-en-togbrws.gsm" => "
To change the browsing state for this box press '1' now.
To go back to the previous menu press the star key now.
With browsing callers need only call this admin box to access the 
message boxes controlled by this admin box. Callers can leave messages 
similar to when dialing the message box directly. A menu explains options 
to callers at the end of each of the message box greetings.
(Pause 3 seconds)
",
	"ll-en-browse.gsm" => "
To leave a message press the number sign now.
When you have finished recording press the star key 
to go back to this menu.
To go to the next box press 1 now
To go to the previous box press 2 now.
To hear the current box greeting again press 3 now.
To go to the first box press 4 now.
To skip ahead 10 boxes press 5 now.
To skip back 10 boxes press 6 now.
To exit press the star key now or hang up.
(pause 5 seconds)
",
	"ll-en-brwson.gsm" => "
Browsing is now activated.
",
	"ll-en-brwsoff.gsm" => "
Browsing is now deactivated.
",
	"ll-en-delbox.gsm" => "
To clear a box enter the box number now. 
Press the star key four times to go back to the menu.
Clearing a box will remove the box greeting and all messages associated 
with that box as well as deleting that boxes security code. The box will 
not be available over the telephone until a new security code is assigned 
to the box.
(pause 3 secs)
",
	"ll-en-confirm.gsm" => "
Please re-enter the box number of the box you wish to clear.
Press the star key four times to go back to the menu without changing this
box.
(pause 3 secs)
",
	"ll-en-boxcode.gsm" => "
To change the security code for a box enter the box number now.
(pause 3 secs)
",
	"ll-en-chgcode.gsm" => "
Now enter the new security code.
Press the number sign four times to clear the security code for this box.
Press the star key four times to go back to the menu.
(pause 3 secs)
",
	"ll-en-syscode.gsm" => "
Please enter your system box access code now.
(pause 3 secs)
",
	"ll-en-sysmenu.gsm" => "
To change the properties of a box press 1 now.
To listen to the current system announcement press 2 now.
To record a new system announcement press 3 now.
To turn the system announcement on or off press 6 now.
To update boxday counts for active admin boxes press 4 now.
To listen to the system date or change the system time press 5 now.
To change the system access code press the number sign now.
To go back to the start press the star key now.
To repeat these instructions press zero.
(pause 5 secs)
",
	"ll-en-partbup.gsm" => "
Starting partial back up now.
",
	"ll-en-fullbup.gsm" => "
Starting full backup now.
",
	"ll-en-noannc.gsm" => "
You have not recorded an announcement.
",
	"ll-en-anncon.gsm" => "
The system announcement is now on.
",
	"ll-en-anncoff.gsm" => "
The system announcement is now off.
",
	"ll-en-actannc.gsm" => "
Please press 6 to turn the new announcement on.
",
	"ll-en-recannc.gsm" => "
Please record a new system announcement after the beep and press any
key when you are finished recording.
",
	"ll-en-hearannc.gsm" => "
This is your current system announcement.
",
	"ll-en-frcbdup.gsm" => "
Boxday counts for active admin boxes updated.
",
	"ll-en-newinstr.gsm" => "
To hear when the subscription for this box expires press 4 now.
To change the language for this box press 6 now.
To change the assigment range for this box or number of months 
automatically assigned by this box press 7 now.
To change or delete the autoassignment code for this box press the 
number sign now.
To go back to the start press the star key now.
To repeat these instructions press zero.
(pause 5 secs)
",
	"ll-en-newprops.gsm" => "
To hear the current settings for this autoassignment box press '1' now.
To restore the settings for this autoassignment box to their original 
values press '3' now.
To change the range of this autoassignment box press '4' now.
To change the free time of this autoassignment box press '7' now.
To clear the current settings for this autoassignment box press
the number sign now.
Press the star key to back to the previous menu.
(pause 3 secs)
",
	"ll-en-rstnew.gsm" => "
The autoassignment properties have been restored to their original values.
",
	"ll-en-range.gsm" => "
The current range is ...
(play first box)
",
	"ll-en-to.gsm" => "
to...
(play last box)
",
	"ll-en-norange.gsm" => "
There is no autoassignment range set.
",
	"ll-en-active.gsm" => "
Months new boxes will be active ...
(play free time)
",
	"ll-en-nomon.gsm" => "
no months.
",
	"ll-en-rngstart.gsm" => "
Please enter the start of the range of boxes that may be assigned by this
box.
Press the star key four times to go back to the menu without changing the
range.
Press the number sign four times to clear the current box range.
(pause 3 secs)
",
	"ll-en-rngend.gsm" => "
Now enter the end of the range.
(pause 3 seconds)
",
	"ll-en-freetime.gsm" => "
Press one digit to indicate the number of additional months to be 
automatically assigned at the time a subscription is started.
Press the star key to go back to the previous menu without changing this
value. 
(pause 3 seconds)
",
	"ll-en-clrprops.gsm" => "
Autoassignment properties for this box have been set to their defaults.
",
	"ll-en-boxprops.gsm" => "
To change box properties enter a four digit box number now.
Press the star key four times to go back to the previous menu. 
(pause 3 secs)
",
	"ll-en-boxprmnu.gsm" => "
To hear the box properties for this box press '1' now.
To change the properties for this box back to their original
values press '3' now.
To change the subscription date for this box press 4 now.
To change the reminder for this box press '6' now.
To change the admin box for this box press '7' now.
To change the box type for this box press '8' now.
To activate or deactivate this box press '9' now.
To change the security code for this box press the number sign now.
To go back to the previous menu press the star key now.
(pause 3 secs)
",
	"ll-en-playprop.gsm" => "
Properities for box ...
(plays box number)
",
	"ll-en-boxadmn.gsm" => "
Administrative box ...
(plays box number)
",
	"ll-en-oldprops.gsm" => "
The properties for this box have been reset to their orignal values.
",
	"ll-en-togrmd.gsm" => "
To change the reminder for this box press '1' now.
To clear the reminder press the number sign now.
Press the star key to go back to the menu.
(pause 3 secs)
",
	"ll-en-rmall.gsm" => "
This admin box is set to remind all associated message boxes.
",
	"ll-en-sysrm.gsm" => "
Default reminder.
",
	"ll-en-adrm.gsm" => "
Advertising reminder.
",
	"ll-en-nevrm.gsm" => "
Reminder blocked.
",
	"ll-en-admrm.gsm" => "
Custom reminder.
",
	"ll-en-norm.gsm" => "
Reminder has been cleared.
",
	"ll-en-gettype.gsm" => "
To change the box type enter the box number now.
",
	"ll-en-togtype.gsm" => "
To change the box type for this box press '1' now.
To go back to the main menu press the star key now.
(pause 3 secs)
",
	"ll-en-msgtype.gsm" => " 
Box is now a simple message box.
",
	"ll-en-bbstype.gsm" => "
Box is now an bulletin board box.
",
	"ll-en-newtype.gsm" => "
Box is now an autoassignment box.
",
	"ll-en-deftype.gsm" => "
Box is now the default box.
",
	"ll-en-admntype.gsm" => "
Box is now an administrative box.
",
	"ll-en-payctype.gsm" => "
Box is now a paycode administrative box.
",
	"ll-en-newadmn.gsm" => "
Enter the new admin box to be assigned to this box now.
Press the number sign four times to clear the admin box.
Press the star key four times to go back to the menu without 
changing the admin box for this box.
(pause 3 secs)
",
	"ll-en-confadmn.gsm" => "
Now reenter the new admin box for this box.
(pause 3 secs)
",
	"ll-en-isactv.gsm" => "
Box is now active.
",
	"ll-en-notactv.gsm" => "
Box is now inactive.
",
	"ll-en-onhold.gsm" => "
Box is now on hold.
",
	"ll-en-no.gsm" => "
No.
",
	"ll-en-box.gsm" => "
Box.
",
	"ll-en-boxes.gsm" => "
Boxes.
",
	"ll-en-users.gsm" => "
...active users.
",
	"ll-en-minbusy.gsm" => "
...minutes busy in the last 24 hours.
",
	"ll-en-maxwait.gsm" => "
...seconds. Maximum wait time in the last 24 hours.
",
	"ll-en-calls.gsm" => "
...calls in the last 24 hours.
",
	"ll-en-msgs.gsm" => "
...messages in the last 24 hours.
",
	"ll-en-lastcall.gsm" => "
Date of last call.
",
	"ll-en-boxcalls.gsm" => "
calls.
",
	"ll-en-boxmsgs.gsm" => "
messages.
",
	"ll-en-pgcalls.gsm" => "
...page calls in the last 24 hours.
",
	"ll-en-ppcused.gsm" => "
percent paycodes used.
",
	"ll-en-newsys.gsm" => "
Please enter your new system code now.
(pause 3 secs)
",
	"ll-en-exitmenu.gsm" => "
To restart the computer press 1 now.
To exit from the voice mail application without restarting the computer 
press 3 now.
To return to the main system menu press the star key now.
To repeat these instructions press zero.
(pause 3 secs)
",
	"ll-en-continue.gsm" => "
Press 1 to continue shutdown or any other key to return to menu.
(pause 3 secs)
",
	"ll-en-systemcd.gsm" => "
Please enter special system code now.
(pause 3 secs)
",
	"ll-en-msgrec.gsm" => "
(used with NUMBERS files to state time message was recorded)
Message recorded on the...
",
	"ll-en-newintro.gsm" => "
To set up your new voice mailbox please listen to the following
instructions carefully.
If you have a payment number enter the sixteen digit number now.
If you are responding to one of our ads please press the number sign 
followed by the special four digit code from the ad.
For more information press star 1000 now.
(pause 6 seconds)
",
	"ll-en-phintro.gsm" => "
Your phone number is ....
(Play the voice mail phone number.)
",
	"ll-en-ext.gsm" => "
Extension...
(Play the box number)
",
	"ll-en-nobox.gsm" => "
We are sorry, there are no new boxes available at this time. Please contact
your voice mail service provider as soon as possible.
",
	"ll-en-pinintro.gsm" => "
Security code 
(Play security code or prepend with NO.VOX)
",
	"ll-en-newinfo.gsm" => "
Please write down your new box number and security code and keep this 
information in a safe place. 

To hear your new box number and security code again press '1' at any
time while this message is playing.

When someone wants to leave you a message they would dial in, wait for
the phone to pick up and enter your box number at the prompt. They would
then hear your greeting and leave a message at the beep.

To access your box call in, enter your new box number followed immediately
by your new security code. If you hear your greeting and still want to
access your box enter your security code at any time, even after the beep.

If you have any questions regarding your voice mailbox please contact 
your voice mail service provider. Please note that for privacy reasons 
box numbers and security code information can not be given out.

Press '1' to hear your new box number and security code and listen to
this message again.
(Pause 6 seconds)
",
	"ll-en-errintro.gsm" => "
You entered ...
(Play paycode.)
",
	"ll-en-erroutro.gsm" => "
Please try again...
(Back to menu.)
",
	"ll-en-bdmenu.gsm" => "
To listen to the billing status of this box press 1 now.
To restore billing properties to their original values 
press 3 now.
To change the subscription date for this box press 4 now.
To change the value in cents for each box day for this box press 6 now.
To revert the number of billed box days for this box to unbilled 
box days press 7 now.
To change the number of billed box days for this box press 9 now.
To change the number of unbilled box days for this box press the 
number sign now.
To save your changes and go back to the previous menu 
press the star key now.
To repeat these instructions press zero.
(pause 3 seconds)
",
	"ll-en-youowe.gsm" => "
You owe
",
	"ll-en-dollars.gsm" => "
Dollars
",
	"ll-en-and.gsm" => "
And
",
	"ll-en-cents.gsm" => "
Cents
",
	"ll-en-for.gsm" => "
for
",
	"ll-en-nboxdays.gsm" => "
Unbilled box days.
",
	"ll-en-at.gsm" => "
At
",
	"ll-en-centsea.gsm" => "
Cents each.
",
	"ll-en-bboxdays.gsm" => "
Billed box days.
",
	"ll-en-bdupdate.gsm" => "
Date of last boxday update.
",
	"ll-en-getbdval.gsm" => "
To change the value for each billed box day for this admin box enter a 
two digit number now.
To return to the menu press star now.
(pause 3 secs)
",
	"ll-en-bdrevert.gsm" => "
The billed box days for this box have been added back to the 
unbilled box days for this box.
",
	"ll-en-bdrestor.gsm" => "
Billing properties for this box have been restored to 
their original values.
",
	"ll-en-silence.gsm" => "
(pause 10 seconds)
",
	"ll-en-bbdedit.gsm" => "
Enter the new number of billed box days followed by the number sign
or press the star key at any time to return to the menu.
",
	"ll-en-bbdclear.gsm" => "
Billed box days for this box have been cleared
",
	"ll-en-bdedit.gsm" => "
Enter the new number of unbilled box days followed by the number sign
or press the star key at any time to return to the menu.
",
	"ll-en-bdclear.gsm" => "
Unbilled box days for this box have been cleared
",
	"ll-en-edpdto.gsm" => "
To enter the new paid to date for this box enter the two digit day
followed by the two digit month followed by the four digit year.
To clear the paid to date press the number sign eight times.
To go back to the menu without changing the paid to date press the 
star key now.
(pause 3 seconds)
",
	"ll-en-systime.gsm" => "
The current system date and time is...
(plays system time)
",
	"ll-en-edtime.gsm" => "
To edit the system time enter a two digit hour from zero to twenty-three
followed by a two digit minute from zero to fifty-nine.
(pause 3 seconds)
",
	"ll-en-notime.gsm" => "
Error: the date and time could not be changed.
",
	"ll-en-zero.gsm" => "
Zero
",
	"ll-en-nofeat.gsm" => "
This feature is not available.
"
);
