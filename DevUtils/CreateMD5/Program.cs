using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Security.Cryptography;

namespace CreateMD5
{
	class Program
	{
		static void Main(string[] args)
		{
			StreamReader sr;


			Console.WriteLine("-- CREATE MD5 SUM STRING --");
			Console.WriteLine(" ");
			Console.WriteLine("What is the file name?");
			String fileName = Console.ReadLine();

			//try
			//{
			//    // Create an instance of StreamReader to read from a file.
			//    // The using statement also closes the StreamReader.
			//    using (sr = new StreamReader(fileName))
			//    {
			//        String line;
			//        // Read and display lines from the file until the end of 
			//        // the file is reached.
			//        while ((line = sr.ReadLine()) != null)
			//        {
			//            Console.WriteLine(line);
			//        }
			//    }
			//}
			//catch (Exception e)
			//{
			//    // Let the user know what went wrong.
			//    Console.WriteLine("The file could not be read:");
			//    Console.WriteLine(e.Message);
			//}

			FileStream fs = new FileStream(fileName, FileMode.Open);


			// Now that we have a byte array we can ask the CSP to hash it
			MD5 md5 = new MD5CryptoServiceProvider();
			byte[] result = md5.ComputeHash(fs);

			// Build the final string by converting each byte
			// into hex and appending it to a StringBuilder
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < result.Length; i++)
			{
				sb.Append(result[i].ToString("X2"));
			}

			// And return it
			//return sb.ToString();

			Console.WriteLine(" ");

			Console.WriteLine("HashAlgorithm Length: {0}\n", sb.ToString().Length);
			Console.WriteLine("HashAlgorithm Length: {0:N}\n", sb.ToString().Length); 

			Console.WriteLine("PRESS THE 'ENTER' KEY TO FINISH...");
			String nothing = Console.ReadLine();
		}
	}
}
