How to run this project:

1. Go into resources and put the remote server ip in the place of local host
2. Run GPRO-NET-VSLAUNCH and GPRO-NET-VSLAUNCH-REMOTE as administrator
3. In gpro-net-Console-Clint under source files, open main-client.cpp and set the server ip as the one used in step 1
4. Build and run the Console Server
5. Build and run the Console Client
6. The client must now enter a username and can do the followig
	-Type in messages
	-Refresh for new messages by pressing enter
	-Ask for a list of users by entering /users
7. On closing the server, a log of all messages can be found within the remote debug folders under the name serverlog.txt