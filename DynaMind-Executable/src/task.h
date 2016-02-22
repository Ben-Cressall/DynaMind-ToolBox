#ifndef TASK_H
#define TASK_H

#include <QObject>

class Task : public QObject
{
    Q_OBJECT
private:
    int  argc;
    char **argv;
    char **envp;

	std::string unzipDataStructure(const std::string & workspace_uuid,
								   const std::string & simulationfile);

public:
    Task(int & argc, char ** argv, char **envp, QObject *parent = 0) : argc(argc), argv(argv), envp(envp), QObject(parent) {}

public slots:
    void run();

signals:
    void finished();
};

#endif // TASK_H
