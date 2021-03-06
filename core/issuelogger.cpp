#include <QSqlError>
#include <QFile>
#include <QDataStream>
#include <QDir>
#include <QDataStream>
#include <QUrl>
#include <fstream>
#include <iomanip>
#include "kernel.h"
#include "ilwisdata.h"
#include "ilwiscontext.h"
#include "errorobject.h"
#include "issuelogger.h"

using namespace Ilwis;

std::map<std::thread::id, bool> IssueLogger::_silentThreads;

IssueObject::IssueObject(QObject* parent) : QObject(parent)
{
}

Ilwis::IssueObject::IssueObject(const QString &message, int it, quint64 id, QObject *parent) : QObject(parent)
{
    _message = message;
    _itype = it;
    _itime = QDateTime::currentDateTime();
    _id = id;
}

Ilwis::IssueObject::IssueObject(const IssueObject& issueobj) : QObject() {
    _itime = issueobj._itime;
    _message = issueobj._message;
    _id = issueobj._id;
    _line = issueobj._line;
    _func = issueobj._func;
    _file = issueobj._file;

    _itype = issueobj._itype;
}

IssueObject& IssueObject::operator=(const IssueObject& issueobj) {
    _itime = issueobj._itime;
    _message = issueobj._message;
    _id = issueobj._id;
    _line = issueobj._line;
    _func = issueobj._func;
    _file = issueobj._file;

    _itype = issueobj._itype;

    return *this;
}

QString Ilwis::IssueObject::message() const
{
    return _message;
}

QDateTime Ilwis::IssueObject::time() const
{
    return _itime;
}

int Ilwis::IssueObject::type() const
{
    return _itype;
}

QString Ilwis::IssueObject::logMessage(Ilwis::IssueObject::LogMessageFormat) const{
    return QString("%1: (%2) %3").arg(type2String(), _itime.toString(), _message);
}

int IssueObject::codeLine() const {
    return _line;
}

QString IssueObject::codeFunc() const {
    return _func;
}

QString IssueObject::codeFile() const {
    return _file;
}

void IssueObject::addCodeInfo(int line, const QString &func, const QString &file)
{
    _line = line;
    _func = func;
    _file = file;
}

void IssueObject::stream(std::ofstream& stream, LogMessageFormat frmt) {
    stream << std::setw(4) << _id << " ; " << std::setw(9) << type2String().toStdString() << " ; " << std::setw(27)<<_itime.toString().toStdString() << " ; " << _message.toStdString() << std::endl;
    if ( frmt == lmCODE) {
        stream << std::setw(4) << _id << " ; " << _line << " : " << _func.toStdString() << " ; " << _file.toStdString() << std::endl;
    }
}



quint64 IssueObject::id() const
{
    return _id;
}

QString IssueObject::type2String() const{
    switch(_itype) {
    case itCritical:
        return "Critical";
    case itError:
        return "Error";
    case itWarning:
        return "Warning";
    case itMessage:
        return "Message";
    case itDebug:
        return "Debug";
    }
    return "Text";
}

//---------------------------------------------------------------------------
IssueLogger::IssueLogger(QObject *parent) : QObject(parent), _repeatCount(0)
{
    QString apploc= context()->ilwisFolder().absoluteFilePath();
    apploc += "/log";
    QDir dir(apploc);
    if ( !dir.exists())
        dir.mkdir(apploc);
    QString rlogFilePath = apploc + "/logfile.txt";
    QString clogFilePath = apploc + "/logfile_ext.txt";
    _logFileRegular.open(rlogFilePath.toLatin1());
    _logFileCode.open(clogFilePath.toLatin1());
}

IssueLogger::~IssueLogger()
{
    if (_logFileCode.is_open())
        _logFileCode.close();
    if ( _logFileRegular.is_open())
        _logFileRegular.close();
}

