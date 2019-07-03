package com.yineng.piv8

import android.content.Context
import android.database.SQLException
import android.database.sqlite.SQLiteDatabase
import android.os.Handler
import android.os.Looper
import org.json.JSONObject
import java.io.File
import java.util.concurrent.Executors

class piv8DB( private val ctx: Context,private val v8:V8, private var fileName: String = "") {

    init {
        fileName = "/data/data/${ctx.packageName}/databases/"
        val file = File(fileName)
        if(!file.exists()){
            file.mkdirs()
        }
    }

    val db : SQLiteDatabase = SQLiteDatabase.openOrCreateDatabase("${fileName}/piv8.db", null)
    var mainHandler = Handler(Looper.getMainLooper())
    val sExecutorService = Executors.newSingleThreadExecutor()

    internal val CREATE_TABLE_SQL = "CREATE TABLE IF NOT EXISTS %@ ( id TEXT NOT NULL, json TEXT NOT NULL, PRIMARY KEY(id))"
    internal val UPDATE_ITEM_SQL = "REPLACE INTO %@ (id, json) values (?,?)"
    internal val QUERY_ITEM_SQL = "SELECT json from %@ where id = ? Limit 1"
    internal val SELECT_ALL_SQL = "SELECT * from %@"
    internal val CLEAR_ALL_SQL = "DELETE from %@"
    internal val DELETE_ITEM_SQL = "DELETE from %@ where id = ?"

    fun create(inputTabName: String, success: V8Function, fail: V8Function, complete: V8Function){
        val v8Success = success.twin()
        val v8Fail = fail.twin()
        val v8Complete = complete.twin()
        val createSql = CREATE_TABLE_SQL.replace("%@", inputTabName)
        sExecutorService.execute {
            try {
                db.execSQL(createSql)
                mainHandler.post { call(v8Success, null, null) }
            } catch (e: SQLException) {
                mainHandler.post {  val arg = V8Array(v8);arg.push(e.message!!);call(v8Fail, null, arg);arg.close() }
            }
            mainHandler.post { call(v8Complete, null, null) }
        }
    }

    fun delete(inputTabName: String, success: V8Function, fail: V8Function, complete: V8Function){
        val v8Success = success.twin()
        val v8Fail = fail.twin()
        val v8Complete = complete.twin()
        val deleteTabSql = CLEAR_ALL_SQL.replace("%@", inputTabName)
        sExecutorService.execute {
            try {
                db.execSQL(deleteTabSql)
                mainHandler.post { call(v8Success, null, null) }
            } catch (e: SQLException) {
                mainHandler.post {  val arg = V8Array(v8);arg.push(e.message!!);call(v8Fail, null, arg);arg.close() }
            }
            mainHandler.post { call(v8Complete, null, null) }
        }
    }

    fun write(inputTabName: String, key: String, data: String, success: V8Function, fail: V8Function, complete: V8Function){
        val v8Success = success.twin()
        val v8Fail = fail.twin()
        val v8Complete = complete.twin()
        val updateSql = UPDATE_ITEM_SQL.replace("%@", inputTabName)
        sExecutorService.execute {
            try {
                db.execSQL(updateSql, arrayOf<Any>(key, data))
                mainHandler.post { call(v8Success, null, null) }
            } catch (e: SQLException) {
                mainHandler.post {  val arg = V8Array(v8);arg.push(e.message!!);call(v8Fail, null, arg);arg.close() }
            }
            mainHandler.post { call(v8Complete, null, null) }
        }
    }

    fun read(inputTabName: String, key: String, success: V8Function, fail: V8Function, complete: V8Function){
        val v8Success = success.twin()
        val v8Fail = fail.twin()
        val v8Complete = complete.twin()
        sExecutorService.execute {
            try {
                val querySql = QUERY_ITEM_SQL.replace("%@", inputTabName)
                val c = db.rawQuery(querySql, arrayOf(key))
                if (c.moveToNext()) {
                    val name = c.getColumnIndex("json")
                    val temp = c.getString(name)
                    mainHandler.post { val arg = V8Array(v8);arg.push(temp);call(v8Success, null, arg); arg.close() }
                } else {
                    mainHandler.post { val arg = V8Array(v8);arg.push("The key does not exist");call(v8Fail, null, arg);arg.close() }
                }
                c.close()
            } catch (e: SQLException) {
                mainHandler.post { val arg = V8Array(v8);arg.push(e.message!!);call(v8Fail, null, arg);arg.close() }
            }
            mainHandler.post { call(v8Complete, null, null) }
        }
    }

    fun remove(inputTabName: String, key: String, success: V8Function, fail: V8Function, complete: V8Function){
        val v8Success = success.twin()
        val v8Fail = fail.twin()
        val v8Complete = complete.twin()
        val removeSql = DELETE_ITEM_SQL.replace("%@", inputTabName)
        sExecutorService.execute {
            try {
                db.execSQL(removeSql, arrayOf<Any>(key))
                mainHandler.post { call(v8Success, null, null) }
            } catch (e: SQLException) {
                mainHandler.post { val arg = V8Array(v8);arg.push(e.message!!);call(v8Fail, null, arg);arg.close() }
            }
            mainHandler.post { call(v8Complete, null, null) }
        }
    }

    fun iterate(inputTabName: String, success: V8Function, fail: V8Function, complete: V8Function){
        val v8Success = success.twin()
        val v8Fail = fail.twin()
        val v8Complete = complete.twin()
        sExecutorService.execute {
            try {
                val quertyAllsql = SELECT_ALL_SQL.replace("%@", inputTabName)
                val allc = db.rawQuery(quertyAllsql, arrayOf())
                val map = HashMap<String, String>()
                while (allc.moveToNext()) {
                    val name = allc.getColumnIndex("json")
                    val resultValue = allc.getString(name)
                    val keyIndex = allc.getColumnIndex("id")
                    val resultKey = allc.getString(keyIndex)
                    map.put(resultKey, resultValue)
                }
                val json = JSONObject(map)
                mainHandler.post { val arg = V8Array(v8);arg.push(json.toString());call(v8Success, null, arg);arg.close() }
                allc.close()
            } catch (e: SQLException) {
                mainHandler.post { val arg = V8Array(v8);arg.push(e.message!!);call(v8Fail, null, arg);arg.close() }
            }
            mainHandler.post { call(v8Complete, null, null) }
        }
    }

    fun call(func: V8Function?, receiver: V8Object?, parameters: V8Array?): Any? {
        if (func == null || func.toString() == "undefined") {
            return null
        }
        val result = func.call(receiver, parameters)
        func.close()
        return result
    }

}