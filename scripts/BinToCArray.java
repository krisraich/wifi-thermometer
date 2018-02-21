package bintocarray;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import static java.nio.file.StandardOpenOption.*;

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
        Path input = new File(args[0]).toPath();

        byte[] binary = Files.readAllBytes(input);
        
        StringBuilder sb = new StringBuilder();
        sb.append("/*\n");
        sb.append(" *   ");
        sb.append(input.getFileName());
        sb.append("\n");
        sb.append(" */\n");
        sb.append("const uint8_t bin_image[");
        sb.append(binary.length);
        sb.append("] PROGMEM = { \n");

        int currentHex = 0;
        for (byte b : binary) {
            sb.append(String.format(HEX_FORMAT, b));
            if(++currentHex % LINE_BREAK_AFTER == 0){
                 sb.append("\n");
            }
        }
        sb.deleteCharAt(sb.length()-1);
        sb.append("\n};");
        
        //System.out.println(sb.toString());
        
        Files.write(output.toPath(), sb.toString().getBytes(), WRITE, CREATE, TRUNCATE_EXISTING);
    }
}
