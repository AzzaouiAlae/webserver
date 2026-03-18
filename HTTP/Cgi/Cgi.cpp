/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:39:07 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/07 04:36:57 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "CGIPipe.hpp"
#include "AMethod.hpp"
#include "HTTPContext.hpp"

string Cgi::resolveExcPath(const string &excName)
{
	if (excName.find('/') != string::npos)
	{
		char absPath[PATH_MAX];
		if (realpath(excName.c_str(), absPath) == NULL)
			return excName;
		string s = absPath;
		return s;
	}

	const char *envPath = getenv("PATH");
	if (!envPath)
		return excName;

	string pathString(envPath);
	stringstream ss(pathString);
	string directory;

	while (getline(ss, directory, ':'))
	{
		string fullPath = (directory.empty() ? "." : directory) + "/" + excName;

		if (access(fullPath.c_str(), F_OK | X_OK) == 0)
			return fullPath;
	}
	return excName;
}

Cgi::Cgi(Routing *router, SocketIO *sok) : _router(router), _req(router->GetRequest()), _sok(sok), _buf(Utility::GetBuffer())
{
	if (APipe::GetPipe(_pipefd) == false)
	{
		Error::ThrowError("500");
	}
	_endSend = false;
	_statusfd = 0;
	_reqlen = 0;
	_responseHeaderStr.clear();
	_responselen = 0;
	_shouldSend = 0;
	_pid = -1;
	_childErrorStatus = -1;
	_isChunkedRequest = _req.isChunkedTransferEncoding();
	_tmpFd = -1;
	_isChunkedResponse = false;
	_chunkedFinalQueued = false;
	_chunkedFinalSent = false;
	_chunkedOutSent = 0;
	_cgiBodyOffset = 0;
	_bufferedResponse = false;
	_responseTmpFd = -1;
	_responseBodyLen = 0;
	_responseSentFromFile = 0;
	_initialBodyBuffered = false;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	_in = new CGIPipe(_pipefd[0], this);
	MulObj->AddAsEpollIn(_in);

	_out = new CGIPipe(_pipefd[1], this);
	MulObj->AddAsEpollOut(_out);

	_stdin = _pipefd[0];
	_chunkedOut.reserve(BUF_SIZE * 2);

	if (_isChunkedRequest)
	{
		// Generate temp file path for buffering chunked body
		stringstream ss;
		ss << "/tmp/cgi_chunked_" << hex;
		for (int i = 0; i < 16; i++)
			ss << (rand() % 16);
		_tmpPath = ss.str();
		_tmpFd = open(_tmpPath.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);
		if (_tmpFd == -1)
			Error::ThrowError("500");
		_status = eBufferChunkedBody;
	}
	else
		_status = eFork;

	CGILog(DDEBUG) << "Cgi::Cgi() initialized. chunkedRequest=" << (_isChunkedRequest ? "true" : "false")
				   << ", status=" << _status;

	INFO() << "Initialized new CGI Context for executable " << _router->GetPath().getLocation()->cgiPassPath;
}

void Cgi::_activeCgiPipe()
{
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollIn(_in);
	MulObj->ChangeToEpollOut(_out);
}

int Cgi::isChildError()
{
	if (_pid == -1)
		return -1;
	if (_childErrorStatus != -1)
		return _childErrorStatus;
	int status;
	if (waitpid(_pid, &status, WNOHANG) != _pid)
		return -1;

	if (WIFEXITED(status))
	{
		_childErrorStatus = WEXITSTATUS(status) ? 1 : 0;
	}
	else if (WIFSIGNALED(status) || WIFSTOPPED(status))
	{
		_childErrorStatus = 1;
	}
	return _childErrorStatus;
}

void Cgi::changeDir()
{
	string dir = _router->GetPath().getLocation()->cgiPassPath;
	dir = dir.substr(0, dir.find_last_of('/'));
	if (chdir(dir.c_str()) != 0)
		throw "500";
}

