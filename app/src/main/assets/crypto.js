
var crypto = {}

crypto.getRandomValues = function(typedArray){
    for(var i = 0; i < typedArray.length; i++ ){
        typedArray[i] = JSVM.getRandomValues();
    }
    return typedArray;
}


