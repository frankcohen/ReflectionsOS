// Experience.h
#ifndef EXPERIENCE_H
#define EXPERIENCE_H

class Experience {
public:
  virtual ~Experience() {}

  virtual void setup() = 0;
  virtual void run() = 0;
  virtual void teardown() = 0;

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

private:
    bool setupComplete = false;
    bool runComplete = false;
    bool teardownComplete = false;
    bool stopped = false;
};

#endif // EXPERIENCE_H
