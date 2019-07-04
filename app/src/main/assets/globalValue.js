
//添加jsc需要的对象
var navigator = {};
JSVM.module = {};
var document = {};
document.body = {};
document.body.style = undefined;
var location = {};


//获取对象类型
function getType(obj) {
    var type = Object.prototype.toString.call(obj).match(/^\[object (.*)\]$/)[1].toLowerCase();
    if(type === 'string' && typeof obj === 'object') return 'object';
    if (obj === null) return 'null';
    if (obj === undefined) return 'undefined';
    return type;
}

//添加Map转JSON的方法
var map2dic = function(m){
    var dic = {};
    for(var [key,value] of m){
        if (getType(value) === "map"){
            value = map2dic(value);
        }
        dic[key] = value;
    }
    return dic;
}

//JSON转Map的方法
var dic2map = function(d){
    var m = new Map();
    for (var k in d){
        if (getType(d[k]) === "object"){
            d[k] = dic2map(d[k]);
        }
        m.set(k,d[k]);
    }
    return m;
}
