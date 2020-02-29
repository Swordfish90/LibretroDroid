package com.swordfish.libretrodroid;

public class Variable {
    public String key;
    public String value;
    public String description;

    public Variable(String key, String value, String description) {
        this.key = key;
        this.value = value;
        this.description = description;
    }

    public Variable() { }
}
