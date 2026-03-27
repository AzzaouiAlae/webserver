#include "MultipartData.hpp"

FormData::FormData(string &data) : _rowData(&data), fd(-1), startIdx(0), size(0), isComplete(false), isHeaderParsed(false)
{
}

int FormData::openFile()
{
    if (filename.empty() || fd != -1)
        return fd;
    fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
    return fd;
}

FormData::~FormData()
{
    if (fd != -1)
    {
        close(fd);
    }
}

char *FormData::data()
{
    if (_rowData)
    {
        return (char *)(_rowData->data() + startIdx);
    }
    return NULL;
}

MultipartData::MultipartData()
{
    _status = eParseBoundary;
    _idx = 0;
    _startHeaderIdx = 0;
    _contentDispositionParsed = false;
    _contentTypeParsed = false;
    _isIncompleteParse = false;
    _currentFormData = NULL;
    DDEBUG("MultipartData") << "MultipartData: constructed, initial status=eParseBoundary";
}

MultipartData::~MultipartData()
{
    while (!_formData.empty()) {
        FormData *formData = _formData.front();
        _formData.pop();
        delete formData;
    }
    if (_currentFormData != NULL) {
        delete _currentFormData;
    }
}

void MultipartData::_parseContentDisposition(string &data)
{
    size_t boundaryPos = data.find("\r\n", _idx);
    if (boundaryPos == string::npos)
        return;
    while (_idx < boundaryPos)
    {
        size_t endPos = data.find(";", _idx);
        if (endPos == string::npos || endPos > boundaryPos)
        {
            endPos = boundaryPos;
        }
        if (endPos == string::npos || endPos > boundaryPos)
        {
            if (boundaryPos == string::npos)
            {
                _isIncompleteParse = true;
                return;
            }
            endPos = boundaryPos;
        }
        string name = data.substr(_idx, endPos - _idx);
        Utility::trim(name, " ");
        size_t pos = name.find("=");
        if (pos != string::npos)
        {
            string key = name.substr(0, pos);
            string value = name.substr(pos + 1);
            Utility::trim(value, "\"");
            if (key == "name")
                _currentFormData->name = value;
            else if (key == "filename")
                _currentFormData->filename = Utility::addRandomStr(value);
        }
        _idx = endPos + 1;
    }
    _idx++;
    _contentDispositionParsed = true;
    DDEBUG("MultipartData") << "MultipartData::_parseContentDisposition: parsed disposition, name='" << _currentFormData->name << "' filename='" << _currentFormData->filename << "'";
}

void MultipartData::_parseContentType(string &data)
{
    size_t pos = data.find("\r\n", _idx);
    if (pos == string::npos)
        return;
    _currentFormData->contentType = data.substr(_idx, pos - _idx);
    Utility::trim(_currentFormData->contentType, " ");
    _idx = pos + 2;
    _contentTypeParsed = true;
    DDEBUG("MultipartData") << "MultipartData::_parseContentType: contentType='" << _currentFormData->contentType << "'";
}

void MultipartData::_parseBoundry(string &data, const string &boundary)
{
    if (data.length() - _idx < boundary.length() + 2)
        return;
    if (data.find(boundary, _idx) == _idx)
    {
        _idx += boundary.length() + 2;
        if (data.substr(_idx - 2, 2) == "--")
            _status = eComplete;
        else {
            _status = eParseHeader;
            _currentFormData = new FormData(data);
        }
    }
    else
        _status = eError;
}

void MultipartData::_parseHeader(string &data)
{
    size_t boundaryPos = data.find("\r\n\r\n", _idx);
    if (boundaryPos == string::npos)
    {
        if (data.length() - _startHeaderIdx > MAX_HEADER_SIZE)
        {
            _status = eError;
            return;
        }
        boundaryPos = data.find("\r\n", _idx);
        if (boundaryPos == string::npos)
        {
            return;
        }
    }
    else if (boundaryPos - _startHeaderIdx > MAX_HEADER_SIZE)
    {
        _status = eError;
        return;
    }
    while (_idx < boundaryPos)
    {
        size_t pos = data.find(":", _idx);
        if (pos > boundaryPos || pos == string::npos)
            return;
        string key = data.substr(_idx, pos - _idx);
        _idx = pos + 1;
        if (key == "Content-Disposition")
            _parseContentDisposition(data);
        else if (key == "Content-Type")
            _parseContentType(data);
        else
            _idx = data.find("\r\n", _idx) + 2;
        if ((_contentDispositionParsed && _contentTypeParsed) ||
            _idx >= boundaryPos)
        {
            _currentFormData->isHeaderParsed = true;
            _status = eParseBody;
            _idx = boundaryPos + 4;
            DDEBUG("MultipartData") << "MultipartData::_parseHeader: header parsed, switching to eParseBody";
            return;
        }
        if (_isIncompleteParse)
            return;
    }
}

