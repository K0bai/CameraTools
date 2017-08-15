/****************************************************************************
** Meta object code from reading C++ file 'DataProcess.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../DataProcess.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DataProcess.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DataProcess_t {
    QByteArrayData data[8];
    char stringdata0[91];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DataProcess_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DataProcess_t qt_meta_stringdata_DataProcess = {
    {
QT_MOC_LITERAL(0, 0, 11), // "DataProcess"
QT_MOC_LITERAL(1, 12, 16), // "SendUpdataImgMsg"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 15), // "SendPaintImgMsg"
QT_MOC_LITERAL(4, 46, 16), // "SendImageGrabMsg"
QT_MOC_LITERAL(5, 63, 9), // "SendAbort"
QT_MOC_LITERAL(6, 73, 3), // "ret"
QT_MOC_LITERAL(7, 77, 13) // "SaveGrabImage"

    },
    "DataProcess\0SendUpdataImgMsg\0\0"
    "SendPaintImgMsg\0SendImageGrabMsg\0"
    "SendAbort\0ret\0SaveGrabImage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DataProcess[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,
       3,    0,   40,    2, 0x06 /* Public */,
       4,    0,   41,    2, 0x06 /* Public */,
       5,    1,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   45,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void DataProcess::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DataProcess *_t = static_cast<DataProcess *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->SendUpdataImgMsg(); break;
        case 1: _t->SendPaintImgMsg(); break;
        case 2: _t->SendImageGrabMsg(); break;
        case 3: _t->SendAbort((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->SaveGrabImage(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (DataProcess::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DataProcess::SendUpdataImgMsg)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (DataProcess::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DataProcess::SendPaintImgMsg)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (DataProcess::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DataProcess::SendImageGrabMsg)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (DataProcess::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DataProcess::SendAbort)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject DataProcess::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_DataProcess.data,
      qt_meta_data_DataProcess,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *DataProcess::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataProcess::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_DataProcess.stringdata0))
        return static_cast<void*>(const_cast< DataProcess*>(this));
    return QThread::qt_metacast(_clname);
}

int DataProcess::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void DataProcess::SendUpdataImgMsg()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void DataProcess::SendPaintImgMsg()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void DataProcess::SendImageGrabMsg()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void DataProcess::SendAbort(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