void Cgi::createChild()
{
	CGILog(DDEBUG) << "Attempting to fork child process for CGI...";
	string exec = resolveExcPath(_router->GetPath().getLocation()->cgiPassPath);
	if (access(exec.c_str(), F_OK | X_OK) != 0)
		Error::ThrowError("500");
	char *execArgs[] =
		{
			(char *)exec.c_str(),
			(char *)_router->GetPath().getFullPath().c_str(),
			NULL};
	_pid = fork();
	if (_pid < 0)
		Error::ThrowError("500");
	else if (_pid == 0)
	{
		Environment::CreateEnv(_req.getrequestenv());
		changeDir();
		if (_isChunkedRequest)
		{
			dup2(_stdin, 0);
		}
		else
		{
			dup2(_sok->pipefd[0], 0);
		}
		dup2(_pipefd[1], 1);
		execve(exec.c_str(), execArgs, environ);
		throw "500";
	}
	if (_isChunkedRequest)
		_status = eReadCgiResponse;
	else
		_status = eSendBuffToPipe;
	INFO() << "Successfully forked CGI child process.";
}

void Cgi::sendBuffToPipe()
{
	int len = 0;
	size_t size = _req.getcontentlen() - _reqlen;
	if (_req.getthereisbody())
	{
		string &body = _req.getBody();
		void *b = (char *)body.c_str() + _reqlen;

		if (size > body.size() - _reqlen)
			len = _sok->SendBuffToPipe(b, body.size() - _reqlen, false);
		else
			len = _sok->SendBuffToPipe(b, size, false);
		CGILog(DDEBUG) << "Sending buffer to CGI pipe. write: " << len << " bytes.";
		if (_sok->errorNumber == ePipe1Error)
			ErrorHandler();
		_reqlen += len;
		CGILog(DDEBUG) << "Wrote " << len
					   << " bytes from buffer. Total sent: "
					   << _reqlen << "/" << _req.getcontentlen();
		if (_reqlen == body.size())
		{
			INFO() << "[CGI PID: " << _pid << "] Memory buffer fully written to CGI pipe. Transitioning to eSendSockToPipe.";
			_status = eSendSockToPipe;
		}
	}
	checkWriteToCgiComplete();
}

void Cgi::sendSockToPipe()
{
	int len = 0;
	size_t size = _req.getcontentlen() - _reqlen;

	CGILog(DDEBUG) << "Streaming directly from socket to CGI pipe. Target size: " << size;
	len = _sok->SendSocketToPipe(size, false);
	if (_sok->errorNumber == eReadError)
		ErrorHandler();
	_reqlen += len;
	CGILog(DDEBUG) << "Streamed " << len
				   << " bytes to pipe. Total sent: "
				   << _reqlen << "/" << _req.getcontentlen();
	checkWriteToCgiComplete();
}

void Cgi::checkWriteToCgiComplete()
{
	if (_req.getBody().length() == _req.getcontentlen() || _reqlen >= _req.getcontentlen())
	{
		CGILog(DDEBUG) << "Body fully received from client. Switching Multiplexer to EpollOut.";
		Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
		MulObj->ChangeToEpollOut(_sok);
	}
	if (_reqlen >= _req.getcontentlen() || _req.getthereisbody() == false)
	{
		INFO() << "[CGI PID: " << _pid << "] Request transmission to CGI complete. Transitioning to eReadCgiResponse.";
		_status = eReadCgiResponse;
	}
}

