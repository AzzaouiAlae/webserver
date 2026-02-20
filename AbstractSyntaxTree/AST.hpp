#pragma once

#include <iostream>
#include <map>
#include <vector>

using namespace std;


template <typename content>
class AST
{
	content Value;
	vector<AST<content> > Children;
	vector<content> Arguments;

public:
	AST(content value)
	{
		Value = value;
	}

	void AddChild(content value)
	{
		Children.push_back(AST(value));
	}

	void AddChild(AST<content> node)
	{
		Children.push_back(node);
	}

	void AddArgument(content value)
	{
		Arguments.push_back(value);
	}

	vector<AST<content> >& GetChildren()
	{
		return Children;
	}

	vector<content>& GetArguments()
	{
		return Arguments;
	}

	content GetValue()
	{
		return Value;
	}

	void SetValue(content val)
	{
		Value = val;
	}
	
};