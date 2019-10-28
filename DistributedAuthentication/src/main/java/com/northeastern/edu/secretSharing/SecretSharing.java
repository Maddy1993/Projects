package com.northeastern.edu.secretSharing;

import javafx.util.Pair;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import java.io.*;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.*;
import java.util.logging.Logger;

/**
 * Class prepares and reconstructs the secret keys
 */
public class SecretSharing {
    //Logger for the class.
    private static Logger LOGGER = Logger.getLogger(SecretSharing.class.getName());

    private static String secretString;
    private static BigInteger secret; // s
    private static int n = 5;  // NUM_SHARES
    private static int k = 3; // NUM_SUBSET_REQUIRED
    private static String secretStorageFileName;

    public SecretSharing(String fileName) {
        secretStorageFileName = fileName;
    }

    /**
     * Preparation phase of Secret Sharing
     * 1.) Randomly obtain k-1 numbers
     * 2.) Construct polynomial
     * 3.) Construct n points
     * 4.) Create list of n keys containing point and value p
     *
     * @param s
     */
    public List<Key> preparation(String s) throws IOException {
        secretString = s;
        // Convert input secret into BigInt format
        secret = new BigInteger(1, s.getBytes());
        LOGGER.info("Secret: " + secret);
        writeSecretToMemory();

        // obtain k - 1 random numbers (a1, a2, a3, etc.) to construct polynomial:
        // f(x) = a0 + a1x + a2x^2 + ...
        List<BigInteger> coefficients = getCoefficients();

        // Generate list of Keys
        return generateKeys(coefficients);
    }

    /**
     * Reconstruction phase of Secret Sharing.
     * 1.) Need k keys to reconstruct secret -> ELSE reconstruction fails
     * 2.) Reconstruct polynomial using Lagrange Polynomial Interpolation
     * 3.) Solve for constant value to obtain original secret
     * 4.) If reconstructd constant does not match original secret -> reconstruction fails
     *
     * @param clientKeyList
     */
    public boolean reconstruction(List<Key> clientKeyList) throws IOException {
        // Does client have enough keys to reconstruct secret?
        if (clientKeyList.size() != k) {
            return false;
        }

        // Calculate Lagrange Basis Polynomials l(x)
        List<Integer> lagrangeBasisPoly = new ArrayList<>();
        BigDecimal y = BigDecimal.ZERO;
        for (int j = 0; j < clientKeyList.size(); j++) {
            System.out.println("x" + j + " " + clientKeyList.get(j).getPoint().getKey());
            System.out.println("y" + j + " " + clientKeyList.get(j).getPoint().getValue());

            BigDecimal xj = BigDecimal.valueOf(clientKeyList.get(j).getPoint().getKey());
            BigDecimal yj = new BigDecimal(clientKeyList.get(j).getPoint().getValue());
            //int l = 1;
            BigDecimal l = BigDecimal.ONE;
            //int x = 0;
            BigDecimal x = BigDecimal.ZERO;

            // Calculate the constants in the polynomial for each point and sum them together
            for (int m = 0; m < clientKeyList.size(); m++) {
                BigDecimal xm = BigDecimal.valueOf(clientKeyList.get(m).getPoint().getKey());
                if (j != m) {
                    // Lagrange Basis: l = l * ((x - xm) / (xj - xm));
                    BigDecimal numerator = x.subtract(xm).setScale(12, BigDecimal.ROUND_HALF_UP);
                    BigDecimal denominator = xj.subtract(xm).setScale(12, BigDecimal.ROUND_HALF_UP);
                    BigDecimal result = numerator.divide(denominator, BigDecimal.ROUND_HALF_UP);
                    l = l.multiply(result);
                }
            }
            y = y.add(yj.multiply(l));
        }
        System.out.println("Y: " + y);

        // Round big decimal Y
        y = y.setScale(0, BigDecimal.ROUND_HALF_UP);
        System.out.println("Yrounded: " + y);

        // Convert y to BigInteger for comparison to secret (BigInteger)
        BigInteger yConverted = y.toBigInteger();

        System.out.println("Y big Integer: " + yConverted);

        String sec = (String) loadSecretFromMemory(1);

        if (!sec.isEmpty())
        {
            BigInteger storedSecret = new BigInteger(sec);
            // Check that constructd Y value is equal to Secret value
            if (yConverted.equals(storedSecret)) {
                return true;
            } else {
                return false;
            }
        }

        return false;

    }