void Cgi::bufferChunkedBodyToFile()
{
	CGILog(DDEBUG) << "bufferChunkedBodyToFile() entered. bodySize=" << _req.getBody().size()
				   << ", decodedSize=" << _req.getDecodedBody().size()
				   << ", writtenToFile=" << _reqlen;

	// Process any data already in memory
	string &body = _req.getBody();
	if (!body.empty())
	{
		CGILog(DDEBUG) << "bufferChunkedBodyToFile(): processing buffered body bytes=" << body.size();
		_req.processChunkedBody();
	}

	string &decoded = _req.getDecodedBody();

	// Write any new decoded data to temp file
	if (decoded.length() > 0)
	{
		const char *data = decoded.c_str();
		size_t toWrite = decoded.length();
		ssize_t written = write(_tmpFd, data, toWrite);
		if (written > 0)
		{
			_reqlen += written;
			decoded.erase(0, written);
			CGILog(DDEBUG) << "bufferChunkedBodyToFile(): wrote " << written
						   << " bytes to temp file. totalWritten=" << _reqlen
						   << ", decodedRemaining=" << decoded.length();
		}
	}

	// If chunked transfer not complete, read more from socket
	if (!_req.isChunkedComplete())
	{
		ssize_t len = read(_sok->GetFd(), _buf, BUF_SIZE);
		_sok->UpdateTime();
		if (len > 0)
		{
			CGILog(DDEBUG) << "bufferChunkedBodyToFile(): read " << len << " bytes from client socket.";
			body.append(_buf, len);
			_req.processChunkedBody();

			if (decoded.length() > 0)
			{
				const char *data = decoded.c_str();
				size_t toWrite = decoded.length();
				ssize_t written = write(_tmpFd, data, toWrite);
				if (written > 0)
				{
					_reqlen += written;
					decoded.erase(0, written);
					CGILog(DDEBUG) << "bufferChunkedBodyToFile(): wrote " << written
								   << " bytes after socket read. totalWritten=" << _reqlen
								   << ", decodedRemaining=" << decoded.length();
				}
			}
		}
		else if (len == 0)
		{
			CGILog(DDEBUG) << "bufferChunkedBodyToFile(): client closed connection while buffering body.";
			ErrorHandler();
			return;
		}
	}

	CGILog(DEBUG) << "bufferChunkedBodyToFile: buffered " << _reqlen << " bytes"
				  << (_req.isChunkedComplete() ? " [complete]" : " [in progress]");

	if (_req.isChunkedComplete() && decoded.length() == 0)
	{
		CGILog(DDEBUG) << "bufferChunkedBodyToFile(): chunked body complete, finalizing temp file.";
		finalizeChunkedBodyFile();
	}
}

void Cgi::finalizeChunkedBodyFile()
{
	CGILog(DDEBUG) << "finalizeChunkedBodyFile() entered. tempPath=" << _tmpPath
				   << ", bytesBuffered=" << _reqlen;
	// Set CONTENT_LENGTH in the request env
	stringstream ss;
	ss << _reqlen;
	_req.getrequestenv()["CONTENT_LENGTH"] = ss.str();

	CGILog(DEBUG) << "Chunked body fully buffered to " << _tmpPath
				  << " (" << _reqlen << " bytes). Set CONTENT_LENGTH=" << ss.str();

	// Reopen the file from the beginning for reading
	close(_tmpFd);
	_tmpFd = open(_tmpPath.c_str(), O_RDONLY | O_CLOEXEC);
	if (_tmpFd == -1)
		Error::ThrowError("500");
	_stdin = _tmpFd;

	// Switch multiplexer so we stop reading from socket
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(_sok);

	CGILog(DDEBUG) << "finalizeChunkedBodyFile(): switched to fork state with CONTENT_LENGTH=" << ss.str();
	_status = eFork;
}

