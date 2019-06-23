package com.yineng.piv8.utils;

public abstract class SingleTypeAdapter implements TypeAdapter {

    private int typeToAdapt;

    /**
     * Create a SingleTypeAdapter
     *
     * @param typeToAdapt The V8 Type this TypeAdapter should be applied to.
     */
    public SingleTypeAdapter(final int typeToAdapt) {
        this.typeToAdapt = typeToAdapt;
    }

    @Override
    public Object adapt(final int type, final Object value) {
        if (type == typeToAdapt) {
            return adapt(value);
        }
        return TypeAdapter.DEFAULT;
    }


    public abstract Object adapt(final Object value);

}
