modttl 0.1.5 by quest
github - http://github.com/zquestz/modttl

modttl is intended for network administrators that are looking to modify
the TTL of packets beings sent from their servers.

This allows you to restrict the TTL of packets that you only want going
a certain number of hops from your server or extend the TTL of packets
that for some reason are set to low.

usage: modttl -r rule [-t TTL] [-h] [-d port]
-r rule         IPFW rule number to remove when quit (required)
-t TTL          TTL of packet
-d port         Divert port (optional)
-h              This help screen

If you don't know why this app is useful, then don't use it.

-quest