void Cgi::readfromcgi()
{
	CGILog(DDEBUG) << "readfromcgi() called.";
	int len = 0;
	if (!CanUsePipe0())
	{
		CGILog(DDEBUG) << "readfromcgi(), Can't Use Pipe 0";
		return;
	}
	len = read(_pipefd[0], _buf, MAXHEADERSIZE);
	CGILog(DDEBUG) << "readfromcgi(): read len=" << len << ", currentStatus=" << _status;
	if (len <= 0)
	{
		CGILog(DDEBUG) << "readfromcgi(): pipe EOF/error, marking complete.";
		_status = eComplete;
	}
	if (_cgireq.isComplete(_buf, len))
	{
		CGILog(DDEBUG) << "readfromcgi(): CGI response headers parsed successfully.";
		_status = eCreateResponseHeader;
	}
	CGILog(DDEBUG) << "readfromcgi(), read len: " << len;
}

void Cgi::createCgiResponse()
{
	_responseHeaderStr = "HTTP/1.1 " + getStatusCode() + " " + AMethod::getStatusMap()[getStatusCode()] + "\r\n";
	map<string, string> &env = _cgireq.getrequestenv();
	for (map<string, string>::iterator it = env.begin(); it != env.end(); it++)
	{
		if (it->first == "Status")
			continue;
		_responseHeaderStr += it->first + ": " + it->second + "\r\n";
	}
	CGILog(DDEBUG) << "createCgiResponse(): status=" << getStatusCode()
				   << ", envHeaders=" << env.size();

	// Check if CGI provided Content-Length
	if (_cgireq.isContentLenghtExist())
	{
		CGILog(DDEBUG) << "createCgiResponse(): CGI provided Content-Length=" << _cgireq.getcontentlen();
		// Has Content-Length, send immediately
		_responseHeaderStr += "\r\n";
		_shouldSend = _responseHeaderStr.length() + _cgireq.getcontentlen();
		_status = eWriteBuffToClient;
	}
	else
	{
		CGILog(DDEBUG) << "createCgiResponse(): CGI has no Content-Length. chunkedSend="
					   << (_router->GetPath().getLocation()->chunkedSend ? "true" : "false");
		// No Content-Length - check if we should use chunked or buffered response
		bool chunkedSendEnabled = _router->GetPath().getLocation()->chunkedSend;

		if (chunkedSendEnabled)
		{
			// Use chunked transfer encoding
			_isChunkedResponse = true;
			if (env.find("Transfer-Encoding") == env.end())
				_responseHeaderStr += "Transfer-Encoding: chunked\r\n";
			_responseHeaderStr += "\r\n";
			_shouldSend += _responseHeaderStr.length();
			CGILog(DDEBUG) << "createCgiResponse(): using chunked response. headerSize=" << _responseHeaderStr.length();
			_status = eWriteBuffToClient;
		}
		else
		{
			// Buffer response to file - don't close headers yet
			_bufferedResponse = true;
			_responseBodyLen = 0;		  // Reset body length counter for new buffering
			_initialBodyBuffered = false; // Reset flag to track initial body buffering
			stringstream ss;
			ss << "/tmp/cgi_response_" << hex;
			for (int i = 0; i < 16; i++)
				ss << (rand() % 16);
			_responseTmpPath = ss.str();
			_responseTmpFd = open(_responseTmpPath.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);
			if (_responseTmpFd == -1)
				Error::ThrowError("500");
			CGILog(DEBUG) << "Buffering CGI response to file: " << _responseTmpPath;
			CGILog(DDEBUG) << "createCgiResponse(): buffering response to temp file because no Content-Length was provided.";
			_status = eBufferCgiResponse;
			Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
			MulObj->ChangeToEpollOut(_sok);
		}
	}
	CGILog(DDEBUG) << "Cgi::createCgiResponse():\n"
				   << _responseHeaderStr;
}

