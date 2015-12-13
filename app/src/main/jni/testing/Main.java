/*
 * Create an image from a pixel array in log.txt
 *
 * */

import java.util.*;
import java.nio.*;
import java.nio.file.*;
import java.io.*;
import java.awt.*;
import java.awt.image.*;
import javax.imageio.*;

class Main {
  public static void main(String[] args) {
    try {
      Path path = Paths.get("log.txt");
      byte[] data = Files.readAllBytes(path);

      BufferedImage out;
      int type;

      type = BufferedImage.TYPE_4BYTE_ABGR;

      int width = data.length;

      out = new BufferedImage(1280, 960, type);

      //out.getRaster().setDataElements(0, 0, 1280, 960, data);
      final byte[] targetPixels = ((DataBufferByte) out.getRaster().getDataBuffer()).getData();
      for (int i = 0; i < data.length; i += 4) {
        targetPixels[i] = data[i + 3];
        targetPixels[i + 1] = data[i + 2];
        targetPixels[i + 2] = data[i + 1];
        targetPixels[i + 3] = data[i];
      }
      //System.arraycopy(data, 0, targetPixels, 0, data.length);  

      File outputfile = new File("saved.png");
      if (outputfile.exists())
        outputfile.delete();
      outputfile.createNewFile();
      ImageIO.write(out, "png", outputfile);
    } catch (IOException e) {
      System.out.println("ERROR: " + e.getMessage());
    }
  }
}
