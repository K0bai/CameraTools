/****************************************************************************
** Meta object code from reading C++ file 'CameraTools.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../CameraTools.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraTools.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CameraTools_t {
    QByteArrayData data[10];
    char stringdata0[147];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CameraTools_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CameraTools_t qt_meta_stringdata_CameraTools = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CameraTools"
QT_MOC_LITERAL(1, 12, 10), // "ThreadStop"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 22), // "button_startshow_click"
QT_MOC_LITERAL(4, 47, 17), // "button_grab_click"
QT_MOC_LITERAL(5, 65, 9), // "paint_img"
QT_MOC_LITERAL(6, 75, 7), // "src_img"
QT_MOC_LITERAL(7, 83, 22), // "combobox_camera_change"
QT_MOC_LITERAL(8, 106, 20), // "button_addmask_click"
QT_MOC_LITERAL(9, 127, 19) // "slider_value_change"

    },
    "CameraTools\0ThreadStop\0\0button_startshow_click\0"
    "button_grab_click\0paint_img\0src_img\0"
    "combobox_camera_change\0button_addmask_click\0"
    "slider_value_change"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CameraTools[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   50,    2, 0x08 /* Private */,
       4,    0,   51,    2, 0x08 /* Private */,
       5,    1,   52,    2, 0x08 /* Private */,
       7,    0,   55,    2, 0x08 /* Private */,
       8,    0,   56,    2, 0x08 /* Private */,
       9,    0,   57,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QImage,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CameraTools::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CameraTools *_t = static_cast<CameraTools *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ThreadStop(); break;
        case 1: _t->button_startshow_click(); break;
        case 2: _t->button_grab_click(); break;
        case 3: _t->paint_img((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 4: _t->combobox_camera_change(); break;
        case 5: _t->button_addmask_click(); break;
        case 6: _t->slider_value_change(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (CameraTools::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CameraTools::ThreadStop)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject CameraTools::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_CameraTools.data,
      qt_meta_data_CameraTools,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *CameraTools::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraTools::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_CameraTools.stringdata0))
        return static_cast<void*>(const_cast< CameraTools*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int CameraTools::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void CameraTools::ThreadStop()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
