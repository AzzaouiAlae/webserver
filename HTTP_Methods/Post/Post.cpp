#include "Post.hpp"

Post::Post(SocketIO *sock, Routing *router) : AMethod(sock, router)
{
	_path = &router->GetPath();
	_originalPath = &_path->OriginalPath;
	uploadFd = -1;
	readyToUpload = false;
	contentBodySize = 0;
	uploadedSize = 0;
	_isMultipartUpload = false;
	_isChunkedUpload = false;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollIn(sock);
	DEBUG("Post")
		<< "Post initialized, socket fd="
		<< sock->GetFd();
	_buff = Utility::GetBuffer();
}

Post::~Post()
{
	if (uploadFd != -1)
		close(uploadFd);
	Utility::ReleaseBuffer(_buff);
}

bool Post::HandleResponse()
{
	sock->SetStateByFd(sock->GetFd());

	DEBUG("Post")
		<< "Socket fd: " << sock->GetFd()
		<< ", Post::HandleResponse() start";
	DDEBUG("Post") << "Socket fd: " << sock->GetFd()
				   << ", readyToSend=" << readyToSend
				   << ", readyToUpload=" << readyToUpload
				   << ", uploadedSize=" << uploadedSize
				   << ", contentBodySize=" << contentBodySize;

	if (sock->isTimeOut())
	{
		if (router->GetPath().isCGI())
			HandelErrorPages("504");
		else
			HandelErrorPages("408");
		sock->UpdateTime();
		return del;
	}
	if (readyToSend)
	{
		SendResponse();
		return del;
	}

	if (readyToUpload)
	{
		if (_isMultipartUpload)
			handleMultipartUpload();
		else if (_isChunkedUpload)
			uploadChunkedToDisk();
		else
			uploadFileToDisk();
		return del;
	}

	ResolvePath();

	if (!IsMethodAllowed("POST"))
	{
		DDEBUG("Post")
			<< "Socket fd: " << sock->GetFd()
			<< ", POST method not allowed, sending 405.";
		HandelErrorPages("405");
		return del;
	}

	if (router->GetPath().isRedirection())
	{
		DDEBUG("Post")
			<< "Socket fd: " << sock->GetFd()
			<< ", redirection detected.";
		SendRedirection();
		return del;
	}

	if (router->GetPath().isCGI())
	{
		DDEBUG("Post")
			<< "Socket fd: " << sock->GetFd()
			<< ", CGI path detected .";
		HandelCGI();
		return del;
	}

	if (_originalPath->found && _originalPath->isFile)
	{
		DDEBUG("Post")
			<< "Socket fd: " << sock->GetFd()
			<< ", file already exists, sending 409.";
		HandelErrorPages("405");
		return del;
	}

	if (_originalPath->isDir)
	{
		if (!router->GetRequest().isMultipartFormData())
		{
			string resolvedName = resolveUploadFileName();
			if (!filename.empty() && filename[filename.length() - 1] != '/')
				filename += "/";
			filename += resolvedName;
			DEBUG("Post")
				<< "Socket fd: " << sock->GetFd()
				<< ", path is directory, resolved filename: " << filename;
		}
	}

	PostMethod();
	return del;
}

