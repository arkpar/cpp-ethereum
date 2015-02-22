/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file FileIo.cpp
 * @author Arkadiy Paronyan arkadiy@ethdev.com
 * @date 2015
 * Ethereum IDE client.
 */

#include <QJsonObject>
#include <QDebug>
#include <QDirIterator>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>
#include <json/json.h>
#include <libdevcrypto/CryptoPP.h>
#include <libdevcrypto/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcrypto/SHA3.h>
#include "FileIo.h"

using namespace dev;
using namespace dev::crypto;
using namespace dev::mix;

void FileIo::makeDir(QString const& _url)
{
	QUrl url(_url);
	QDir dirPath(url.path());
	if (!dirPath.exists())
		dirPath.mkpath(dirPath.path());
}

QString FileIo::readFile(QString const& _url)
{
	QUrl url(_url);
	QString path(url.path());
	if (url.scheme() == "qrc")
		path = ":" + path;
	QFile file(path);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream stream(&file);
		QString data = stream.readAll();
		return data;
	}
	else
		error(tr("Error reading file %1").arg(_url));
	return QString();
}

void FileIo::writeFile(QString const& _url, QString const& _data)
{
	QUrl url(_url);
	QFile file(url.path());
	if (file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QTextStream stream(&file);
		stream << _data;
	}
	else
		error(tr("Error writing file %1").arg(_url));
}

void FileIo::copyFile(QString const& _sourceUrl, QString const& _destUrl)
{
	if (QUrl(_sourceUrl).scheme() == "qrc")
	{
		writeFile(_destUrl, readFile(_sourceUrl));
		return;
	}

	QUrl sourceUrl(_sourceUrl);
	QUrl destUrl(_destUrl);
	if (!QFile::copy(sourceUrl.path(), destUrl.path()))
		error(tr("Error copying file %1 to %2").arg(_sourceUrl).arg(_destUrl));
}

QString FileIo::getHomePath() const
{
	return QDir::homePath();
}

void FileIo::moveFile(QString const& _sourceUrl, QString const& _destUrl)
{
	QUrl sourceUrl(_sourceUrl);
	QUrl destUrl(_destUrl);
	if (!QFile::rename(sourceUrl.path(), destUrl.path()))
		error(tr("Error moving file %1 to %2").arg(_sourceUrl).arg(_destUrl));
}

bool FileIo::fileExists(QString const& _url)
{
	QUrl url(_url);
	QFile file(url.path());
	return file.exists();
}

QString FileIo::sha3(QString const& _url)
{
	QUrl url(_url);
	QString path(url.path());
	QFile file(path);
	dev::h256 h;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QByteArray data = file.readAll();
		h = dev::sha3(dev::bytesConstRef(reinterpret_cast<unsigned char*>(data.data()), data.size()));
	}
	QString ret = QString::fromStdString(toHex(h.ref()));
	return ret;
}

QStringList FileIo::compress(QString const& _deploymentFolder)
{

	Json::Value manifest;
	Json::Value entries(Json::arrayValue);

	QUrl folder(_deploymentFolder);
	QString path(folder.path());
	QDir deployDir = QDir(path);

	dev::RLPStream str;
	int k = 1;
	for (auto item: deployDir.entryInfoList(QDir::Files))
	{
		QFile qFile(item.filePath());
		if (qFile.open(QIODevice::ReadOnly))
		{
			QFileInfo i = QFileInfo(qFile.fileName());
			QByteArray _a = qFile.readAll();
			bytes b = bytes(_a.begin(), _a.end());

			Json::Value f;
			f["path"] = (i.fileName() == "index.html") ? "/" : "/" + i.fileName().toStdString(); //TODO: Manage relative sub folder
			f["file"] = "/" + i.fileName().toStdString();
			f["contentType"] = "text/html"; //TODO: manage multiple content type
			f["hash"] = toHex(dev::sha3(b).ref());
			entries.append(f);
			k++;
		}

	}
	str.appendList(k);

	manifest["entries"] = entries;

	std::stringstream jsonStr;
	jsonStr << manifest;
	QByteArray b =  QString::fromStdString(jsonStr.str()).toUtf8();
	str.append(bytes(b.begin(), b.end()));


	for (auto item: deployDir.entryInfoList(QDir::Files))
	{
		QFile qFile(item.filePath());
		if (qFile.open(QIODevice::ReadOnly))
		{
			QFileInfo i = QFileInfo(qFile.fileName());
			QByteArray _a = qFile.readAll();
			bytes b = bytes(_a.begin(), _a.end());
			str.append(b);

			Json::Value f;
			f["path"] = (i.fileName() == "index.html") ? "/" : "/" + i.fileName().toStdString(); //TODO: Manage relative sub folder
			f["file"] = "/" + i.fileName().toStdString();
			f["contentType"] = "text/html"; //TODO: manage multiple content type
			f["hash"] = toHex(dev::sha3(b).ref());
			entries.append(f);
		}
		qFile.close();
	}

	bytes dapp = str.out();
	dev::h256 h = dev::sha3(dapp);

	//encrypt
	KeyPair key(h);
	Secp256k1 enc;
	enc.encrypt(key.pub(), dapp);

	QUrl url(_deploymentFolder + "package.dapp");
	QFile compressed(url.path());
	QByteArray qFileBytes((char*)dapp.data(), dapp.size());
	if (compressed.open(QIODevice::WriteOnly))
	{
		compressed.write(qFileBytes);
		compressed.flush();
	}
	else
		error(tr("Error creating package.dapp"));
	compressed.close();
	QStringList ret;
	ret.append(QString::fromStdString(toHex(h.ref())));
	ret.append(qFileBytes.toBase64());
	return ret;
}

