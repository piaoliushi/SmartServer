#-------------------------------------------------
#
# Project created by QtCreator 2016-02-22T09:26:13
#
#-------------------------------------------------

QT       += core gui sql network

QTPLUGIN+=qsqlpsql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = SmartServer
TEMPLATE = app
TRANSLATIONS = SmartServer_CN.ts

CONFIG+=precompile_header
PRECOMPILED_HEADER=stable.h
CONFIG += console
SOURCES += main.cpp\
        mainwindow.cpp \
    net/SvcMgr.cpp \
    net/message.cpp \
    net/io_service_pool.cpp \
    net/server/server_work.cpp \
    net/server/ServerMgr.cpp \
    net/server/connect_handler.cpp \
    net/server/http/RequestHandlerFactory.cpp \
    net/server/http/RequestHandler.cpp \
    net/server/http/CommonPrecomp.cpp \
    net/client/MsgHandleAgentImpl.cpp \
    net/client/MsgHandleAgent.cpp \
    net/client/device_session.cpp \
    net/client/device_message.cpp \
    net/client/DevClientMgr.cpp \
    net/client/DevClient.cpp \
    net/client/client_work.cpp \
    net/client/dev_message/Envir_message.cpp \
    net/client/dev_message/Electric_message.cpp \
    net/client/dev_message/base_message.cpp \
    net/client/dev_message/104/iec104.cpp \
    protocol/protocol.pb.cc \
    qnotifyhandler.cpp \
    LocalConfig.cpp \
    StationConfig.cpp \
    net/server/local_server.cpp \
    qsvcstatepage.cpp \
    qdevstatepage.cpp \
    qclientstatepage.cpp \
    qsysteminfopage.cpp \
    nettool.cpp \
    utility.cpp \
    protocol/bohui_protocol.cpp \
    database/DataBaseOperation.cpp \
    net/net_session.cpp \
    net/client/http_request_session.cpp \
    net/client/urdl.cpp \
    net/client/dev_message/Tsmt_message.cpp \
    net/client/dev_message/transmmiter/Transmmiter.cpp \
    net/client/dev_message/transmmiter/CDtransmmiter.cpp \
    net/client/dev_message/transmmiter/GmeTransmmit.cpp \
    net/client/dev_message/Timer_message.cpp \
    net/client/dev_message/transmmiter/LngdTransmmit.cpp \
    net/client/dev_message/Link_message.cpp \
    net/client/dev_message/transmmiter/ShTransmmit.cpp \
    net/client/http_work.cpp \
    net/client/dev_message/Media_message.cpp \
    database/ConnectionPool.cpp \
    net/client/parse_ass_device.cpp \
    net/client/dev_message/transmmiter/AhhxTransmmit.cpp \
    signalApp/qtlocalpeer.cpp \
    signalApp/qtlockedfile.cpp \
    signalApp/qtlockedfile_unix.cpp \
    signalApp/qtlockedfile_win.cpp \
    signalApp/qtsingleapplication.cpp

