#include <QObject>

class Scene : public QObject {
  Q_OBJECT

 public:
  Scene(QObject* parent = 0);
  void getStats(int* frames, double* min, double* max, double* average);

 private slots:
  void beforeRendering();
  void afterRendering();

 private:
  struct timespec t1_;
  struct timespec t2_;
  qint64 frames_;
  double sum_;
  double min_;
  double max_;
};
