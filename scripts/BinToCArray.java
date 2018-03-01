package WebServer;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import static java.nio.file.StandardOpenOption.*;
import java.util.zip.GZIPOutputStream;

/**
 *
 * @author Kris
 */
public class BinToCArray {

    public static final int LINE_BREAK_AFTER = 16;
    public static final String HEX_FORMAT = "0x%02X,";
    
    public static void main(String[] args) throws IOException {
        
        if(args.length != 2){
            System.err.println("Specify input and output. Usage: java -jar BinToCArray.jar inputfile.bin outputfile.h");
            return;
        }
        
        File output = new File(args[1]);
        File input = new File(args[0]);
        Path path = input.toPath();

        byte[] binary = Files.readAllBytes(path);
        
        
        //gzip bin
        binary = compressArray(binary);
        String fileName = format_filename(path.getFileName());
        
        StringBuilder sb = new StringBuilder();
        sb.append("/*\n");
        sb.append(" *   ");
        sb.append(path.getFileName());
        sb.append(" (uncompressed size: ");
        sb.append(input.length());
        sb.append(" bytes)\n");
        sb.append(" */\n");
        sb.append("#define ");
        sb.append(fileName);
        sb.append("_len ");
        sb.append(binary.length);
        sb.append("\n");
        sb.append("const uint8_t ");
        sb.append(fileName);
        sb.append("[] PROGMEM = { \n");

        int currentHex = 0;
        for (byte b : binary) {
            sb.append(String.format(HEX_FORMAT, b));
            if(++currentHex % LINE_BREAK_AFTER == 0){
                 sb.append("\n");
            }
        }
        sb.deleteCharAt(sb.length()-1);
        sb.append("\n};");
        
        Files.write(output.toPath(), sb.toString().getBytes(), WRITE, CREATE, TRUNCATE_EXISTING);
    }
    
    
    private static byte[] compressArray(byte[] input) throws IOException{
        
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream(input.length);
        GZIPOutputStream zipStream = new GZIPOutputStream(byteStream);
        zipStream.write(input);
        zipStream.close();
        byteStream.close();
        
        return byteStream.toByteArray();
    }
    
    private static String format_filename (Path path) {
        String str = path.toString();
        int pos = str.lastIndexOf(".");
        if(pos == -1 ) return str;
        
        return str.substring(0, pos) + "_" + str.substring(pos+1, str.length());
        
    }
    
}
