#include "Post.hpp"
#include "MulipartUploadStrategy.hpp"
#include "BuffersStrategy.hpp"

Post::Post(SocketIO *sock, Routing *router) : AMethod(sock, router)
{
	_path = &router->GetPath();
	_originalPath = &_path->OriginalPath;
	_uploadFd = -1;
	_contentBodySize = 0;
	_uploadedSize = 0;
	_multiplexer->ChangeToEpollIn(sock);
	DEBUG("Post")
		<< "Post initialized, socket fd="
		<< sock->GetFd();
	_buff = Utility::GetBuffer();
}

Post::~Post()
{
	if (_uploadFd != -1)
		close(_uploadFd);
	Utility::ReleaseBuffer(_buff);
}

void Post::_initPost()
{
	_resolvePath();

	if (!_isMethodAllowed("POST"))
		HandelErrorPages("405");
	else if (_router->GetPath().isRedirection())
		_sendRedirection();
	else if (_router->GetPath().isCGI())
		_handelCGI();
	else if (_originalPath->found && _originalPath->isFile)
		HandelErrorPages("405");
	if (_status != Post::eCreateResponse)
		return;
	if (_originalPath->isDir)
	{
		if (!_router->GetRequest().isMultipartFormData())
			Utility::addRandomStr(_filename);
	}
	_contentBodySize = _router->GetRequest().getcontentlen();
	if (_router->GetPath().getLocation() == NULL)
	{
		HandelErrorPages("403");
	}
	else if (_router->GetRequest().isMultipartFormData())
	{
		_readStrategy = new MulipartUploadStrategy(_sock, _router);
		if (_status == Post::eCreateResponse)
			_status = Post::eMultipartUpload;
	}
	else if (_router->GetRequest().isChunkedTransferEncoding())
	{
		_openUploadFile();
		if (_uploadFd == -1)
			HandelErrorPages("403");
		else {
			_uploadChunkedToDisk();
			if (_status == Post::eInitPost) 
				_status = Post::eChunkedUpload;
		}
	}
	else
	{
		_openUploadFile();
		if (_uploadFd == -1)
			HandelErrorPages("403");
		else
			_uploadFileToDisk();
	}
	if (_status == Post::eCreateResponse)
		_status = Post::eUploadFile;
}

bool Post::HandleResponse()
{
	_sock->SetStateByFd(_sock->GetFd());

	DEBUG("Post")
		<< "Socket fd: " << _sock->GetFd()
		<< ", Post::HandleResponse() start";
	DDEBUG("Post") << "Socket fd: " << _sock->GetFd()
				   << ", uploadedSize=" << _uploadedSize
				   << ", contentBodySize=" << _contentBodySize
				   << ", status=" << _status;

	if (_sock->isTimeOut())
		_createTimeoutResponse();
	else if (_readStrategy && _readStrategy->GetStatus() == AStrategy::eContinue)
	{
		_executeStrategy(_readStrategy);
		if (_readStrategy->GetStatus() == AStrategy::eComplete)
			_createPostResponse();
	}
	else if (_sendStrategy && _sendStrategy->GetStatus() == AStrategy::eContinue)
	{
		_executeStrategy(_sendStrategy);
		if (_sendStrategy->GetStatus() == AStrategy::eComplete)
			_status = eComplete;
	}
	else if (_status == Post::eInitPost)
		_initPost();
	else if (_status == Post::eUploadFile)
		_uploadFileToDisk();
	else if (_status == Post::eMultipartUpload)
		_handleMultipartUpload();
	else if (_status == Post::eChunkedUpload)
		_uploadChunkedToDisk();
	else if (_status == Post::eCGIResponse)
		_handelCGI();
	return _status == Post::eComplete;
}

