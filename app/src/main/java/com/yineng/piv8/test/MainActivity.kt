package com.yineng.piv8.test

import android.annotation.SuppressLint
import android.app.Service
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import com.yineng.piv8.*
import kotlinx.android.synthetic.main.activity_main.*
import java.io.BufferedReader
import java.io.InputStreamReader
import java.security.SecureRandom

class MainActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)


    }




    override fun onDestroy() {
        super.onDestroy()
    }





}

