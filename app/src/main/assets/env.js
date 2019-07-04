
console.log = function(one,two,three,four){
    if (isConsole) {
        console.print(one,two,three,four);
    }
};

class WebSocket{
    constructor(url) {
        this.ws = WebSocketManger.getWebSocket(url);
    }

    send(sendData){
        if( typeof(sendData) == "string" ){
            this.ws.sendWithType(sendData,'string');
        }else{
            var u8 = new Uint8Array(sendData)
            this.ws.sendWithType(self.base64js.fromByteArray(u8),'bin');
        }
    }

    close(){
        this.ws.close();
    }
    
    set onopen(cb){
        this.ws.onopen = cb;
    }

    set onclose(cb){
        this.ws.onclose = cb;
    }

    set onerror(cb){
        this.ws.onerror = cb;
    }

    set onmessage(cb){
        var om = function (dic, type){
            if( type == "string" ){
                cb(dic);
            }else{
                dic.data = base64js.toByteArray(dic.data).buffer;
                cb(dic);
            }
        }
        this.ws.onmessage = om;
    }

}
