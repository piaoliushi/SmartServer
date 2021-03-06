#-------------------------------------------------
#
# Project created by QtCreator 2016-02-22T09:26:13
#
#-------------------------------------------------

QT += core gui sql network
CONFIG += serialport

QTPLUGIN+=qsqlpsql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = SmartServer
TEMPLATE = app
TRANSLATIONS = SmartServer_CN.ts
RC_FILE = SmartServer.rc

CONFIG+=precompile_header

PRECOMPILED_HEADER=stable.h
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
    signalApp/qtsingleapplication.cpp \
    net/server/websocket_server.cpp \
    net/client/dev_message/transmmiter/dexintransmmit.cpp \
    net/client/dev_message/transmmiter/gsbrtransmmit.cpp \
    net/client/dev_message/transmmiter/GlsqTransmmit.cpp \
    net/client/dev_message/antenna_message.cpp \
    gpschecktime.cpp \
    sms/gsms.cpp \
    net/client/dev_message/transmmiter/ckangtetransmmit.cpp \
    net/client/dev_message/transmmiter/zctransmmit.cpp \
    net/client/dev_message/transmmiter/harristransmmit.cpp \
    net/client/dev_message/transmmiter/sgtransmmit.cpp \
    net/client/dev_message/transmmiter/bgtransmmiter.cpp \
    net/client/dev_message/transmmiter/hgtransmmitr.cpp \
    net/client/dev_message/transmmiter/hctransmmiter.cpp \
    net/client/dev_message/transmmiter/rstransmmit.cpp \
    net/client/dev_message/transmmiter/rvrtransmmit.cpp \
    net/client/dev_message/transmmiter/astransmmit.cpp \
    yaolog.cpp \
    net/client/dev_message/transmmiter/zttransmmit.cpp \
    net/client/dev_message/transmmiter/huakangtransmmit.cpp \
    net/client/dev_message/transmmiter/xttransmmit.cpp

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
    signalApp/qtsingleapplication.h \
    net/server/websocket_server.h \
    net/client/dev_message/transmmiter/dexintransmmit.h \
    net/client/dev_message/transmmiter/gsbrtransmmit.h \
    net/client/dev_message/transmmiter/GlsqTransmmit.h \
    net/client/dev_message/antenna_message.h \
    gpschecktime.h \
    sms/gsms.h \
    net/client/dev_message/transmmiter/ckangtetransmmit.h \
    net/client/dev_message/transmmiter/zctransmmit.h \
    net/client/dev_message/transmmiter/harristransmmit.h \
    net/client/dev_message/transmmiter/sgtransmmit.h \
    net/client/dev_message/transmmiter/bgtransmmiter.h \
    net/client/dev_message/transmmiter/hgtransmmitr.h \
    net/client/dev_message/transmmiter/hctransmmiter.h \
    net/client/dev_message/transmmiter/rstransmmit.h \
    net/client/dev_message/transmmiter/rvrtransmmit.h \
    net/client/dev_message/transmmiter/astransmmit.h \
    yaolog.h \
    net/client/dev_message/transmmiter/zttransmmit.h \
    net/client/dev_message/transmmiter/huakangtransmmit.h \
    net/client/dev_message/transmmiter/xttransmmit.h

FORMS    += mainwindow.ui

DEFINES+=BOOST_NETWORK_NO_LIB
DEFINES+=URDL_DISABLE_SSL
DEFINES+=URDL_NO_LIB
#DEFINES+=BOOST_EXCEPTION_DISABLE

linux-g++ {

LIBS += /usr/local/boost_1_48_0/boost_sdk_ubuntu/lib/libboost_system-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_ubuntu/lib/libboost_thread-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_ubuntu/lib/libboost_date_time-mt.a
LIBS += /usr/local/protobuf-2.6.1/ubuntu_build/lib/libprotobuf.a
LIBS += /usr/local/protobuf-2.6.1/ubuntu_build/lib/libprotoc.a
LIBS += /usr/local/protobuf-2.6.1/ubuntu_build/lib/libprotobuf-lite.a
LIBS += /home/piaoliu/Project/build-snmplib-x86_linux-Debug/libsnmplib.a
INCLUDEPATH += /usr/local/protobuf-2.6.1/ubuntu_build/include
INCLUDEPATH += /usr/local/boost_1_48_0/urdl-0.1/include
INCLUDEPATH += /home/piaoliu/Project/snmplib
INCLUDEPATH += /usr/local/boost_1_48_0
INCLUDEPATH += /usr/local/boost_1_48_0/boost
}
else: !win32{
DEFINES+=ARM_LINUX_DEF

LIBS += /usr/local/boost_1_48_0/boost_sdk_arm/lib/libboost_system-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_arm/lib/libboost_thread-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_arm/lib/libboost_date_time-mt.a
LIBS += /usr/local/protobuf-2.6.1/arm_build/lib/libprotobuf.a
LIBS += /usr/local/protobuf-2.6.1/arm_build/lib/libprotoc.a
LIBS += /usr/local/protobuf-2.6.1/arm_build/lib/libprotobuf-lite.a
LIBS += /home/piaoliu/Project/build-snmplib-arm_linux-Release/libsnmplib.a
INCLUDEPATH += /usr/local/protobuf-2.6.1/arm_build/include
INCLUDEPATH += /usr/local/boost_1_48_0/urdl-0.1/include
INCLUDEPATH += /home/piaoliu/Project/snmplib
INCLUDEPATH += /usr/local/boost_1_48_0
INCLUDEPATH += /usr/local/boost_1_48_0/boost
}
else:{

QMAKE_CXXFLAGS +=-Zm300
win32:CONFIG(release, debug|release): {
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
#CONFIG += console
LIBS += -lQtSerialPort
LIBS += -LD:/ThirdSdk/protobuf-2.6.1/lib/vs2010 -llibprotobuf -llibprotoc -llibprotobuf-lite
LIBS += -LD:/ThirdSdk/snmp++/lib -lsnmplib
LIBS += -LD:/ThirdSdk/boost_1_48_0/stage/lib -llibboost_system-vc100-mt-1_48 -llibboost_date_time-vc100-mt-1_48 -llibboost_thread-vc100-mt-1_48 -llibboost_chrono-vc100-mt-1_48
}
else:win32:CONFIG(debug, debug|release): {
#CONFIG += console
LIBS += -lQtSerialPortd
LIBS += -LD:/ThirdSdk/protobuf-2.6.1/lib/vs2010 -llibprotobufD -llibprotocD -llibprotobuf-liteD
LIBS += -LD:/ThirdSdk/snmp++/lib/ -lsnmplib
LIBS += -LD:/ThirdSdk/boost_1_48_0/stage/lib -llibboost_system-vc100-mt-gd-1_48 -llibboost_date_time-vc100-mt-gd-1_48 -llibboost_thread-vc100-mt-gd-1_48
#LIBS += -LD:/ThirdSdk/Visual Leak Detector/lib/Win32/ -lvld
}
INCLUDEPATH += 'D:/ThirdSdk/boost_1_48_0'
INCLUDEPATH += 'D:/ThirdSdk/boost_1_48_0/boost'
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
    SmartServer_CN.ts \
    SmartServer.ico \
    SmartServer.rc \
    libdvbpsi/src/stamp-h.in \
    libdvbpsi/src/Makefile.am

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