HEADERS  += \
    mainwindow.h \
    net/Worker.h \
    net/UserWork.h \
    net/taskqueue.h \
    net/SvcMgr.h \
    net/share_ptr_object_define.h \
    net/message.h \
    net/io_service_pool.h \
    net/config.h \
    net/server/server_work.h \
    net/server/ServerMgr.h \
    net/server/connect_handler.h \
    net/server/http/RequestHandlerFactory.h \
    net/server/http/RequestHandler.h \
    net/server/http/CommonPrecomp.h \
    net/client/MsgHandleAgentImpl.h \
    net/client/MsgHandleAgent.h \
    net/client/device_session.h \
    net/client/device_message.h \
    net/client/DevClientMgr.h \
    net/client/DevClient.h \
    net/client/dev_message/Envir_message.h \
    net/client/dev_message/Electric_message.h \
    net/client/dev_message/base_message.h \
    net/client/dev_message/104/iec104_types.h \
    net/client/dev_message/104/iec104.h \
    protocol/protocol.pb.h \
    MsgDefine.h \
    include.h \
    ErrorCode.h \
    DataType.h \
    qnotifyhandler.h \
    StructDef.h \
    LocalConfig.h \
    StationConfig.h \
    net/server/local_server.h \
    qsvcstatepage.h \
    qdevstatepage.h \
    qclientstatepage.h \
    qsysteminfopage.h \
    nettool.h \
    rapidxml/rapidxml_utils.hpp \
    rapidxml/rapidxml_print.hpp \
    rapidxml/rapidxml.hpp \
    utility.h \
    protocol/bohui_protocol.h \
    protocol/bohui_const_define.h \
    DataTypeDefine.h \
    database/DataBaseOperation.h \
    net/net_session.h \
    net/client/http_request_session.h \
    net/client/dev_message/Tsmt_message.h \
    net/client/dev_message/transmmiter/Transmmiter.h \
    net/client/dev_message/transmmiter/CDtransmmiter.h \
    net/client/dev_message/transmmiter/GmeTransmmit.h \
    net/client/dev_message/Timer_message.h \
    net/client/dev_message/transmmiter/LngdTransmmit.h \
    net/client/dev_message/Link_message.h \
    net/client/dev_message/transmmiter/ShTransmmit.h \
    net/client/http_work.h \
    stable.h \
    net/client/dev_message/Media_message.h \
    database/ConnectionPool.h \
    net/client/parse_ass_device.h \
    net/client/dev_message/transmmiter/AhhxTransmmit.h \
    signalApp/qtlocalpeer.h \
    signalApp/qtlockedfile.h \
    signalApp/qtsingleapplication.h

FORMS    += mainwindow.ui

DEFINES+=BOOST_NETWORK_NO_LIB
DEFINES+=URDL_DISABLE_SSL
DEFINES+=URDL_NO_LIB
#DEFINES+=BOOST_EXCEPTION_DISABLE