void Cgi::bufferCgiResponse()
{
	CGILog(DDEBUG) << "bufferCgiResponse() called.";
	CGILog(DDEBUG) << "bufferCgiResponse(): bufferedSize=" << _responseBodyLen
				   << ", responseTmpFd=" << _responseTmpFd
				   << ", initialBodyBuffered=" << (_initialBodyBuffered ? "true" : "false");

	// First, buffer any data that's already in the CGI response body (only once)
	if (!_initialBodyBuffered && _cgireq.getBody().size() > 0)
	{
		const char *data = _cgireq.getBody().data();
		size_t toWrite = _cgireq.getBody().size();
		ssize_t written = write(_responseTmpFd, data, toWrite);
		if (written > 0)
		{
			_responseBodyLen += written;
			CGILog(DDEBUG) << "Buffered initial body data: " << written << " bytes";
		}
		else
			CGILog(DDEBUG) << "bufferCgiResponse(): failed to buffer initial body data.";
		_initialBodyBuffered = true;
	}

	// Read from CGI pipe if available
	if (CanUsePipe0())
	{
		ssize_t len = _sok->PipeToFile(_responseTmpFd, _pipefd[0], BUF_SIZE);
		if (len > 0)
		{
			_responseBodyLen += len;
			CGILog(DDEBUG) << "Buffered from pipe: " << len << " bytes. Total: " << _responseBodyLen;
		}
		else if (len == 0 && isChildError() != -1)
		{
			CGILog(DDEBUG) << "bufferCgiResponse(): child finished and pipe drained, finalizing response.";
			// No more data and child process has finished
			finalizeAndSendBufferedResponse();
			return;
		}
		else if (len < 0)
		{
			CGILog(DDEBUG) << "bufferCgiResponse(): pipe-to-file error.";
			ErrorHandler();
			return;
		}
	}
	else if (!_endSend && isChildError() != -1)
	{
		CGILog(DDEBUG) << "bufferCgiResponse(): child finished but more data may still be pending.";
		_endSend = true;
	}
	else if (_endSend == true)
	{
		CGILog(DDEBUG) << "bufferCgiResponse(): child finished, finalizing response now.";
		// Child finished but we haven't read all data yet
		finalizeAndSendBufferedResponse();
		return;
	}
}

void Cgi::finalizeAndSendBufferedResponse()
{
	CGILog(DDEBUG) << "finalizeAndSendBufferedResponse() entered. tmpPath=" << _responseTmpPath;
	// Get the actual file size from the file system
	long fileSize = Utility::getFileSize(_responseTmpPath);
	if (fileSize < 0)
		Error::ThrowError("500");

	_responseBodyLen = (size_t)fileSize;
	CGILog(DEBUG) << "finalizeAndSendBufferedResponse() called. Actual file size: " << _responseBodyLen << " bytes.";

	// Close temp file and reopen for reading
	close(_responseTmpFd);
	_responseTmpFd = open(_responseTmpPath.c_str(), O_RDONLY | O_CLOEXEC);
	if (_responseTmpFd == -1)
		Error::ThrowError("500");
	CGILog(DDEBUG) << "finalizeAndSendBufferedResponse(): reopened response temp file for reading.";

	// Complete the headers with Content-Length
	stringstream ss;
	ss << _responseBodyLen;
	_responseHeaderStr += "Content-Length: " + ss.str() + "\r\n";
	_responseHeaderStr += "\r\n";

	_shouldSend = _responseHeaderStr.length() + _responseBodyLen;
	_responselen = 0;
	_responseSentFromFile = 0;
	_bufferedResponse = false;
	_status = eWriteBuffToClient;

	CGILog(DDEBUG) << "Response finalized. Headers: \n"
				   << _responseHeaderStr;
}

void Cgi::ErrorHandler()
{
	if (_responselen == 0)
		Error::ThrowError("502");
	else
		_status = eComplete;
}

