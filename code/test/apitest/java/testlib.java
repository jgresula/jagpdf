// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


package testlib;
import com.jagpdf.StreamOut;

import java.io.*;


public class testlib
{
    public static String getResourcesDir()
    {
        String result;
        try
        {
            BufferedReader in = new BufferedReader(new FileReader("JAG_TEST_RESOURCES_DIR.env"));
            result = in.readLine();
            in.close();
        }
        catch(IOException exc)
        {
            throw new RuntimeException(exc);
        }

        return result;
    }

    public static StreamOut getNoopStream()
    {
        return new NoopStream();
    }


    public static void wait_for_key()
    {
        try
        {
            System.in.read();
        }
        catch(IOException ignore) {}
    }
}


//
//
//
class NoopStream extends StreamOut
{
    public void write(byte[] data_in) {}
    public void close() {}
}



