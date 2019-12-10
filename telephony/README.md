# dialogic 

This is the legacy dialogic software for Lifeline. 

# asterisk-1.4, asterisk-1.6:

This is a reference build of asterisk that is known to work. Its identical to asterisk but contain
the apps/app_recordkeys.c (RecordKeys) asterisk application that can branch based on used input
while recording. RecordKeys is not part of the current asterisk distribution but I have send along
a patch: https://issues.asterisk.org/view.php?id=17487

This version of RecordKeys also differs from Record in that it does not automatically delete 
messages and stop the application on hang up.

Both key capture and tolerant hang up are needed to make the user interface less confusing for
people who are not comfortable working with IVR systems.

I am running asterisk as the user asterisk in its own tree. I gave the asterisk user sudo 
privelages (added the sudo group to the user with "usermod --append -G sudo asterisk" run as root).

## Building Asterisk

The commands used to make the versions of asterisk were:

asterisk 1.4: 
sudo mkdir /usr/local/asterisk-1.4
sudo chown asterisk:asterisk /usr/local/asterisk-1.4
[in the telephony/asterisk-1.4* source directory]
./configure --with-gsm=internal --prefix=/usr/local/asterisk-1.4 --enable-dev-mode

asterisk 1.6:
sudo mkdir /usr/local/asterisk-1.6
sudo chown asterisk:asterisk /usr/local/asterisk-1.6
[in the telephony/asterisk-1.6 source directory]
./configure --with-gsm=internal --prefix=/usr/local/asterisk-1.6 --enable-dev-mode

Then as asterisk

make && make install 

I used a symlink /usr/local/asterisk to default to a specific version of asterisk (1.4 currently).

To make a working asterisk server you may want to additionally run "make samples" to build sample
configuration files in /usr/local/asterisk/etc/asterisk. There is an example extensions.conf 
in asterisk-common/etc/asterisk/extensions.conf. Follow the instructions from your VoIP 
ISP for how to configure the trunk connection from their servers to your asterisk server.

To start the asterisk server simply use the command "asterisk". To use the asterisk CLI 
(command line interface) I use "asterisk -vvvdnr" once the asterisk server is started. For asterisk
to start automatically on server reboot add the "asterisk" command above to /etc/rc.local:

su -c /usr/local/asterisk-1.4/sbin/asterisk asterisk

This syntax ensures that the asterisk server is run as asterisk. If you are using the symlink
you may want to change /usr/local/asterisk-1.4 to /usr/local/asterisk.

## Other notes:

* The "--enable-dev-mode" option is not essential.

* You may need to remove the menuselect.makeopts if you rebuild with a new version of asterisk

* Version 1.6 changed substantially from June 2010 to January 2011 and may change substantially again.
  This may mean having to manually update apps/app_recordkey.c and apps/app_recordkeys.c if there are major
  updates in the future. 1.4 thankfully isn't being actively worked on.