void Cgi::writeBuffToClient()
{
	int len = 0;
	CGILog(DDEBUG)
		<< "Writing CGI headers to client. Header size: "
		<< _responseHeaderStr.length() << " bytes, Sent so far: " << _responselen << "\n"
		<< _responseHeaderStr;
	void *buff = (void *)(_responseHeaderStr.c_str() + _responselen);
	len = _sok->Send(buff, _responseHeaderStr.length() - _responselen);
	if (_sok->errorNumber == eWriteError)
		ErrorHandler();
	_sok->UpdateTime();
	_responselen += len;
	CGILog(DDEBUG) << "Sent " << len << " header bytes. Total sent: " << _responselen;
	if (_responselen == _responseHeaderStr.length())
	{
		INFO() << "Finished sending CGI headers. Transitioning to eWritePipeToClient.";
		CGILog(DDEBUG) << "writeBuffToClient(): header send complete, switching to body send.";
		_status = eWritePipeToClient;
	}
	checkWriteToClientSoket();
}

void Cgi::writePipeToClient()
{
	int len = 0;
	CGILog(DDEBUG) << "writePipeToClient() entered. responselen=" << _responselen
				   << ", shouldSend=" << _shouldSend
				   << ", chunked=" << (_isChunkedResponse ? "true" : "false")
				   << ", buffered=" << (_bufferedResponse ? "true" : "false");

	// Handle buffered response from temporary file
	if (_responseTmpFd != -1 && !_bufferedResponse)
	{
		CGILog(DDEBUG) << "writePipeToClient(): sending buffered file body. sentFromFile=" << _responseSentFromFile
					   << ", bodyLen=" << _responseBodyLen;
		// Send body from temporary file using read/send loop
		if (_responseSentFromFile < _responseBodyLen)
		{
			len = _sok->FileToSocket(_responseTmpFd, _responseBodyLen - _responseSentFromFile);
			if (_sok->errorNumber == eWriteError)
				ErrorHandler();
			if (len > 0)
			{
				_sok->UpdateTime();
				_responselen += len;
				_responseSentFromFile += len;
			}
			else
				ErrorHandler();
			CGILog(DDEBUG) << "Sent " << len << " body bytes from buffered file. Total sent: " << _responselen << "/" << _shouldSend;
		}
		if (_responselen >= _shouldSend)
		{
			CGILog(DDEBUG) << "writePipeToClient(): buffered file body fully sent.";
			_status = eComplete;
		}
		return;
	}
	if (_isChunkedResponse)
	{
		CGILog(DDEBUG) << "writePipeToClient(): chunked response path. chunkedOutSize=" << _chunkedOut.size()
					   << ", chunkedOutSent=" << _chunkedOutSent
					   << ", cgiBodyOffset=" << _cgiBodyOffset
					   << ", cgiBodySize=" << _cgireq.getBody().size();
		_clearChunkedOut();
		if (_chunkedOutSent < _chunkedOut.size())
		{
			CGILog(DDEBUG) << "writePipeToClient(): flushing queued chunked data.";
			flushChunkedOut();
			if (_chunkedFinalSent)
				_status = eComplete;
			return;
		}

		if (_cgiBodyOffset < _cgireq.getBody().size())
		{
			size_t remaining = _cgireq.getBody().size() - _cgiBodyOffset;
			size_t chunkSize = min(remaining, (size_t)BUF_SIZE);
			CGILog(DDEBUG) << "writePipeToClient(): queueing chunk from parsed CGI body, size=" << chunkSize;
			queueChunk(_cgireq.getBody().data() + _cgiBodyOffset, chunkSize);
			_cgiBodyOffset += chunkSize;
			flushChunkedOut();
			return;
		}

		if (CanUsePipe0())
		{
			ssize_t readLen = read(_pipefd[0], _buf, BUF_SIZE);
			CGILog(DDEBUG) << "writePipeToClient(): read from CGI pipe returned " << readLen;
			if (readLen > 0)
			{
				queueChunk(_buf, readLen);
				flushChunkedOut();
				return;
			}
			if (readLen <= 0 && isChildError() != -1)
			{
				queueFinalChunk();
				flushChunkedOut();
				if (_chunkedFinalSent)
					_status = eComplete;
				return;
			}
		}
		else if (_endSend == false && isChildError() != -1)
		{
			_endSend = true;
		}
		else if (_endSend == true && !_chunkedFinalSent && !_chunkedFinalQueued)
		{
			CGILog(DDEBUG) << "writePipeToClient(): child finished, queueing final chunk.";
			queueFinalChunk();
			flushChunkedOut();
			if (_chunkedFinalSent)
				_status = eComplete;
		}
	}

	else if (_cgireq.getBody().size() + _responseHeaderStr.length() > _responselen)
	{
		int size = _responselen - _responseHeaderStr.length();
		void *buff = (void *)(_cgireq.getBody().data() + size);
		CGILog(DDEBUG) << "writePipeToClient(): sending parsed CGI body from memory. offset=" << size
					   << ", bodySize=" << _cgireq.getBody().size();
		len = _sok->Send(buff, _cgireq.getBody().length() - size);
		CGILog(DDEBUG) << "Writing CGI body from memory buffer. Attempting to send " << size << " bytes.";
		if (_sok->errorNumber == eWriteError)
			ErrorHandler();
		_sok->UpdateTime();
		_responselen += len;
		CGILog(DDEBUG) << "Sent " << len << " body bytes from buffer. Total sent: " << _responselen;
	}

	else if (CanUsePipe0())
	{
		CGILog(DDEBUG) << "writePipeToClient(): streaming CGI pipe directly to socket.";
		if (_cgireq.isContentLenghtExist())
			len = _sok->SendPipeToSock(_pipefd[0], _shouldSend - _responselen);
		else
			len = _sok->SendPipeToSock(_pipefd[0]);
		if (_sok->errorNumber == eWriteError)
			ErrorHandler();
		_responselen += len;
		CGILog(DDEBUG) << "Sent " << len << " body bytes from pipe. Total sent: " << _responselen;
	}
	else if (_endSend == false && isChildError() != -1)
	{
		_endSend = true;
	}
	else if (_endSend == true && !_cgireq.isContentLenghtExist() && _responselen >= _shouldSend)
	{
		CGILog(DDEBUG) << "writePipeToClient(): response complete by size check.";
		_status = eComplete;
	}
	checkWriteToClientSoket();
}

