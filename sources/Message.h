#pragma once
#include <string>
#include <QDateTime>

class Message
{
    static int num;	//уникальный номер, который получает id
	static int messageCounter;
	int _id;
    int _senderID;
	std::string _sender;
	int _destID;
	std::string _text;
    QDateTime _timestamp;
public:
	Message();
    Message(int senderID, std::string writer, std::string text);
    Message(int senderID, std::string writer, int target, std::string text);
    Message(int senderID, std::string writer, int target, std::string text, QDateTime timestamp);

  bool searchByTarget(int) const;
  std::string getSender() const;
  int getSenderId() const;
  int getDest() const;
  int getID() const;
  std::string getText() const;
  QDateTime getTimestamp() const;
};
