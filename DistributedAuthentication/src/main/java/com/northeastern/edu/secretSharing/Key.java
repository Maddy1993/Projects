package com.northeastern.edu.secretSharing;

import java.math.BigInteger;

import javafx.util.Pair;

public class Key
{
    private Pair<Integer, BigInteger> point;

    public Key(Pair<Integer, BigInteger> point) {
        this.point = point;
    }

    public Pair<Integer, BigInteger> getPoint() {
        return point;
    }

    @Override
    public String toString() {
        if (point != null) {
            return point.toString();
        } else {
            return "";
        }
    }
}
