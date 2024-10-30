// Experience.h
#ifndef EXPERIENCE_H
#define EXPERIENCE_H

class Experience 
{

public:
  virtual void setup() = 0;
  virtual void run() = 0;
  virtual void teardown() = 0;
  virtual void init() { /* Default implementation (can be empty) */ }

  bool isSetupComplete() const {
    return setupComplete;
  }

  bool isRunComplete() const {
    return runComplete;
  }

  bool isTeardownComplete() const {
    return teardownComplete;
  }

  bool isStopped() const {
    return stopped;
  }

  bool isIdle() const {
    return idle;
  }

protected:
  void setSetupComplete(bool complete) {
    setupComplete = complete;
  }

  void setRunComplete(bool complete) {
    runComplete = complete;
  }

  void setTeardownComplete(bool complete) {
    teardownComplete = complete;
  }

  void setStopped( bool stop )
  {
    stopped = stop;
  }

  void setIdle( bool myidle )
  {
    idle = myidle;
  }

protected:
  bool setupComplete = false;
  bool runComplete = false;
  bool teardownComplete = false;
  bool stopped = false;
  bool idle = false;
};

#endif // EXPERIENCE_H