void Cgi::queueChunk(const char *data, size_t len)
{
	stringstream ss;
	ss << hex << len;
	_chunkedOut += ss.str();
	_chunkedOut += "\r\n";
	_chunkedOut.append(data, len);
	_chunkedOut += "\r\n";
}

void Cgi::queueFinalChunk()
{
	if (_chunkedFinalQueued || _chunkedFinalSent)
		return;
	_chunkedOut += "0\r\n\r\n";
	_chunkedFinalQueued = true;
}

bool Cgi::flushChunkedOut()
{
	if (_chunkedOutSent >= _chunkedOut.size())
	{
		CGILog(DDEBUG) << "flushChunkedOut(): nothing to flush.";
		return true;
	}
	void *buff = (void *)(_chunkedOut.c_str() + _chunkedOutSent);
	int len = _sok->Send(buff, _chunkedOut.size() - _chunkedOutSent);
	if (_sok->errorNumber == eWriteError)
		ErrorHandler();
	if (len > 0)
	{
		_sok->UpdateTime();
		_chunkedOutSent += len;
		_responselen += len;
	}
	if (_chunkedOutSent >= _chunkedOut.size())
	{
		if (_chunkedFinalQueued)
		{
			_chunkedFinalQueued = false;
			_chunkedFinalSent = true;
		}
		CGILog(DDEBUG) << "flushChunkedOut(): queued chunk data fully sent.";
		return true;
	}
	return false;
}

void Cgi::checkWriteToClientSoket()
{
	if (_shouldSend <= _responselen && _cgireq.isContentLenghtExist())
	{
		INFO()
			<< "CGI Response fully sent to client. Total bytes: "
			<< _responselen << " / " << _shouldSend
			<< ". Status set to eComplete.";
		_status = eComplete;
	}
}