void Post::OpenUploadFile()
{
	uploadFd = open(filename.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
	DDEBUG("Post")
		<< "Socket fd: " << sock->GetFd()
		<< ", OpenUploadFile: '" << filename
		<< "', fd=" << uploadFd;
}

void Post::PostMethod()
{
	if (router->GetPath().getLocation() == NULL)
	{
		DDEBUG("Post")
			<< "Socket fd: " << sock->GetFd()
			<< ", PostMethod: no location matched, sending 403.";
		HandelErrorPages("403");
		return;
	}

	contentBodySize = router->GetRequest().getcontentlen();

	if (router->GetRequest().isMultipartFormData())
	{
		_isMultipartUpload = true;
		readyToUpload = true;
		DEBUG("Post") << "Socket fd: " << sock->GetFd()
					  << ", PostMethod: multipart upload detected.";
		handleMultipartUpload();
		return;
	}

	if (router->GetRequest().isChunkedTransferEncoding())
	{
		_isChunkedUpload = true;
		OpenUploadFile();
		if (uploadFd == -1)
		{
			DDEBUG("Post")
				<< "Socket fd: " << sock->GetFd()
				<< ", PostMethod: failed to open upload file for chunked, sending 403.";
			HandelErrorPages("403");
			return;
		}
		readyToUpload = true;
		DEBUG("Post") << "Socket fd: " << sock->GetFd()
					  << ", PostMethod: chunked upload detected.";
		uploadChunkedToDisk();
		return;
	}

	OpenUploadFile();
	if (uploadFd == -1)
	{
		DDEBUG("Post")
			<< "Socket fd: " << sock->GetFd()
			<< ", PostMethod: failed to open upload file, sending 403.";
		HandelErrorPages("403");
		return;
	}

	contentBodySize = router->GetRequest().getcontentlen();
	readyToUpload = true;
	DDEBUG("Post")
		<< "Socket fd: " << sock->GetFd()
		<< ", PostMethod: starting upload, contentBodySize="
		<< contentBodySize;
	uploadFileToDisk();
}

void Post::WriteBodyFromMemory()
{
	string &body = router->GetRequest().getBody();
	if (body.length() <= uploadedSize)
		return;

	const char *data = &(body[uploadedSize]);
	int toWrite = body.length() - uploadedSize;
	int written = write(uploadFd, data, toWrite);

	if (written > 0)
		uploadedSize += written;
}

void Post::WriteBodyFromSocket()
{
	int written = sock->SocketToFile(uploadFd, contentBodySize - uploadedSize);
	if (written > 0)
		uploadedSize += written;
}

void Post::uploadFileToDisk()
{
	string &body = router->GetRequest().getBody();

	if (body.length() > uploadedSize)
	{
		WriteBodyFromMemory();
	}
	else if (uploadedSize < contentBodySize)
	{
		WriteBodyFromSocket();
	}

	DEBUG("Post")
		<< "Socket fd: " << sock->GetFd()
		<< " POST uploaded " << uploadedSize
		<< " / " << contentBodySize << " bytes";

	if (uploadedSize >= contentBodySize)
	{
		INFO()
			<< "Client " << Socket::getRemoteName(sock->GetFd())
			<< " upload complete: " << filename << " ("
			<< contentBodySize << " bytes)";
		close(uploadFd);
		uploadFd = -1;
		readyToUpload = false;
		createPostResponse();
	}
}

bool Post::GetLocationReturn(string &retCode, string &retBody)
{
	const Config::Server::Location *loc = router->GetPath().getLocation();
	if (loc == NULL || loc->returnCode.empty())
		return false;

	retCode = loc->returnCode;
	retBody = loc->returnArg;
	return true;
}

void Post::SendPostRedirection(const string &retCode, const string &retBody)
{
	CreateRedirectionHeader(retCode, retBody);
	DEBUG("Post")
		<< "Socket fd: " << sock->GetFd()
		<< " POST redirect " << retCode
		<< " to " << retBody;
	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(sock);
}

void Post::SendPostCustomBody(const string &retCode, const string &retBody)
{
	DDEBUG("Post")
		<< "Socket fd: " << sock->GetFd()
		<< ", SendPostCustomBody: code="
		<< retCode << ", bodyLen="
		<< retBody.length();
	code = retCode;
	bodySize = retBody.length();
	filename = ".json";
	CreateResponseHeader();
	responseHeaderStr += retBody;
	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(sock);
}

void Post::createPostResponse()
{
	string retCode, retBody;

	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollOut(sock);

	if (GetLocationReturn(retCode, retBody))
	{
		DDEBUG("Post")
			<< "Socket fd: " << sock->GetFd()
			<< ", createPostResponse: location return code="
			<< retCode;

		if (retCode == "301" || retCode == "302")
			SendPostRedirection(retCode, retBody);
		else
			SendPostCustomBody(retCode, retBody);
	}
	else
	{
		DDEBUG("Post")
			<< "Socket fd: " << sock->GetFd()
			<< ", createPostResponse: no return directive, sending 201 Created.";
		SendDefaultRespense("201");
	}
}

void Post::uploadChunkedToDisk()
{
	ClientRequest &req = router->GetRequest();
	string &body = req.getBody();

	if (!body.empty())
	{
		try
		{
			req.processChunkedBody();
		}
		catch (const std::exception &e)
		{
			if (sended == 0)
				HandelErrorPages(e.what());
			else
			{
				del = true;
				sock->closeConnection = true;
			}
			return;
		}
	}

	string &decoded = req.getDecodedBody();
	if (decoded.length() > uploadedSize)
	{
		const char *data = decoded.c_str() + uploadedSize;
		size_t toWrite = decoded.length() - uploadedSize;
		ssize_t written = write(uploadFd, data, toWrite);
		if (written > 0)
			uploadedSize += written;
	}

	if (!req.isChunkedComplete())
	{
		ssize_t len = read(sock->GetFd(), _buff, BUF_SIZE);
		if (len > 0)
		{
			body.append(_buff, len);
			try
			{
				req.processChunkedBody();
			}
			catch (const std::exception &e)
			{
				if (sended == 0)
					HandelErrorPages(e.what());
				else
				{
					del = true;
					sock->closeConnection = true;
				}
				return;
			}

			if (decoded.length() > uploadedSize)
			{
				const char *data = decoded.c_str() + uploadedSize;
				size_t toWrite = decoded.length() - uploadedSize;
				ssize_t written = write(uploadFd, data, toWrite);
				if (written > 0)
					uploadedSize += written;
			}
		}
		else if (len == 0)
		{
			HandelErrorPages("400");
			return;
		}
	}

	DEBUG("Post") << "Socket fd: " << sock->GetFd()
				  << " POST chunked uploaded " << uploadedSize << " bytes"
				  << (req.isChunkedComplete() ? " [complete]" : " [in progress]");

	if (req.isChunkedComplete() && uploadedSize >= decoded.length())
	{
		contentBodySize = uploadedSize;
		INFO() << "Client " << Socket::getRemoteName(sock->GetFd())
			   << " chunked upload complete: " << filename
			   << " (" << contentBodySize << " bytes)";
		close(uploadFd);
		uploadFd = -1;
		readyToUpload = false;
		createPostResponse();
	}
}

string Post::extractFilenameFromDisposition(const string &header)
{
	size_t fnPos = header.find("filename=\"");
	if (fnPos == string::npos)
		return "";
	fnPos += 10;
	size_t fnEnd = header.find('"', fnPos);
	if (fnEnd == string::npos || fnEnd == fnPos)
		return "";
	string fname = header.substr(fnPos, fnEnd - fnPos);
	// sanitize: strip path components, keep only the base name
	size_t slashPos = fname.find_last_of("/\\");
	if (slashPos != string::npos)
		fname = fname.substr(slashPos + 1);
	return fname;
}

string Post::resolveUploadFileName()
{
	map<string, string> &env = router->GetRequest().getrequestenv();

	// 1. Check Content-Disposition header for filename="..."
	if (env.find("Content-Disposition") != env.end())
	{
		string name = extractFilenameFromDisposition(env["Content-Disposition"]);
		if (!name.empty())
		{
			DDEBUG("Post") << "Socket fd: " << sock->GetFd()
						   << ", filename from Content-Disposition: " << name;
			return name;
		}
	}

	// 2. Check X-File-Name custom header
	if (env.find("X-File-Name") != env.end() && !env["X-File-Name"].empty())
	{
		string name = env["X-File-Name"];
		size_t slashPos = name.find_last_of("/\\");
		if (slashPos != string::npos)
			name = name.substr(slashPos + 1);
		if (!name.empty())
		{
			DDEBUG("Post") << "Socket fd: " << sock->GetFd()
						   << ", filename from X-File-Name: " << name;
			return name;
		}
	}

	// 3. Fallback: generate a random filename with extension from Content-Type
	string extension;
	string contentType;
	if (env.find("CONTENT_TYPE") != env.end())
		contentType = env["CONTENT_TYPE"];

	size_t semiPos = contentType.find(';');
	if (semiPos != string::npos)
		contentType = contentType.substr(0, semiPos);

	map<string, string> &mime = Singleton::GetMime();
	for (map<string, string>::iterator it = mime.begin(); it != mime.end(); ++it)
	{
		if (it->second == contentType)
		{
			extension = "." + it->first;
			break;
		}
	}

	stringstream ss;
	ss << hex;
	for (int i = 0; i < 16; i++)
		ss << (rand() % 16);

	DDEBUG("Post") << "Socket fd: " << sock->GetFd()
				   << ", no filename provided, generated random: " << ss.str() + extension;
	return ss.str() + extension;
}

void Post::handleMultipartUpload()
{
	ClientRequest &req = router->GetRequest();
	string &body = req.getBody();
	bool isChunked = req.isChunkedTransferEncoding();

	if (isChunked)
	{
		try
		{
			if (!body.empty())
				req.processChunkedBody();

			if (!req.isChunkedComplete())
			{
				char readBuf[8192];
				ssize_t len = read(sock->GetFd(), readBuf, sizeof(readBuf));
				if (len > 0)
				{
					body.append(readBuf, len);
					req.processChunkedBody();
				}
				else if (len == 0)
				{
					HandelErrorPages("400");
					return;
				}
			}
		}
		catch (const std::exception &e)
		{
			if (sended == 0)
				HandelErrorPages(e.what());
			else
			{
				del = true;
				sock->closeConnection = true;
			}
			return;
		}
		if (!req.isChunkedComplete())
			return;

		_multipartBuffer = req.getDecodedBody();
	}
	else
	{
		if (body.length() > uploadedSize)
		{
			_multipartBuffer.append(body, uploadedSize, body.length() - uploadedSize);
			uploadedSize = body.length();
		}

		if (uploadedSize < contentBodySize)
		{
			char readBuf[8192];
			size_t toRead = contentBodySize - uploadedSize;
			if (toRead > sizeof(readBuf))
				toRead = sizeof(readBuf);
			ssize_t len = read(sock->GetFd(), readBuf, toRead);
			if (len > 0)
			{
				_multipartBuffer.append(readBuf, len);
				uploadedSize += len;
			}
			else if (len == 0)
			{
				HandelErrorPages("400");
				return;
			}
		}
	}

	if (uploadedSize < contentBodySize)
		return;

	DEBUG("Post") << "Socket fd: " << sock->GetFd()
				  << ", handleMultipartUpload: full body received ("
				  << _multipartBuffer.length() << " bytes), parsing multipart.";

	if (!parseMultipartBody())
	{
		HandelErrorPages("400");
		return;
	}
	uploadMultipartParts();
}

bool Post::parseMultipartBody()
{
	string boundary = router->GetRequest().getMultipartBoundary();
	string firstDelimiter = "--" + boundary + "\r\n";
	string delimiter = "\r\n--" + boundary;

	_parts.clear();

	size_t start = _multipartBuffer.find(firstDelimiter);
	if (start == string::npos)
		return false;
	start += firstDelimiter.length();

	while (start < _multipartBuffer.length())
	{
		size_t end = _multipartBuffer.find(delimiter, start);
		if (end == string::npos)
			break;

		string partContent = _multipartBuffer.substr(start, end - start);

		size_t headerEnd = partContent.find("\r\n\r\n");
		if (headerEnd == string::npos)
			return false;

		string headerSection = partContent.substr(0, headerEnd);
		string bodyContent = partContent.substr(headerEnd + 4);

		MultipartPart part;
		if (!extractPartInfo(headerSection, part))
			return false;
		part.body = bodyContent;
		_parts.push_back(part);

		start = end + delimiter.length();
		if (start + 1 < _multipartBuffer.length() && _multipartBuffer[start] == '-' && _multipartBuffer[start + 1] == '-')
			break;
		if (start < _multipartBuffer.length() && _multipartBuffer[start] == '\r')
			start += 2;
	}

	DEBUG("Post") << "Socket fd: " << sock->GetFd()
				  << ", parseMultipartBody: found " << _parts.size() << " parts.";
	return !_parts.empty();
}

bool Post::extractPartInfo(const string &headerSection, MultipartPart &part)
{
	stringstream ss(headerSection);
	string line;
	while (getline(ss, line))
	{
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);
		if (line.empty())
			continue;

		size_t colonPos = line.find(':');
		if (colonPos == string::npos)
			continue;

		string key = line.substr(0, colonPos);
		string value = line.substr(colonPos + 1);
		if (!value.empty() && value[0] == ' ')
			value.erase(0, 1);

		part.headers[key] = value;

		if (key == "Content-Disposition")
		{
			size_t namePos = value.find("name=\"");
			if (namePos != string::npos)
			{
				namePos += 6;
				size_t nameEnd = value.find('"', namePos);
				if (nameEnd != string::npos)
					part.name = value.substr(namePos, nameEnd - namePos);
			}

			size_t fnPos = value.find("filename=\"");
			if (fnPos != string::npos)
			{
				fnPos += 10;
				size_t fnEnd = value.find('"', fnPos);
				if (fnEnd != string::npos)
					part.filename = value.substr(fnPos, fnEnd - fnPos);
			}
		}
		else if (key == "Content-Type")
		{
			part.contentType = value;
		}
	}
	return true;
}

