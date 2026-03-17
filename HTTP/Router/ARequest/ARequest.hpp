/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ARequest.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 06:22:09 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/03 03:54:29 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "iostream"
#include <map>
#include <vector>
#include "SessionManager.hpp"

using namespace std;
#define MAXHEADERSIZE 8192

class ARequest
{
protected:
    map<string, string> *_reqDirectives;
    map<string, string> _env;
    map<string, string> _cookies;
    Session* _currentSession;
    bool _firstline;
    string _requestbuff;
	size_t _content_len;
    bool _Thereisbody;
    int  _headersize;
    void parseCookies();

    // Chunked transfer encoding state
    bool _isChunked;
    bool _chunkedComplete;
    enum eChunkState { eChunkSize, eChunkData, eChunkTrailer, eChunkDone };
    eChunkState _chunkState;
    size_t _currentChunkSize;
    size_t _currentChunkRead;
    string _decodedBody;
    bool processChunkedData();
    

    // Multipart form-data state
    bool _isMultipart;
    string _boundary;

public:
    virtual bool getFullLine(string &line) = 0;
    virtual void parseHeaderLine(const string &line) = 0;
    ARequest();
    virtual ~ARequest();
    virtual bool isComplete(char *request, int size) = 0;
    
    map<string, string> &getrequestenv();
    bool 		        getthereisbody();
    size_t		        getcontentlen();
    string              &getBody();
    void initSession();
    Session* getSession();
    string getCookie(const string &name);
    bool isChunkedTransferEncoding();
    bool isMultipartFormData();
    string getMultipartBoundary();
    bool isChunkedComplete() const;
    string &getDecodedBody();
};

#include "Headers.hpp"
