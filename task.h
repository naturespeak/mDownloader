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

#ifndef _TASK_H
#define _TASK_H

#include <sys/types.h>
#include <stdio.h>

#include "macro.h"
#include "url.h"
#include "proxy.h"

class Task
{
	public:
		Task(void);
		~Task(void);

	public:
		const char* get_local_dir(void);
		const char* get_local_file(void);
		const char* get_referer(void);
		void set_local_dir(const char *dir);
		void set_local_file(const char *file);
		void set_referer(const char *referer);
		Task& operator = (Task& task);

	public:
		off_t fileSize;
		bool isDirectory;
		bool resumeSupported;
		int tryCount;
		long retryInterval;
		long timeout;
		int ftpActive;
		int threadNum;
		URL url;
		Proxy proxy;

	private:
		char *localDir;
		char *localFile;
		char *referer;
};

#endif // _TASK_H
