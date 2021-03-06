
#ifndef ARTICLE_H
#define ARTICLE_H

#include <string>
#include <deque>

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/smart_ptr.hpp>

struct Article {
	typedef boost::shared_ptr< Article > ArticlePtr;

	Article() 
		: guid(), title(), link(), body(), author(), category(), pubDate(boost::posix_time::not_a_date_time)
	{}

	std::string guid;
	std::string title;
	std::string link;
	std::string body;
	std::string author;
	std::string comments;
	std::deque< std::string > category;
	boost::posix_time::ptime pubDate;
};

#endif /* ARCTICLE_H */

