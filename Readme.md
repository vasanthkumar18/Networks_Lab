# LocalDNSServer

Implement a simple local DNS server called LocalDNSServer. LocalDNSServer accepts queries
from clients and tries to resolve them, first by checking its local cache and in case of a miss, by
asking other name servers. LocalDNSServer must reply to A, AAAA, NS, and CNAME requests,
and it may drop queries of all other types. Before you start to implement your server, you should
have a clear understanding of the DNS protocol and, specifically, of the message format.