void Post::_openUploadFile()
{
	_uploadFd = open(_filename.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
	DDEBUG("Post")
		<< "Socket fd: " << _sock->GetFd()
		<< ", OpenUploadFile: '" << _filename
		<< "', fd=" << _uploadFd;
}

void Post::_writeBodyFromMemory()
{
	string &body = _router->GetRequest().getBody();
	if (body.length() <= _uploadedSize)
		return;

	char *data = &(body[_uploadedSize]);
	int toWrite = body.length() - _uploadedSize;
	int written = _uploadToDisk(_uploadFd, data, toWrite);

	if (written > 0)
		_uploadedSize += written;
}

void Post::_writeBodyFromSocket()
{
	int written = _sock->SocketToFile(_uploadFd, _contentBodySize - _uploadedSize);
	if (written > 0)
	{
		_uploadedSize += written;
		_sock->UpdateTime();
	}
	else
		_multiplexer->ChangeToEpollInOut(_sock);
}

void Post::_uploadFileToDisk()
{
	string &body = _router->GetRequest().getBody();

	if (body.length() > _uploadedSize)
	{
		_writeBodyFromMemory();
	}
	else if (_uploadedSize < _contentBodySize)
	{
		_writeBodyFromSocket();
	}

	DEBUG("Post")
		<< "Socket fd: " << _sock->GetFd()
		<< " POST uploaded " << _uploadedSize
		<< " / " << _contentBodySize << " bytes";

	if (_uploadedSize >= _contentBodySize)
	{
		INFO()
			<< "Client " << Socket::getRemoteName(_sock->GetFd())
			<< " upload complete: " << _filename << " ("
			<< _contentBodySize << " bytes)";
		close(_uploadFd);
		_uploadFd = -1;
		_createPostResponse();
	}
}

bool Post::_getLocationReturn(string &retCode, string &retBody)
{
	const Config::Server::Location *loc = _router->GetPath().getLocation();
	if (loc == NULL || loc->returnCode.empty())
		return false;

	retCode = loc->returnCode;
	retBody = loc->returnArg;
	return true;
}

void Post::_sendPostRedirection(const string &retCode, const string &retBody)
{
	_createRedirectionHeader(retCode, retBody);
	DEBUG("Post")
		<< "Socket fd: " << _sock->GetFd()
		<< " POST redirect " << retCode
		<< " to " << retBody;
	_totalByteToSend = _responseHeaderStr.length();
	_status = Post::eSendResponse;
	_multiplexer->ChangeToEpollOut(_sock);
}

void Post::_sendPostCustomBody(const string &retCode, const string &retBody)
{
	DDEBUG("Post")
		<< "Socket fd: " << _sock->GetFd()
		<< ", SendPostCustomBody: code="
		<< retCode << ", bodyLen="
		<< retBody.length();
	_statusCode = retCode;
	_bodySize = retBody.length();
	_filename = ".json";
	_createResponseHeader();
	_responseHeaderStr += retBody;
	_totalByteToSend = _responseHeaderStr.length();
	_status = Post::eSendResponse;
	_multiplexer->ChangeToEpollOut(_sock);
}

void Post::_createPostResponse()
{
	string retCode, retBody;

	_multiplexer->ChangeToEpollOut(_sock);

	if (_getLocationReturn(retCode, retBody))
	{
		DDEBUG("Post")
			<< "Socket fd: " << _sock->GetFd()
			<< ", createPostResponse: location return code="
			<< retCode;

		if (retCode == "301" || retCode == "302")
			_sendPostRedirection(retCode, retBody);
		else
			_sendPostCustomBody(retCode, retBody);
		_addPair(_responseHeaderStr);
	}
	else
	{
		DDEBUG("Post")
			<< "Socket fd: " << _sock->GetFd()
			<< ", createPostResponse: no return directive, sending 201 Created.";
		_sendDefaultRespense("201");
	}
	_sendStrategy = new BuffersStrategy(_buffers, *_sock);
}

void Post::_checkMaxBodySize(size_t bodySize)
{
	size_t maxSize = _router->GetRequest().getMaxBodySize();
	if (maxSize > 0 && bodySize > maxSize)
	{
		DDEBUG("Post")
			<< "Socket fd: " << _sock->GetFd()
			<< ", checkMaxBodySize: body size " << bodySize
			<< " exceeds max body size " << maxSize
			<< ", sending 413.";
		Error::ThrowError("413");
	}
}

void Post::_decodBody(string &body, ChunkedData &decoder, int &status)
{
	ssize_t maxSize = _router->GetRequest().getMaxBodySize();
	if ((int)body.length() > decoder.GetUnchunkedSize())
	{
		status = decoder.UnchunkData(body);
		if (decoder.GetTotalUnchunkedSize() > maxSize)
		{
			HandelErrorPages("413");
			return;
		}
	}
	if (decoder.GetStatus() < ChunkedData::eComplete &&
		_status != Post::eInitPost)
	{
		int readLen = read(_sock->GetFd(), _buff, BUF_SIZE);
		if (readLen > 0)
			body.append(_buff, readLen);
		else
		{
			ERR() << "Error reading chunked body from socket, readLen=" << readLen;
			_sock->closeConnection = true;
			_status = Post::eComplete;
			return;
		}
		status = decoder.UnchunkData(body);
	}
	if (decoder.GetTotalUnchunkedSize() > maxSize)
	{
		HandelErrorPages("413");
		return;
	}
	if (status == -1)
	{
		HandelErrorPages("400");
		return;
	}
}

ssize_t Post::_uploadToDisk(int fd, char *data, ssize_t bodySize)
{
	ssize_t totalWritten = 0;
	while (totalWritten < bodySize)
	{
		char *s = data + totalWritten;
		ssize_t writeLen = write(fd, s, bodySize - totalWritten);
		if (writeLen <= 0)
		{
			return writeLen;
		}
		_uploadedSize += writeLen;
		totalWritten += writeLen;
	}
	return totalWritten;
}

void Post::_uploadChunkedToDisk()
{
	ClientRequest &req = _router->GetRequest();
	string &body = req.getBody();
	ChunkedData &decoder = _router->GetChunkedData();
	int status = -1;

	if (_status == Post::eChunkedUpload) {
		_decodBody(body, decoder, status);
	}
	if (decoder.GetUnchunkedSize() == 0) {
		return;
	}
	ssize_t writeLen = _uploadToDisk(_uploadFd, (char *)body.c_str(), decoder.GetUnchunkedSize());
	if (writeLen < 0)
	{
		HandelErrorPages("403");
		return;
	}
	decoder.SizeWritting(writeLen);
	body.erase(0, writeLen);
	if (status >= ChunkedData::eComplete && body.empty())
		_createPostResponse();
}

void Post::_uploadDataForm(FormData *formData)
{
	if (formData->filename.empty())
		formData->filename = Utility::getRandomStr();
	formData->filename = _filename + "/" + formData->filename;

	DDEBUG("Post") << "1-_uploadDataForm: "
			<< "is formData queue empty? " 
			<< (_router->GetMultipartData().getFormData().empty() ? "yes" : "no")
			<< ", name: " <<formData->name
			<< ", filename: " << formData->filename
			<< ", contentType: " << formData->contentType
			<< ", startIdx: " << formData->startIdx
			<< ", size: " << formData->size
			<< ", isComplete: " << formData->isComplete
			<< ", isHeaderParsed: " << formData->isHeaderParsed
			<< ", data: \n" << string(formData->data(), formData->size);
	
	
	int fd = formData->openFile();
	if (fd == -1)
	{
		DDEBUG("Post") << "2-_uploadDataForm: failed to open file '" << formData->filename << "' for writing, error: " << strerror(errno);
		HandelErrorPages("403");
		return;
	}
	ssize_t written = _uploadToDisk(fd, formData->data(), formData->size);
	DDEBUG("Post") << "3-_uploadDataForm: \n"
				   << string(formData->data(), written);
	if (written <= 0)
	{
		DDEBUG("Post") << "4-_uploadDataForm: failed to write to file '" << formData->filename << "', error: " << strerror(errno);
		HandelErrorPages("403");
	}
}

int Post::_initMultipartUpload()
{
	ClientRequest &req = _router->GetRequest();
	string &body = req.getBody();
	ChunkedData &decoder = _router->GetChunkedData();
	bool isChunked = req.isChunkedTransferEncoding();
	int status, chunkStatus = 0;

	if (isChunked)
	{
		_decodBody(body, decoder, chunkStatus);
		if (_status != Post::eMultipartUpload && _status != Post::eInitPost)
			return chunkStatus;
	}
	else if (_status != Post::eInitPost)
	{
		int size = read(_sock->GetFd(), _buff, BUF_SIZE);
		body.append(_buff, size);
	}
	MultipartData &multipartData = _router->GetMultipartData();
	string boundary = "--" + req.getMultipartBoundary();
	DDEBUG("Post") << "_initMultipartUpload: calling MultipartData::Parse";
	status = multipartData.Parse(body, boundary);
	if (chunkStatus == ChunkedData::eComplete
		&& status != MultipartData::eComplete) 
		HandelErrorPages("400");
	DDEBUG("Post") << "_initMultipartUpload: Parse result=" << status;
	return status;
}

void Post::_handleMultipartUpload()
{
	DDEBUG("Post") << "1-_handleMultipartUpload: enter";
	int parseResult = _initMultipartUpload();
	if (parseResult == MultipartData::eError)
	{
		DDEBUG("Post") << "2-_handleMultipartUpload: parse error (-1), sending 400";
		HandelErrorPages("400");
	}
	if (_status != Post::eMultipartUpload && _status != Post::eInitPost)
		return;
	ClientRequest &req = _router->GetRequest();
	string &body = req.getBody();
	DDEBUG("Post") << "3-_handleMultipartUpload: body:\n"
				   << body;
	MultipartData &multipartData = _router->GetMultipartData();
	FormData *formData;
	ChunkedData &decoder = _router->GetChunkedData();
	DDEBUG("Post") << "4-_handleMultipartUpload: processing queued form-data entries: " << multipartData.getFormData().size();
	while (!multipartData.getFormData().empty())
	{
		formData = multipartData.getFormData().front();
		multipartData.getFormData().pop();
		_uploadDataForm(formData);
		DDEBUG("Post") << "5-_handleMultipartUpload: processed form-data entry";
		delete formData;
	}
	if (parseResult < MultipartData::eComplete)
	{
		formData = multipartData.getCurrentFormData();
		if (formData == NULL || formData->size == 0)
			return;
		_uploadDataForm(formData);
		size_t idx = multipartData.getLastIndex();
		decoder.SizeWritting(idx + 1);
		body.erase(0, idx);
		multipartData.resetIndex();
		return;
	}
	DDEBUG("Post") << "7-_handleMultipartUpload: complete, creating post response";
	_createPostResponse();
}
