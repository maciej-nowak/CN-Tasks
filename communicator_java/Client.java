import java.io.*;
import java.net.*;

//MACIEJ NOWAK 384013

public class Client implements Runnable 
{
	private static BufferedReader message; //buffer to read message from client 
	private static DataOutputStream output; //stream to send message
	private static BufferedReader input; //buffer to get message from server
	private static Socket clientSocket; //socket for client
	private static boolean connection = true; //variable to check the connection
  
	public static void main(String[] args) throws IOException
	{
		//default port and host
		int port = 5566;
		String host = "localhost";

		//if run with parameters, new port and host
		if(args.length == 2)
		{
			port = Integer.valueOf(args[1]).intValue();
			host = args[0];
		}

		//making socket, output, input and buffer message's
		clientSocket = new Socket(host, port); 
		output = new DataOutputStream(clientSocket.getOutputStream()); 
		input = new BufferedReader(new InputStreamReader(clientSocket.getInputStream())); 		
		message = new BufferedReader(new InputStreamReader(System.in));
    
		//checking connection
		if(clientSocket == null || output == null || input == null) 
		{
			throw new IOException("Connection failed");
		}
		else
		{
			new Thread(new Client()).start(); //making new thread of Client
			while(connection) 
			{
				output.writeBytes(message.readLine() + '\n'); //sending message to server
			}
			//closing everything
			output.close();
			input.close();
			clientSocket.close();
		} 
	}

	//method which is run for every new thread, getting message from server and printing
	public void run()
	{
		String message;
		
		//get message from server
		try
		{
			while((message = input.readLine()) != null) 
			{
				System.out.println(message);
				if(message.equals("EXIT")) break;
			}
			connection = false;
		}
		catch(IOException e)
		{
			e.printStackTrace();
		}
	}
}