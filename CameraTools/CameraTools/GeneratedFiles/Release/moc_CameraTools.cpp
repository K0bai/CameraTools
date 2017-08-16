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
    QByteArrayData data[14];
    char stringdata0[239];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CameraTools_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CameraTools_t qt_meta_stringdata_CameraTools = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CameraTools"
QT_MOC_LITERAL(1, 12, 22), // "button_startshow_click"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 17), // "button_grab_click"
QT_MOC_LITERAL(4, 54, 20), // "button_addmask_click"
QT_MOC_LITERAL(5, 75, 25), // "button_startcapture_click"
QT_MOC_LITERAL(6, 101, 9), // "paint_img"
QT_MOC_LITERAL(7, 111, 22), // "combobox_camera_change"
QT_MOC_LITERAL(8, 134, 26), // "combobox_imagestyle_change"
QT_MOC_LITERAL(9, 161, 27), // "combobox_detecttools_change"
QT_MOC_LITERAL(10, 189, 19), // "slider_value_change"
QT_MOC_LITERAL(11, 209, 9), // "get_abort"
QT_MOC_LITERAL(12, 219, 3), // "msg"
QT_MOC_LITERAL(13, 223, 15) // "checkbox_filter"

    },
    "CameraTools\0button_startshow_click\0\0"
    "button_grab_click\0button_addmask_click\0"
    "button_startcapture_click\0paint_img\0"
    "combobox_camera_change\0"
    "combobox_imagestyle_change\0"
    "combobox_detecttools_change\0"
    "slider_value_change\0get_abort\0msg\0"
    "checkbox_filter"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CameraTools[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    0,   71,    2, 0x08 /* Private */,
       5,    0,   72,    2, 0x08 /* Private */,
       6,    0,   73,    2, 0x08 /* Private */,
       7,    0,   74,    2, 0x08 /* Private */,
       8,    0,   75,    2, 0x08 /* Private */,
       9,    0,   76,    2, 0x08 /* Private */,
      10,    0,   77,    2, 0x08 /* Private */,
      11,    1,   78,    2, 0x08 /* Private */,
      13,    0,   81,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,

       0        // eod
};

void CameraTools::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CameraTools *_t = static_cast<CameraTools *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->button_startshow_click(); break;
        case 1: _t->button_grab_click(); break;
        case 2: _t->button_addmask_click(); break;
        case 3: _t->button_startcapture_click(); break;
        case 4: _t->paint_img(); break;
        case 5: _t->combobox_camera_change(); break;
        case 6: _t->combobox_imagestyle_change(); break;
        case 7: _t->combobox_detecttools_change(); break;
        case 8: _t->slider_value_change(); break;
        case 9: _t->get_abort((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->checkbox_filter(); break;
        default: ;
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
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
