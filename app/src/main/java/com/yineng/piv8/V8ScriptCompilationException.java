package com.yineng.piv8;

@SuppressWarnings("serial")
public class V8ScriptCompilationException extends V8ScriptException {

    V8ScriptCompilationException(final String fileName, final int lineNumber,
                                 final String message, final String sourceLine, final int startColumn, final int endColumn) {
        super(fileName, lineNumber, message, sourceLine, startColumn, endColumn, null, null);
    }

}
