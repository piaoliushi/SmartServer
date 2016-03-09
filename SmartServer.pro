#-------------------------------------------------
#
# Project created by QtCreator 2016-02-22T09:26:13
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SmartServer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    net/SvcMgr.cpp \
    net/session.cpp \
    net/message.cpp \
    net/io_service_pool.cpp \
    net/server/server_work.cpp \
    net/server/ServerMgr.cpp \
    net/server/connect_subSvc_handler.cpp \
    net/server/connect_handler.cpp \
    net/server/http/request_parser.cpp \
    net/server/http/RequestHandlerFactory.cpp \
    net/server/http/RequestHandler.cpp \
    net/server/http/request_handler.cpp \
    net/server/http/reply.cpp \
    net/server/http/mime_types.cpp \
    net/server/http/connection.cpp \
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
    net/client/dev_message/Hx_message.cpp \
    net/client/dev_message/Envir_message.cpp \
    net/client/dev_message/Electric_message.cpp \
    net/client/dev_message/Eda_message.cpp \
    net/client/dev_message/base_message.cpp \
    net/client/dev_message/104/iec104.cpp \
    protocol/protocol.pb.cc \
    database/dbmanager.cpp \
    qnotifyhandler.cpp \
    LocalConfig.cpp \
    StationConfig.cpp \
    net/server/local_server.cpp

HEADERS  += \
    mainwindow.h \
    net/Worker.h \
    net/UserWork.h \
    net/taskqueue.h \
    net/SvcMgr.h \
    net/share_ptr_object_define.h \
    net/session.h \
    net/message.h \
    net/io_service_pool.h \
    net/config.h \
    net/server/server_work.h \
    net/server/ServerMgr.h \
    net/server/connect_subSvc_handler.h \
    net/server/connect_handler.h \
    net/server/http/server.hpp \
    net/server/http/request_parser.hpp \
    net/server/http/RequestHandlerFactory.h \
    net/server/http/RequestHandler.h \
    net/server/http/request_handler.hpp \
    net/server/http/request.hpp \
    net/server/http/reply.hpp \
    net/server/http/mime_types.hpp \
    net/server/http/header.hpp \
    net/server/http/connection.hpp \
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
    net/client/dev_message/Hx_message.h \
    net/client/dev_message/Envir_message.h \
    net/client/dev_message/Electric_message.h \
    net/client/dev_message/Eda_message.h \
    net/client/dev_message/base_message.h \
    net/client/dev_message/104/iec104_types.h \
    net/client/dev_message/104/iec104.h \
    protocol/protocol.pb.h \
    MsgDefine.h \
    include.h \
    ErrorCode.h \
    DataType.h \
    database/dbmanager.h \
    qnotifyhandler.h \
    StructDef.h \
    LocalConfig.h \
    StationConfig.h \
    net/server/local_server.h

FORMS    += mainwindow.ui



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
}
INCLUDEPATH += /usr/local/boost_1_48_0
