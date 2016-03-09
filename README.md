Using Libraris from https://code.google.com/archive/p/avr-net/source/default/source
for the tcp/ip stack by Paweł Lebioda <pawel.lebioda89@gmail.com> all under GPL2.
I have modified most of the files. Before that were seperate ethernet buffers for transmitt and receive. As well as tcp had buffers per socket.
None of that exists now. Instead there is only one ethernet buffer to reduce ram usage.
TCP has been greatly simplified: does no longer defragment packets and there is no timeout and retransmission. 

Based of code from work done by Eric Rasmussen:
https://www.olimex.com/Products/Modules/Ethernet/ENC28J60-H/resources/Webserver_ATMega32_ENC28J60-H.zip
He based it on code by Guido Socher and Pascal Stang, and hence licensed as GPL2. See http://www.gnu.org/licenses/gpl.html


All code is under GPL2.

# AVR_WebServer
SchoolProjectWebServer