void Cgi::_clearChunkedOut()
{
	tcp_info info;
	socklen_t len;
	len = sizeof(info);

	if (getsockopt(_sok->GetFd(), IPPROTO_TCP, TCP_INFO, &info, &len) == 0)
	{
		if (info.tcpi_unacked == 0)
		{
			if (_chunkedOutSent >= _chunkedOut.size())
			{
				_chunkedOut.clear();
				_chunkedOutSent = 0;
			}
		}
	}
}

void Cgi::Handle()
{

	CGILog(DDEBUG) << "Handle() called. Current status: " << _status;

	if (_status == eBufferChunkedBody)
		bufferChunkedBodyToFile();
	if (_status == eFork)
		createChild();
	if (!_isChunkedRequest)
	{
		if (_status == eSendBuffToPipe)
			sendBuffToPipe();
		else if (_status == eSendSockToPipe)
			sendSockToPipe();
	}
	if (_status == eReadCgiResponse)
		readfromcgi();
	if (_status == eBufferCgiResponse)
		bufferCgiResponse();
	if (_status == eCreateResponseHeader)
		createCgiResponse();
	else if (_status == eWriteBuffToClient)
		writeBuffToClient();
	else if (_status == eWritePipeToClient)
		writePipeToClient();
	if (isChildError() == 1)
		ErrorHandler();
	if (_status != eComplete)
	{
		CGILog(DDEBUG) << "Handle(): re-arming CGI pipes for next event. status=" << _status;
		_activeCgiPipe();
	}
}

void Cgi::SetStateByFd(int fd)
{
	CGILog(DDEBUG) << "SetStateByFd() called. Active fd: " << fd;
	if (fd == _pipefd[0])
		_statusfd |= ePipe0;
	else if (fd == _pipefd[1])
		_statusfd |= ePipe1;
}

Cgi::~Cgi()
{
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	if (_in)
	{
		DDEBUG("HTTPContext") << "  -> Deleting in-pipe fd=" << _in->GetFd();
		MulObj->DeleteFromEpoll(_in);
		delete _in;
	}
	if (_out)
	{
		DDEBUG("HTTPContext") << "  -> Deleting out-pipe fd=" << _out->GetFd();
		MulObj->DeleteFromEpoll(_out);
		delete _out;
	}
	Utility::ReleaseBuffer(_buf);
	if (_tmpFd != -1)
		close(_tmpFd);
	if (!_tmpPath.empty())
		unlink(_tmpPath.c_str());
	if (_responseTmpFd != -1)
		close(_responseTmpFd);
	if (!_responseTmpPath.empty())
		unlink(_responseTmpPath.c_str());
	if (_pid > 0)
	{
		if (isChildError() == -1)
		{
			INFO() << "Child process still running. Sending SIGKILL to PID: " << _pid;
			kill(_pid, SIGKILL);
			waitpid(_pid, NULL, 0);
		}
	}
	else if (isChildError() == 0)
		INFO() << "Child process already terminated with status: " << (isChildError() == 1 ? "Error" : "Success");
	if (CanUsePipe0())
	{
		read(_pipefd[0], _buf, BUF_SIZE + 1);
		_sok->UpdateTime();
	}
	APipe::ReleasePipe(_pipefd);
}

bool Cgi::CanUsePipe0()
{
	bool res = (_statusfd & ePipe0);
	_statusfd &= ~ePipe0;
	return res;
}

bool Cgi::CanUsePipe1()
{
	bool res = (_statusfd & ePipe1);
	_statusfd &= ~ePipe1;
	return res;
}

string &Cgi::getStatusCode()
{
	return _cgireq.getStatusCode();
}

CgiRequest &Cgi::getCgiReq()
{
	return (_cgireq);
}

bool Cgi::isComplete()
{
	return _status == eComplete;
}
