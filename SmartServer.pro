#-------------------------------------------------
#
# Project created by QtCreator 2016-02-22T09:26:13
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = SmartServer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    net/SvcMgr.cpp \
    net/message.cpp \
    net/io_service_pool.cpp \
    net/server/server_work.cpp \
    net/server/ServerMgr.cpp \
    net/server/connect_subSvc_handler.cpp \
    net/server/connect_handler.cpp \
    net/server/http/RequestHandlerFactory.cpp \
    net/server/http/RequestHandler.cpp \
    net/server/http/CommonPrecomp.cpp \
    net/client/SvcClient.cpp \
    net/client/MsgHandleAgentImpl.cpp \
    net/client/MsgHandleAgent.cpp \
    net/client/device_session.cpp \
    net/client/device_message.cpp \
    net/client/DevClientMgr.cpp \
    net/client/DevClient.cpp \
    net/client/client_work.cpp \
    net/client/client_session.cpp \
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
    net/client/dev_message/Tsmt_message.cpp

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
    net/server/connect_subSvc_handler.h \
    net/server/connect_handler.h \
    net/server/http/RequestHandlerFactory.h \
    net/server/http/RequestHandler.h \
    net/server/http/CommonPrecomp.h \
    net/client/SvcClient.h \
    net/client/MsgHandleAgentImpl.h \
    net/client/MsgHandleAgent.h \
    net/client/device_session.h \
    net/client/device_message.h \
    net/client/DevClientMgr.h \
    net/client/DevClient.h \
    net/client/client_work.h \
    net/client/client_session.h \
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
    net/client/dev_message/Tsmt_message.h

FORMS    += mainwindow.ui

DEFINES+=BOOST_NETWORK_NO_LIB
DEFINES+=URDL_DISABLE_SSL

linux-g++ {

LIBS += /usr/local/boost_1_48_0/boost_sdk_ubuntu/lib/libboost_system-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_ubuntu/lib/libboost_thread-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_ubuntu/lib/libboost_date_time-mt.a
LIBS += /usr/local/postgresql_ubuntu_build/lib/libpq.a
LIBS += /usr/local/postgresql_ubuntu_build/lib/libpgport.a
LIBS += /usr/local/postgresql_ubuntu_build/lib/libpgtypes.a
LIBS += /usr/local/postgresql_ubuntu_build/lib/libecpg.a
LIBS += /usr/local/postgresql_ubuntu_build/lib/libpgcommon.a
LIBS += /usr/local/postgresql_ubuntu_build/lib/libecpg_compat.a
LIBS += /usr/local/protobuf-2.6.1/ubuntu_build/lib/libprotobuf.a
LIBS += /usr/local/protobuf-2.6.1/ubuntu_build/lib/libprotoc.a
LIBS += /usr/local/protobuf-2.6.1/ubuntu_build/lib/libprotobuf-lite.a
INCLUDEPATH += /usr/local/postgresql_ubuntu_build/include
INCLUDEPATH += /usr/local/protobuf-2.6.1/ubuntu_build/include
INCLUDEPATH += /usr/local/boost_1_48_0/urdl-0.1/include
}
else: {
LIBS += /usr/local/boost_1_48_0/boost_sdk_arm/lib/libboost_system-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_arm/lib/libboost_thread-mt.a
LIBS += /usr/local/boost_1_48_0/boost_sdk_arm/lib/libboost_date_time-mt.a
LIBS += /usr/local/postgresql_arm_build/lib/libpq.a
LIBS += /usr/local/postgresql_arm_build/lib/libpgport.a
LIBS += /usr/local/postgresql_arm_build/lib/libpgtypes.a
LIBS += /usr/local/postgresql_arm_build/lib/libecpg.a
LIBS += /usr/local/postgresql_arm_build/lib/libpgcommon.a
LIBS += /usr/local/postgresql_arm_build/lib/libecpg_compat.a
LIBS += /usr/local/protobuf-2.6.1/arm_build/lib/libprotobuf.a
LIBS += /usr/local/protobuf-2.6.1/arm_build/lib/libprotoc.a
LIBS += /usr/local/protobuf-2.6.1/arm_build/lib/libprotobuf-lite.a
INCLUDEPATH += /usr/local/postgresql_arm_build/include
INCLUDEPATH += /usr/local/protobuf-2.6.1/arm_build/include
INCLUDEPATH += /usr/local/boost_1_48_0/urdl-0.1/include
}
INCLUDEPATH += /usr/local/boost_1_48_0

OTHER_FILES += \
    rapidxml/manual.html

RESOURCES += \
    image/resource.qrc
