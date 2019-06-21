package com.yineng.piv8

import java.io.Closeable

interface Releasable : Closeable{

    override fun close()

}