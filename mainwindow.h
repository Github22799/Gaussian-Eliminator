#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <boost/rational.hpp>

using boost::rational;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void init();
    void solve();
    void printLine(int i, rational<int> **array, int biggestNumLen, int firstRow);
    void print2Matrices(rational<int> **arr1, rational<int> **arr2, QString text);
    void print();
    void write(QString s);
    QString CheckIfAllZeroesAndSwap();
    void printAndCopy(QString text, QString message);
    void setRemainingToZero();
    void reset1();
    void reset2();
    bool allValuesInRowIsZero(int row);
    void printResults(bool canProceed);
    void addRow(int i);
    bool solve1();
    void solve2();
    void applyOnNonZeroInRow(int n);
    void multAndAddColumn(int c1, rational<int> val, int c2);


private slots:
    void on_setVarNum_clicked();

    void on_add_clicked();

    void on_solve_clicked();

    void on_reset_clicked();

    void on_cridits_clicked();

    void on_update_clicked();

    void on_equations_currentIndexChanged(int index);

    void on_remove_clicked();

    void on_jordan_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