quint64 IssueLogger::log(const QString &message, int it)
{
    ++_repeatCount;
    std::thread::id id = std::this_thread::get_id();
    if ( _silentThreads.find(id)!= _silentThreads.end() && it != IssueObject::itCritical){
        _repeatCount = 0;
        return i64UNDEF;
    }


    if ( _lastmessage == message && _repeatCount == 10) {
        return _issueId;
    } else {
        // Mechanism to prevent(some) 'stuck in a loop' kind of errors.
        // this should basically be solved at the place the loop is happening but oversights happen.
        // This is a last fallback to break the loop without the need to stop the process
        if ( _repeatCount > 10 && _lastmessage != message ){
            _issues.enqueue(IssueObject(QString("Message repeated %1 times").arg(_repeatCount), it, _issueId));
            _repeatCount = 0;
            throw ErrorObject(QString("Error message cascade : %1").arg(message));
        }
        _repeatCount = 0;
    }

    _issues.enqueue(IssueObject(message, it, _issueId));
    if ( _lastmessage == message)
        return _issueId;
    IssueObject& obj = _issues.back();
    if ( _logFileRegular.is_open()) {
        obj.stream(_logFileRegular, IssueObject::lmREGULAR);
    }
    if ( hasType(context()->runMode(),rmCOMMANDLINE)){
        if ( it == IssueObject::itError)
            std::cerr << message.toStdString() << "\n";
    }
    emit updateIssues(obj);

    _lastmessage = message;
    return _issueId++;
}

quint64 IssueLogger::log(const QString& objectName, const QString &message, int it)
{
    QString newmessage = objectName + ":" + message;
    return log(newmessage, it);
}

void IssueLogger::addCodeInfo(quint64 issueid, int line, const QString &func, const QString &file)
{
    for(auto iter=_issues.begin(); iter != _issues.end(); ++iter) {
        IssueObject& issue = *iter;
        if ( issue.id() == issueid) {
            issue.addCodeInfo(line, func, file);
            if ( _logFileCode.is_open()) {
                issue.stream(_logFileCode, IssueObject::lmCODE);
            }
            break;
        }
    }
}

bool IssueLogger::silent() const
{
    std::thread::id id = std::this_thread::get_id();
    auto iter = _silentThreads.find(id);
    if ( iter != _silentThreads.end())
        return iter->second;
    return false;
}

void IssueLogger::silent(bool yesno)
{
    std::thread::id id = std::this_thread::get_id();
    if ( yesno == false){
        auto iter = _silentThreads.find(id) ;
        if (iter != _silentThreads.end())
            _silentThreads.erase(iter);
    }else
        _silentThreads[id] = true;
}

quint64 IssueLogger::logSql(const QSqlError &err)
{
   return log(err.text(), IssueObject::itError);
}

IssueObject::IssueType IssueLogger::maxIssueLevel() const
{
    int type = IssueObject::itNone;
    foreach(IssueObject issue, _issues) {
        type |= issue.type();
    }
    if ( type & IssueObject::itCritical)
        return IssueObject::itCritical;
    if ( type & IssueObject::itError)
        return IssueObject::itError;
    if ( type & IssueObject::itWarning)
        return IssueObject::itWarning;
    if ( type & IssueObject::itMessage)
        return IssueObject::itMessage;
    return IssueObject::itNone;
}

void IssueLogger::copy(QList<IssueObject> &other)
{
  foreach(IssueObject issue, _issues) {
      other.append(issue);
  }
}

QString IssueLogger::popfirst(int tp) {
    if ( tp != IssueObject::itAll){
        for(auto iter= --_issues.end(); iter != _issues.begin(); --iter ){
            if (hasType((*iter).type(), tp)){
                QString mes = (*iter).message();
                _issues.erase(iter);
                return mes;
            }
        }
    }
    if ( _issues.size() > 0)
        return _issues.dequeue().message();
    return "?";
}

QString IssueLogger::poplast(int tp) {
    if ( tp != IssueObject::itAll){
        for(auto iter= _issues.begin(); iter != _issues.end(); ++iter ){
            if (hasType((*iter).type(), tp)){
                QString mes = (*iter).message();
                _issues.erase(iter);
                return mes;
            }
        }
    }
    if ( _issues.size() >0 )
        return _issues.takeLast().message();
    return "?";
}

void IssueLogger::clear() {
    _issues.clear();
}