void MultipartData::_parseBody(string &data, const string &boundary)
{
    size_t boundaryPos = data.find(boundary, _idx);
    if (boundaryPos == string::npos)
    {
        _currentFormData->startIdx = _idx;
        _currentFormData->size = data.length() - _idx;
        _currentFormData->isComplete = false;
        _isIncompleteParse = true;
        DDEBUG("MultipartData") << "MultipartData::_parseBody: body incomplete, startIdx=" << _currentFormData->startIdx << " size=" << _currentFormData->size;
    }
    else
    {
        _currentFormData->startIdx = _idx;
        _currentFormData->size = boundaryPos - _idx - 2;
        _currentFormData->isComplete = true;
        _idx = boundaryPos;
        _status = eParseBodyComplete;
        DDEBUG("MultipartData") << "MultipartData::_parseBody: body complete, size=" << _currentFormData->size << " status=" << _status;
    }
}

void MultipartData::_parseBodyComplete()
{
    DDEBUG("MultipartData") << "1-_parseBodyComplete: "
			<< "is formData queue empty? " 
			<< (getFormData().empty() ? "yes" : "no")
			<< ", name: " << _currentFormData->name
			<< ", filename: " << _currentFormData->filename
			<< ", contentType: " << _currentFormData->contentType
			<< ", startIdx: " << _currentFormData->startIdx
			<< ", size: " << _currentFormData->size
			<< ", isComplete: " << _currentFormData->isComplete
			<< ", isHeaderParsed: " << _currentFormData->isHeaderParsed
			<< ", data: \n" << string(_currentFormData->data(), _currentFormData->size);

    _formData.push(_currentFormData);
    _contentDispositionParsed = false;
    _contentTypeParsed = false;
    _currentFormData = NULL;
    _startHeaderIdx = _idx;
    _status = eParseBoundary;
}


int MultipartData::Parse(string &data, const string &boundary)
{
    if (_status == eError || _status == eComplete)
        return _status;
    
    if (_currentFormData)
        DDEBUG("MultipartData")
            << "MultipartData::Parse: enter, dataLen=" << data.length()
            << ", filename=" << _currentFormData->filename
            << ", startIdx=" << _currentFormData->startIdx
            << ", size=" << _currentFormData->size
            << ", name=" << _currentFormData->name
            << ", idx=" << _idx
            << ", status=" << _status
            << ", contentDispositionParsed=\n"
            << string(_currentFormData->data(), _currentFormData->size);
    
    if (_currentFormData == NULL)
        _currentFormData = new FormData(data);
    do
    {
        if (_status == eParseBoundary) {
            _parseBoundry(data, boundary);
            if (_status == eParseBoundary) {
                break;
            }
        }
        if (_status == eParseHeader)
            _parseHeader(data);
        if (_status == eParseBody)
            _parseBody(data, boundary);
        if (_status == eParseBodyComplete)
            _parseBodyComplete();
    } while (_status == eParseBoundary);
    DDEBUG("MultipartData") << "MultipartData::Parse: exit status=" << _status << " queued=" << _formData.size();
    return _status;
}

queue<FormData *> &MultipartData::getFormData()
{
    DDEBUG("MultipartData") << "MultipartData::getFormData: returning queue with size=" << _formData.size();
    return _formData;
}

FormData *MultipartData::getCurrentFormData()
{
    DDEBUG("MultipartData") << "MultipartData::getCurrentFormData: returning currentFormData (headerParsed=" << (_currentFormData ? _currentFormData->isHeaderParsed : false) << ")";
    return _currentFormData;
}

size_t MultipartData::getLastIndex()
{
    DDEBUG("MultipartData") << "MultipartData::getLastIndex: idx=" << _idx;
    return _idx;
}

void MultipartData::resetIndex()
{
    DDEBUG("MultipartData") << "MultipartData::resetIndex: resetting idx and currentFormData startIdx";
    _idx = 0;
    if (_currentFormData)
        _currentFormData->startIdx = 0;
}