linux-g++ {

LIBS += /usr/local/boost_1_48_0/boost_sdk_ubuntu/lib/libboost_system-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_ubuntu/lib/libboost_thread-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_ubuntu/lib/libboost_date_time-mt.a
#LIBS += /usr/local/postgresql_ubuntu_build/lib/libpq.a
#LIBS += /usr/local/postgresql_ubuntu_build/lib/libpgport.a
#LIBS += /usr/local/postgresql_ubuntu_build/lib/libpgtypes.a
#LIBS += /usr/local/postgresql_ubuntu_build/lib/libecpg.a
#LIBS += /usr/local/postgresql_ubuntu_build/lib/libpgcommon.a
#LIBS += /usr/local/postgresql_ubuntu_build/lib/libecpg_compat.a
LIBS += /usr/local/protobuf-2.6.1/ubuntu_build/lib/libprotobuf.a
LIBS += /usr/local/protobuf-2.6.1/ubuntu_build/lib/libprotoc.a
LIBS += /usr/local/protobuf-2.6.1/ubuntu_build/lib/libprotobuf-lite.a
LIBS += /home/piaoliu/Project/build-snmplib-x86_linux-Debug/libsnmplib.a
#INCLUDEPATH += /usr/local/postgresql_ubuntu_build/include
INCLUDEPATH += /usr/local/protobuf-2.6.1/ubuntu_build/include
INCLUDEPATH += /usr/local/boost_1_48_0/urdl-0.1/include
INCLUDEPATH += /home/piaoliu/Project/snmplib
INCLUDEPATH += /usr/local/boost_1_48_0
}
else: !win32{
DEFINES+=ARM_LINUX_DEF

LIBS += /usr/local/boost_1_48_0/boost_sdk_arm/lib/libboost_system-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_arm/lib/libboost_thread-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_arm/lib/libboost_date_time-mt.a
#LIBS += /usr/local/postgresql_arm_build/lib/libpq.a
#LIBS += /usr/local/postgresql_arm_build/lib/libpgport.a
#LIBS += /usr/local/postgresql_arm_build/lib/libpgtypes.a
#LIBS += /usr/local/postgresql_arm_build/lib/libecpg.a
#LIBS += /usr/local/postgresql_arm_build/lib/libpgcommon.a
#LIBS += /usr/local/postgresql_arm_build/lib/libecpg_compat.a
LIBS += /usr/local/protobuf-2.6.1/arm_build/lib/libprotobuf.a
LIBS += /usr/local/protobuf-2.6.1/arm_build/lib/libprotoc.a
LIBS += /usr/local/protobuf-2.6.1/arm_build/lib/libprotobuf-lite.a
LIBS += /home/piaoliu/Project/build-snmplib-arm_linux-Release/libsnmplib.a
#INCLUDEPATH += /usr/local/postgresql_arm_build/include
INCLUDEPATH += /usr/local/protobuf-2.6.1/arm_build/include
INCLUDEPATH += /usr/local/boost_1_48_0/urdl-0.1/include
INCLUDEPATH += /home/piaoliu/Project/snmplib
INCLUDEPATH += /usr/local/boost_1_48_0
}
else:{

QMAKE_CXXFLAGS +=-Zm300
win32:CONFIG(release, debug|release): {
LIBS += -LD:/ThirdSdk/protobuf-2.6.1/lib -llibprotobuf -llibprotoc -llibprotobuf-lite
LIBS += -LD:/ThirdSdk/snmp++/lib -lsnmplib
LIBS += -LD:/ThirdSdk/boost_1_48_0/stage/lib -llibboost_system-vc100-mt-1_48 -llibboost_date_time-vc100-mt-1_48 -llibboost_thread-vc100-mt-1_48
}
else:win32:CONFIG(debug, debug|release): {
LIBS += -LD:/ThirdSdk/protobuf-2.6.1/lib/ -llibprotobufD -llibprotocD -llibprotobuf-liteD
LIBS += -LD:/ThirdSdk/snmp++/lib/ -lsnmplib
LIBS += -LD:/ThirdSdk/boost_1_48_0/stage/lib -llibboost_system-vc100-mt-gd-1_48 -llibboost_date_time-vc100-mt-gd-1_48 -llibboost_thread-vc100-mt-gd-1_48
#LIBS += -LD:/ThirdSdk/Visual Leak Detector/lib/Win32/ -lvld
}
INCLUDEPATH += 'D:/ThirdSdk/boost_1_48_0'
INCLUDEPATH += 'D:/ThirdSdk/protobuf-2.6.1/protobuf-2.6.1/src'
INCLUDEPATH += 'D:/ThirdSdk/boost_1_48_0/boost/urdl-0.1/include'
INCLUDEPATH += 'D:/ThirdSdk/snmp++/include'
#INCLUDEPATH += 'D:/ThirdSdk/Visual Leak Detector/include'
}


OTHER_FILES += \
    rapidxml/manual.html

RESOURCES += \
    image/resource.qrc

DISTFILES += \
    SmartServer_CN.ts

CONFIG(debug, debug|release) {
  #TARGET = $$join(TARGET,,,d)
  contains(TEMPLATE, "lib") {
    DESTDIR = ../output/debug/lib
    DLLDESTDIR = ../output/debug/bin
  } else {
    DESTDIR = ../output/debug/bin
  }
  OBJECTS_DIR = ./debug/obj
  MOC_DIR = ./debug/moc
} else {
  contains(TEMPLATE, "lib") {
    DESTDIR = ../output/release/lib
    DLLDESTDIR = ../output/release/bin
  } else {
    DESTDIR = ../output/release/bin
  }
  OBJECTS_DIR = ./release/obj
  MOC_DIR = ./release/moc
}
