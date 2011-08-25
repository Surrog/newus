
#ifndef POOL_HPP
#define POOL_HPP

#include <stack>
#include <vector>

#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>

template <typename T>
class Pool {
public:
	Pool() 
		: index(0), container(), trash() {
		container.push(std::vector< T >());
		container.top().reserve(8);
	}
	
	Pool(const Pool& orig) 
		: index(orig.index), container(orig.container), trash(orig.trash)
	{}

	boost::shared_ptr< T > getObj() {
		boost::shared_ptr< T > ptr;
		if (trash.size()) {
			ptr = boost::shared_ptr<T>(trash.top(), boost::bind(&Pool::redeemObj, this, _1));
			trash.pop();
			return ptr;
		}

		if (index >= container.top().size()) {
			unsigned newSize = container.top().size() * 2;
			container.push(std::vector< T >());
			container.top().reserve(newSize);
			index = 0;
		}
		
		ptr = boost::shared_ptr<T>(&container.top()[index], boost::bind(&Pool::redeemObj, this, _1));
		index++;
		
		return ptr;
	}

	void redeemObj(T* value) {
		trash.push(value);
	}

private:
	unsigned int index;
	std::stack< std::vector< T > > container;
	std::stack< T* > trash;
};


#endif /* POOL_HPP */