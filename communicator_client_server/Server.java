import java.io.*;
import java.net.*;

//MACIEJ NOWAK 384013

public class Server 
{
	private static Socket connectionSocket; //socket for client
	private static ServerSocket serverSocket; //socket for server
	private static final connectionThread[] threads = new connectionThread[100]; //array for client thread's, avaible thread is 100

	public static void main(String args[]) throws IOException
	{
		//default port
		int port = 5566;
		
		//if run with parameters, new port
		if(args.length == 1)
		{
			port = Integer.valueOf(args[0]).intValue();
		}
			
		serverSocket = new ServerSocket(port); //making server socket

		//listening on server socket
		while(true) 
		{
			connectionSocket = serverSocket.accept(); //making an instance (connection) of socket for client
			for(int i=0; i<100; i++) 
			{
				if(threads[i] == null) //if thread not exists, for example, threads[1] exists, so the loop will not make thread on index 1, but go to index 2
				{
					threads[i] = new connectionThread(connectionSocket, threads); //making new thread and adding thread to threads array
					threads[i].start(); //starting thread which means go to run method
					break;
				}
			}    
		}
	}
}

class connectionThread extends Thread 
{
	private String nick; //store nick for every connection
	private String message;  //store message from socket
	private BufferedReader input; //buffer to get message from client
	private PrintStream output; //stream to send message to threads
	private Socket connectionSocket; //socket for client
	private final connectionThread[] threads; //array of threads
	private String text[] = new String[2]; //store nick and message, if message send to specific user


	//constructor
	public connectionThread(Socket connectionSocket, connectionThread[] threads) 
	{
		this.connectionSocket = connectionSocket;
		this.threads = threads;
	}

	//method which is run for every new thread, sending message to threads and printing comments in server's command line
	public void run() 
	{
		connectionThread[] threads = this.threads;

		//making input and output for socket
		try 
		{
			input = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
			output = new PrintStream(connectionSocket.getOutputStream());
			output.println("Enter nick: ");
			nick = input.readLine(); //reading nick
	     
			//sending message
			while(true) 
			{
				int index = 0; //store index of threads
				message = input.readLine(); //reading message from socket
				
				//leave the loop and write to server about it
				if(message.equals("EXIT")) 
				{
					System.out.println(nick + " says: " + message); 
					break;
				}
				
				//write the list of active users and write to server about it
				if(message.equals("LIST")) 
				{
					System.out.println(nick + " says: " + message);
					for(int i=0; i<100; i++)
					{				
						if(threads[i] != null && (threads[i].nick).equals(nick))
						{index = i; break;}
					}					
					for(int i=0; i<100; i++)
					{				
						if(threads[i] != null)
						threads[index].output.println(threads[i].nick);	
					}	
				}
				
				//write the message to only specific user
				if(message.contains(":"))
				{
					text = message.split(":", 2);
					System.out.println(nick + " says: " + text[1] + " to: " + text[0]);
					for(int i=0; i<100; i++)
					{				
						if(threads[i] != null && (threads[i].nick).equals(text[0]))
						threads[i].output.println(nick + ": " + text[1]);	
					}
				}
				//write message to everyone
				else
				{
					if(!message.equals("LIST"))
					{
						System.out.println(nick + " says: " + message);
						for(int i=0; i<100; i++)
						{		
							if(threads[i] != null)
							threads[i].output.println(nick + ": " + message);	
						}
					}
				}		
			}
     
			//clearing slot
			for (int i=0; i<100; i++) 
			{
				if (threads[i] == this) threads[i] = null;
			}

		//closing everything
		output.close();
		input.close();
		connectionSocket.close();
		} 
		catch (IOException e) 
		{
			e.printStackTrace();
		}
	}
}