    /**
     * Creates the polynomial to the k-1 term: f(x) = a0 + a1x + a2x^2 + ... + (ak-1(x^k-1
     *
     * @return list of the coeficients in the polynomial, where a0 = value of secret
     */
    private static List<BigInteger> getCoefficients() {
        Set<BigInteger> setBigInt = new LinkedHashSet<>();
        // a0 = secret big integer value
        setBigInt.add(secret);

        BigInteger bigInteger = secret;// upper limit
        BigInteger min = BigInteger.ONE;// lower limit
        BigInteger bigInteger1 = bigInteger.subtract(min);
        Random rnd = new Random();
        int maxNumBitLength = bigInteger.bitLength();

        BigInteger aRandomBigInt;

        // Randomly choose coefficients between 1 and value of secret
        while (setBigInt.size() < k) {
            aRandomBigInt = new BigInteger(maxNumBitLength, rnd);
            if (aRandomBigInt.compareTo(min) < 0)
                aRandomBigInt = aRandomBigInt.add(min);
            if (aRandomBigInt.compareTo(bigInteger) >= 0)
                aRandomBigInt = aRandomBigInt.mod(bigInteger1).add(min);
            setBigInt.add(aRandomBigInt);
            System.out.println("Coef: " + aRandomBigInt);
        }

        // Convert Set to List to allow for iteration by index
        List<BigInteger> coefsList = new ArrayList<BigInteger>(setBigInt);

        return coefsList;

    }

    /**
     * Constructs n points from the polynomial
     *
     * @return List of n points along the polynomial
     */
    private static List<Key> generateKeys(List<BigInteger> coefs) {
        List<Key> keys = new LinkedList<>();
        Key key;
        // f(x) = a0 + a1x + a2x^2 + ...
        // Calculates points for when x = 1, 2, ...n
        for (int i = 1; i <= n; i++) {
            BigInteger x = BigInteger.valueOf(i);
            // create key with values point and p
            // Pair<x,y<, where y = f(x) mod p
            //key = new Key(new Pair<>(i, getY(coefs, x).mod(p)), p);
            key = new Key(new Pair<>(i, getY(coefs, x)));
            keys.add(key);
        }
        return keys;
    }

    /**
     * Calculates the Y value for a given X from the polynomial f(x)
     *
     * @param coefs
     * @param x
     * @return y value from f(x)
     */
    private static BigInteger getY(List<BigInteger> coefs, BigInteger x) {
        //f(x) = a0 + a1x + a2x^2 + ...anx^n
        BigInteger y = BigInteger.ZERO;

        for (int i = 0; i < coefs.size(); i++) {
            BigInteger xp = x.pow(i);
            y = y.add(coefs.get(i).multiply(xp));
        }

        return y;
    }

//    public static void main(String[] args) {
//        List<Key> keys = preparation("dog");
//
//        // get random 3 out of 5 keys
//        HashSet<Integer> subsetKeys = new LinkedHashSet<>();
//        while (subsetKeys.size() < 3) {
//            Random r = new Random();
//            subsetKeys.add(r.nextInt(n));  // [0...n-1])
//        }
//        List<Key> subsetKeyList = new ArrayList<>();
//
//        for (Integer ind : subsetKeys) {
//            subsetKeyList.add(keys.get(ind));
//        }
//
//        System.out.println(keys.get(0).toString());
//        System.out.println("Subset key list size: " + subsetKeyList.size());
//
//
//        System.out.println(reconstruction(subsetKeyList));
//    }

    //Loads the client password value data store from memory.
    protected Object loadSecretFromMemory(int mode) throws IllegalStateException, IOException {
        try {
            FileReader reader = new FileReader(secretStorageFileName);
            JSONParser jsonParser = new JSONParser();
            return ((Map<String, String>)jsonParser.parse(reader)).getOrDefault("secret", "");
        } catch (IOException e) {
            String message = "Error loading data from memory: " + e.getMessage();
            LOGGER.severe(message);

            if (mode == 0) {
                new File(secretStorageFileName).createNewFile();
            } else {}

        } catch (ParseException e) {
            LOGGER.info("File: " + secretStorageFileName + " is empty.");
        }

        return defaultMemoryObject();
    }

    //Generates the structure of a default memory object
    protected Object defaultMemoryObject() {
        Map<String, Object> defaultMemoryObject = new HashMap<>();
        return defaultMemoryObject;
    }

    //Write the learned keys to memory
    void writeSecretToMemory() throws IOException {
        JSONObject jsonObject = new JSONObject();

        try {
            //Creating a map of values to store.
            FileReader reader = new FileReader(secretStorageFileName);
            JSONParser jsonParser = new JSONParser();

            try {
                jsonObject = (JSONObject) jsonParser.parse(reader);
            } catch (ParseException e) {
                jsonObject = new JSONObject();
            }

            reader.close();

        } catch (IOException e) {
            LOGGER.severe("Error while saving the file to memory." + e.getMessage());
            new File(secretStorageFileName).createNewFile();
        }

        OutputStream writer = new FileOutputStream(secretStorageFileName);
        jsonObject.put("secret", secret.toString());
        writer.write(jsonObject.toJSONString().getBytes());
        writer.flush();
        writer.close();
    }
}
