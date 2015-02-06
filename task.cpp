/*  mDownloader - a multiple-threads downloading accelerator program that is based on Myget.
 *  Homepage: http://qinchuan.me/article.php?id=100
 *  2015 By Richard (qc2105@qq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Myget - A download accelerator for GNU/Linux
 *  Homepage: http://myget.sf.net
 *  2005 by xiaosuo
 */

#include "task.h"
#include "utils.h"

Task::Task()
{
	fileSize = -1;
	isDirectory = false;
	resumeSupported = false;
	tryCount = 99;
	retryInterval = 5;
	timeout = 30;
	ftpActive = PASV;
    threadNum = 10;
	localDir = NULL;
	localFile = NULL;
	referer = NULL;
}

Task::~Task()
{
    delete[] localDir;
	delete[] localFile;
	delete[] referer;
}

Task&
Task::operator = (Task& task)
{
	if(this == &task) return *this;

	delete[] localDir;
	delete[] localFile;
	delete[] referer;
	localDir = StrDup(task.get_local_dir());
	localFile = StrDup(task.get_local_file());
	referer = StrDup(task.get_referer());
	fileSize = task.fileSize;
	isDirectory = task.isDirectory;
	resumeSupported = task.resumeSupported;
	tryCount = task.tryCount;
	retryInterval = task.retryInterval;
	timeout = task.timeout;
	ftpActive = task.ftpActive;
	threadNum = task.threadNum;
	url = task.url;
	proxy = task.proxy;

	return *this;
}
	

const char*
Task::get_local_dir(void)
{
	return localDir;
}

const char*
Task::get_local_file(void)
{
	return localFile;
}

const char*
Task::get_referer(void)
{
	return referer;
}

void
Task::set_local_dir(const char *dir)
{
	delete[] localDir;
	localDir = StrDup(dir);
}

void
Task::set_local_file(const char *file)
{
	delete[] localFile;
	localFile = StrDup(file);
}

void
Task::set_referer(const char *referer)
{
	delete[] this->referer;
	this->referer = StrDup(referer);
}
