package com.yineng.piv8.test

import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import com.yineng.piv8.R
import com.yineng.piv8.V8
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    private var runtime: V8? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        runtime = V8.createV8Runtime()

    }

    override fun onDestroy() {
        super.onDestroy()
        runtime?.close()
    }




    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */


}
