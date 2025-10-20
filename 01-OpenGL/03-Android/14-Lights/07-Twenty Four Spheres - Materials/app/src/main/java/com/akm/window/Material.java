package com.akm.window;
public class Material {
    public float[] ambient  = new float[4];
    public float[] diffuse  = new float[4];
    public float[] specular = new float[4];
    public float shininess;

    public Material(float[] a, float[] d, float[] s, float sh) {
        ambient  = a;
        diffuse  = d;
        specular = s;
        shininess = sh;
    }
}