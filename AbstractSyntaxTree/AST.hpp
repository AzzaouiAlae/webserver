#pragma once

#include "../Headers.hpp"

template <typename content>
class AST
{
	content Value;
	std::vector<AST<content> > Children;
	std::vector<content> Arguments;

	void PreorderTraversal(AST<content> root, void (*f)(content, int), int level = 0)
	{
		f(root.Value, level);
		for (int i = 0; i < (int)root.Arguments.size(); i++)
		{
			// std::cout << "--";
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
		for (int i = 0; i < (int)root.Children.size(); i++)
		{
			if (visited == false && (int)(root.Children.size() / 2) <= i)
			{
				visited = true;
				f(root.Value, level);
				for (int i = 0; i < (int)root.Arguments.size(); i++)
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

};