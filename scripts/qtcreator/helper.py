from dumper import *

def qdump__tinystl__pair(d, value):
    first = value['first']
    second = value['second']
    d.putValue('[ %s, %s ]' % (first.display(), second.display()))
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putSubItem('first', first)
            d.putSubItem('second', second)

def qdump__tinystl__stringT():
    return [Latin1StringFormat, SeparateLatin1StringFormat,
            Utf8StringFormat, SeparateUtf8StringFormat ]

def qdump__tinystl__stringT(d, value):
    (first, last) = value.split('pp')
    if first == 0 or last == 0:
        d.putValue('(null)')
        d.putNumChild(0)
        return
    size = last - first
    d.putCharArrayHelper(first, size, d.createType('char'), d.currentItemFormat())

def qdump__tinystl__vector():
    return arrayForms()

def qdump__tinystl__vector(d, value):
    innerType = value.type[0]
    buffer = value['m_buffer']
    (first, last, capacity) = buffer.split('ppp')
    d.check(last <= capacity)
    size = int((last - first) / innerType.size())
    if size > 0:
        d.checkPointer(first)
        d.checkPointer(last)
        d.checkPointer(capacity)
    d.putItemCount(size)
    d.putPlotData(first, size, innerType)

def qdump__tinystl__unordered_map():
    return mapForms()

def qdump__tinystl__unordered_map(d, value):
    size = value['m_size'].integer()
    d.putItemCount(size)
    if d.isExpanded():
        keyType = value.type[0]
        valueType = value.type[1]
        typeCode = '@{%s}@{%s}pp' % (keyType.name, valueType.name)
        start = value['m_buckets']['first'].dereference()
        p = start.pointer()
        with Children(d, size):
            for i in d.childRange():
                (_, key, _, val, p, _) = d.split(typeCode, p)
                d.putPairItem(i, (key, val))

def qdump__tinystl__unordered_set(d, value):
    size = value['m_size'].integer()
    d.putItemCount(size)
    if d.isExpanded():
        keyType = value.type[0]
        typeCode = '@{%s}@pp' % (keyType.name)
        start = value['m_buckets']['first'].dereference()
        p = start.pointer()
        with Children(d, size):
            for i in d.childRange():
                (_, key, _, p, _) = d.split(typeCode, p)
                d.putSubItem(i, key)

def qdump__glm__vec(d, value):
    numElements = value.type[0]
    if numElements >= 4:
        d.putValue('[ %s, %s, %s, %s ]' % (value['x'].display(), value['y'].display(), value['z'].display(), value['w'].display()))
    elif numElements >= 3:
        d.putValue('[ %s, %s, %s ]' % (value['x'].display(), value['y'].display(), value['z'].display()))
    elif numElements >= 2:
        d.putValue('[ %s, %s ]' % (value['x'].display(), value['y'].display()))
    d.putNumChild(numElements)
    if d.isExpanded():
        with Children(d):
            d.putSubItem('x', value['x'])
            if numElements >= 4:
                d.putSubItem('w', value['w'])
            if numElements >= 3:
                d.putSubItem('z', value['z'])
            if numElements >= 2:
                d.putSubItem('y', value['y'])

def qdump__glm__qua(d, value):
    qdump__glm__vec(d, value)

def qdump__nanoem__ByteArray(d, value):
    buffer = value['m_buffer']
    (first, last, capacity) = buffer.split('ppp')
    size = (last - first)
    d.check(last <= capacity)
    if size > 0:
        d.checkPointer(first)
        d.checkPointer(last)
        d.checkPointer(capacity)
    d.putValue('%d bytes' % size)
    d.putNumChild(0)

def qdump__nanoem__MutableString():
    return [Latin1StringFormat, SeparateLatin1StringFormat,
            Utf8StringFormat, SeparateUtf8StringFormat ]

def qdump__nanoem__MutableString(d, value):
    buffer = value['m_buffer']
    (first, last, capacity) = buffer.split('ppp')
    if first == 0 or last == 0:
        d.putValue('(empty)')
        d.putNumChild(0)
        return
    size = (last - first)
    d.check(last <= capacity)
    if size > 0:
        d.checkPointer(first)
        d.checkPointer(last)
        d.checkPointer(capacity)
    d.putCharArrayHelper(first, size, d.createType('char'), d.currentItemFormat())

def qdump__nanoem__MutableWideString():
    return [SimpleFormat, SeparateFormat]

def qdump__nanoem__MutableWideString(d, value):
    buffer = value['m_buffer']
    (first, last, capacity) = buffer.split('ppp')
    if first == 0 or last == 0:
        d.putValue('(empty)')
        d.putNumChild(0)
        return
    size = (last - first)
    d.check(last <= capacity)
    if size > 0:
        d.checkPointer(first)
        d.checkPointer(last)
        d.checkPointer(capacity)
    d.putCharArrayHelper(first, size, d.createType('wchar_t'), d.currentItemFormat())
