import java.util.logging.Level;
import java.util.logging.Logger;
import static com.mizuho.mlib.SwigMLIB.*;
import com.mizuho.mlib.*;


public class SwigMLibJavaMain {
    
    static {
        System.loadLibrary("swig_mlib");
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        int a = getInt();
        System.out.println(a);        
        double b = getDouble();
        System.out.println(b);
        char c  = getChar();
        System.out.println(c);
        boolean d = getBool();
        System.out.println(d);
        String e = getConstChar();
        System.out.println(e);
        String f = getStdString();
        System.out.println(f);
        
        VecInteger vecOfInt = getVectorOfInt();
        for(int i = 0; i < vecOfInt.size(); i++) {
            System.out.print(vecOfInt.get(i));
            System.out.print(",");
        }
        System.out.print("\n");
        
        VecVecInteger vecOfVecOfInt = getVectorOfVectorOfInt();
        for(int i = 0; i < vecOfVecOfInt.size(); i++) {
            VecInteger internalVecOfInt = vecOfVecOfInt.get(i);
            long internalVecLength = internalVecOfInt.size();
            for(int j = 0; j < internalVecLength; j++) {
                System.out.print(internalVecOfInt.get(j));
                if(j < (internalVecLength - 1)) 
                    System.out.print(",");
            }
            System.out.print("#");
        }
        System.out.print("\n");

        VecDouble vecOfDouble = getVectorOfDouble();
        for(int i = 0; i < vecOfDouble.size(); i++) {
            System.out.print(vecOfDouble.get(i));
            System.out.print(",");
        }
        System.out.print("\n");
        
        VecVecDouble vecOfVecOfDouble = getVectorOfVectorOfDouble();
        for(int i = 0; i < vecOfVecOfDouble.size(); i++) {
            VecDouble internalVecOfDouble = vecOfVecOfDouble.get(i);
            long internalVecLength = internalVecOfDouble.size();
            for(int j = 0; j < internalVecLength; j++) {
                System.out.print(internalVecOfDouble.get(j));
                if(j < (internalVecLength - 1)) 
                    System.out.print(",");
            }
            System.out.print("#");
        }
        System.out.print("\n");
        
        
        VecString vecOfString = getVectorOfString();
        for(int i = 0; i < vecOfString.size(); i++) {
            System.out.print(vecOfString.get(i));
            System.out.print(",");
        }
        System.out.print("\n");
        
        VecVecString vecOfVecOfString = getVectorOfVectorOfString();
        for(int i = 0; i < vecOfVecOfString.size(); i++) {
            VecString internalVecOfString = vecOfVecOfString.get(i);
            long internalVecLength = internalVecOfString.size();
            for(int j = 0; j < internalVecLength; j++) {
                System.out.print(internalVecOfString.get(j));
                if(j < (internalVecLength - 1)) 
                    System.out.print(",");
            }
            System.out.print("#");
        }
        System.out.print("\n");

        try{
            VecDouble vcDouble = new VecDouble(5);
            System.out.println(vcDouble.capacity());
            vcDouble.set(0,465.64);
            vcDouble.set(1,-4465.64);
            vcDouble.set(2,5465.64);
            vcDouble.set(3,15465.64);
            vcDouble.set(4,95465.64);
            for(int i = 0; i < vcDouble.size(); i++) {
                System.out.print(vcDouble.get(i));
                System.out.print(",");
            }            
            System.out.println("");
            
            double res1 = sumUp(vcDouble);
            System.out.println(res1);
            
            double[] dArray = new double[5];
            dArray[0] = 465.64;
            dArray[1] = -4465.64;
            dArray[2] = 5465.64;
            dArray[3] = 15465.64;
            dArray[4] = 95465.64;
            double res2 = sumUp(dArray,5);
            System.out.println(res2);

            throwAnIllegalArgumentException();
        
        } catch(IllegalArgumentException ex) {
            System.out.println(ex.getMessage());
        } catch (Exception ex) {
            System.out.println(ex.getMessage());
        }
        
        String g = Double.toString(doSomeFormula(0.45));
        System.out.println(g);

        String initString = setUpMLIB("X:\\Secure\\DIR_MLib_API\\ir.properties", "X:\\Secure\\DIR_MLib_API\\Calendar.csv");
        System.out.println(initString);
        
        String h = getDateFromTerm("20170115", 1.0, "Act/360", false);
        System.out.println(h);
        String k = getDate("20170115","3M", "MOD_FOLLOWING", "TGT");
        System.out.println(k);
                
        String closeString = tearDownMLIB();
        System.out.println(closeString);
        
        try {
            Thread.sleep(1000);
        } catch (InterruptedException ex) {
            System.out.println(ex.getMessage());
        }
        
    }    
    
}
