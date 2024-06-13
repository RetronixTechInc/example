
Socket sample :

Build at x86 ubuntu18.04 : enter to PVGW folded.	

    g++ pvgw_socket.cpp -static -o pvgw_socket
  
Usage:

    pvgw_socekt -h
    Read : pvgw_socekt server-ip read-tag
	Write : pvgw_socekt server-ip write-tag val

//////////////////////////////////////////////////////////////////////////////////
	
Vendor Resume Transmission Server :

Build at x86 ubuntu18.04 : enter to PVGW/VendorResumeTransmissionServer
  
	make all
	
Usage:
	
	SocketServer port
	example : socketServer 10010
  
//////////////////////////////////////////////////////////////////////////////////

Vendor Resume Transmission Client :

Build at x86 ubuntu18.04 : enter to PVGW/VendorResumeTransmissionClient
  
	make all
	
Usage:
	
	SocketClient ip port
	example : socketServer 192.168.3.127 10011