void Post::uploadMultipartParts()
{
	if (_parts.empty())
	{
		HandelErrorPages("400");
		return;
	}

	string uploadDir = filename;
	if (!uploadDir.empty() && uploadDir[uploadDir.length() - 1] != '/')
		uploadDir += "/";

	bool anyUploaded = false;
	for (size_t i = 0; i < _parts.size(); i++)
	{
		if (_parts[i].filename.empty())
			continue;

		string filePath = uploadDir + _parts[i].filename;
		int fd = open(filePath.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
		if (fd == -1)
		{
			DEBUG("Post") << "Socket fd: " << sock->GetFd()
						  << ", uploadMultipartParts: failed to create " << filePath;
			continue;
		}

		const char *data = _parts[i].body.c_str();
		size_t remaining = _parts[i].body.length();
		while (remaining > 0)
		{
			ssize_t written = write(fd, data, remaining);
			if (written <= 0)
				break;
			data += written;
			remaining -= written;
		}
		close(fd);

		INFO() << "Client " << Socket::getRemoteName(sock->GetFd())
			   << " multipart upload: " << filePath
			   << " (" << _parts[i].body.length() << " bytes)";
		anyUploaded = true;
	}

	if (!anyUploaded)
	{
		// No file parts found — save non-file form-data parts
		for (size_t i = 0; i < _parts.size(); i++)
		{
			string partName = _parts[i].name;
			if (partName.empty())
			{
				stringstream ss;
				ss << "part_" << i;
				partName = ss.str();
			}
			string filePath = uploadDir + partName;
			int fd = open(filePath.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
			if (fd == -1)
				continue;
			const char *data = _parts[i].body.c_str();
			size_t remaining = _parts[i].body.length();
			while (remaining > 0)
			{
				ssize_t written = write(fd, data, remaining);
				if (written <= 0)
					break;
				data += written;
				remaining -= written;
			}
			close(fd);
			anyUploaded = true;
		}
		if (!anyUploaded)
		{
			HandelErrorPages("400");
			return;
		}
	}

	readyToUpload = false;
	createPostResponse();
}
