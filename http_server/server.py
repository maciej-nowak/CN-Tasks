#MACIEJ NOWAK 384013

import socket
import signal
import sys


class Server:

 #Constructor
 def __init__(self): #self - give instance of class to every method of class
     self.host = 'localhost'
     self.port = 8080
     self.path = 'www'

    
 #Running the server 
 def start(self):
     self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #AF_INET - IPv4, SOCK_STREAM - socket type uses TCP to connection with IP
     print("Server HTTP has started on ", self.host, ":",self.port)
     self.socket.bind((self.host, self.port)) 
     self.listening()


 #listening for connection
 def listening(self):
     while True:
         print ("Listening")
         self.socket.listen(10) # max connections        
         client, address = self.socket.accept() #socket to client and client address      
         print("Connection from: ", address)        
         data = client.recv(1024) #receive data from client
         request_original = bytes.decode(data) #decoding to string
	 request = request_original.split(' ') #string.split(' ') deleting blank characters and putting \r\n
	 method = request[0] #request_method, the first word from string

	 #if request method is GET
         if (method == 'GET'): 
	     file_requested = request[1] #element request image.jpg, index.html         
             file_requested = file_requested.split('?')[0] #getting only name of page, without parameters (index.html?arg=wartosc)
     
	     #if only host has entered, go to default index.html
             if (file_requested == '/'): 
                 file_requested = '/index.php'
                  
             file_requested = self.path + file_requested #e.g. www/index.html
	     print(file_requested, " has downloaded")

             #if file has found
             try:
                 handler = open(file_requested,'rb') #making handler to file
                 content = handler.read() #generate response content from server                     
                 handler.close()               	  	
                 header = 'HTTP/1.1 200 OK\nServer: Simple-Python-HTTP-Server\n\n'      
                 
	     #if file has not found
             except Exception as e: 
                 headers = 'HTTP/1.1 404 Not Found\nServer: Simple-Python-HTTP-Server\n\n'             
                 content = b"<html><body><p>Error 404: File not found</p><p>Python HTTP server</p></body></html>"  
                 
	     #connecting header and content
             response =  header.encode()
             response +=  content

	     #sending request to client
             client.send(response)
             client.close()

         else:
             print("Only GET method")
  

def shutdown_server(sig, dummy):
    server.socket.shutdown(socket.SHUT_RDWR) #shut down the socket
    sys.exit(1) #shut down server


#MAIN
signal.signal(signal.SIGINT, shutdown_server) #shut down server when ctrl+c has triggered
server = Server()  #new Server object
server.start() #start server
