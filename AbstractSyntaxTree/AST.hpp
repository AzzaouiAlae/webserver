#pragma once
#include <iostream>
#include <vector>

enum NodeType
{
	server,
	listen,
	root,
	index,
	server_name,
	location
};

template <typename content>
class AST
{
	NodeType type;
	content Value;
	std::vector<AST<content> > Children;
	std::vector<content> Arguments;
	AST *Next;

	void PreorderTraversal(AST<content> root, void (*f)(content, int), int level = 0)
	{
		f(root.Value, level);
		for (int i = 0; i < (int)root.Arguments.size(); i++)
		{
			f(root.Arguments[i], level);
		}
		for (int i = 0; i < (int)root.Children.size(); i++)
		{
			PreorderTraversal(root.Children[i], f, level + 1);
		}
	}

	void PostorderTraversal(AST<content> root, void (*f)(content, int), int level = 0)
	{
		for (int i = 0; i < (int)root.Children.size(); i++)
		{
			PostorderTraversal(root.Children[i], f, level + 1);
		}
		f(root.Value, level);
		for (int i = 0; i < (int)root.Arguments.size(); i++)
		{
			f(root.Arguments[i], level);
		}
	}

	void InorderTraversal(AST<content> root, void (*f)(content, int), int level = 0)
	{
		bool visited = false;
		for (int i = 0; i < root.Children.size(); i++)
		{
			if (visited == false && root.Children.size() / 2 <= i)
			{
				visited = true;
				f(root.Value, level);
				for (int i = 0; i < root.Arguments.size(); i++)
				{
					f(root.Arguments[i], level);
				}
			}
			InorderTraversal(root.Children[i], f, level + 1);
		}
		if (visited == false)
			f(root.Value, level);
	}

public:
	AST(content value) : Value(value)
	{
		Next = NULL;
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

	std::vector<AST<content> > GetChildren()
	{
		return Children;
	}

	content GetValue()
	{
		return Value;
	}

	content SetValue()
	{
		return Value;
	}

	void PreorderTraversal(void (*f)(content, int))
	{
		PreorderTraversal(*this, f);
	}

	void PostorderTraversal(void (*f)(content, int))
	{
		PostorderTraversal(*this, f, 0);
	}

	void InorderTraversal(void (*f)(content, int))
	{
		InorderTraversal(*this, f);
	}

	void SetNext(AST<content> &sibling)
	{
		Next = &sibling;
	}

	AST<content> GetNext()
	{
		return Next;
	}
};