#ifndef TRACABLE_H
#define TRACABLE_H

template<typename T>
class Tracable
{
public:
	Tracable() 
	{
		;
	}

	Tracable( const T& inst )
	{
		this->inst = inst;
	}

	operator T()
	{
		return inst;
	}

	void operator = ( const T& inst )
	{
		this->inst = inst;
	}

private:
	T inst;
};

#endif