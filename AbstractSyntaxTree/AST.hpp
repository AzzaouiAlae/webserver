#pragma once

#include <iostream>
#include <map>
#include <vector>


template <typename content>
class AST
{
	content Value;
	std::vector<AST<content> > Children;
	std::vector<content> Arguments;

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

	std::vector<AST<content> >& GetChildren()
	{
		return Children;
	}

	std::vector<content>& GetArguments()
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