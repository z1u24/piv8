package com.yineng.piv8.utils

import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream

class FileUtil{
    companion object {
        fun readFile(path: String): String {
            var content = ""
            val file = File(path)
            try {
                val stream = FileInputStream(file)
                val bs = ByteArray(stream.available())
                stream.read(bs)
                content = String(bs, Charsets.UTF_8)
                stream.close()
            } catch (e: Exception) {

            }

            return content
        }

        fun readFileToData(path: String): ByteArray? {
            var bs: ByteArray? = null
            val file = File(path)
            try {
                val stream = FileInputStream(file)
                bs = ByteArray(stream.available())
                stream.read(bs)
                stream.close()
            } catch (e: Exception) {

            }

            return bs
        }

        fun writeFile(path: String, content: ByteArray, append: Boolean?) {
            val f = File(path)
            try {
                if (!f.exists()) {
                    File(path.substring(0, path.lastIndexOf('/'))).mkdirs()
                    f.createNewFile()
                }
                val stream = FileOutputStream(f, append!!)
                stream.write(content)
                stream.close()
            } catch (e: Exception) {

            }

        }
    }
}