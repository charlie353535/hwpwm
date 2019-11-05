#ifndef SYSFS_H
#define SYSFS_H

extern int createattrs(struct device* pdev);
extern int delattrs(struct device* pdev);
extern void tvars(unsigned char* _fanbuf);

#endif

