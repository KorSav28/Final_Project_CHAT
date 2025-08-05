#pragma once
#include <string>
#include <QDateTime>

class Message
{
	static int num;		//уникальный номер, который получает id
	static int messageCounter;
	int _id;
	std::string _sender;
	int _destID;
	std::string _text;
    QDateTime _timestamp; //????
public:
	Message();
	Message(std::string writer, std::string text);//сообщение в чат
	Message(std::string writer, int target, std::string text);//личное сообщение
    Message(std::string writer, int target, std::string text, QDateTime timestamp);//???

  bool searchByTarget(int) const;
  std::string getSender() const;
  int getDest() const;
  int getID() const;
  std::string getText() const;
  QDateTime getTimestamp() const; //???
};
