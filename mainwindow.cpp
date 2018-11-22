#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<boost/rational.hpp>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QDir>

using boost::rational;

int vars = 0, eq = 0, lastColumn = 0, lastRow = 0, currIndex = 0;
rational<int> **arr, **copy, *results;
QString console = "";
bool NotFirstTime = false, infflag = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::reset1()
{
    if (NotFirstTime)
    {
        for (int i = 0; i < eq; i++)
        {
            delete[] arr[i];
            delete[] copy[i];
        }

        delete[] results;
        delete[] arr;
        delete[] copy;

        lastColumn = 0; lastRow = 0;

        ui->results->clear();
        ui->steps->clear();

    }
    NotFirstTime = false;

}

void MainWindow::reset2()
{
    vars = 0; eq = 0;
    ui->equations->clear();
    ui->varNum->setValue(1);
    ui->text->clear();
    ui->add->setEnabled(false);
    ui->setVarNum->setEnabled(true);
    ui->solve->setEnabled(false);
    ui->jordan->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_setVarNum_clicked()
{
    ui->add->setEnabled(true);
    ui->setVarNum->setEnabled(false);
    ui->reset->setEnabled(true);
    ui->solve->setEnabled(true);
    ui->jordan->setEnabled(true);
    vars = ui->varNum->value();
    ui->equations->clear();
}

void MainWindow::write(QString s)
{
    ui->steps->moveCursor (QTextCursor::End);
    ui->steps->insertPlainText (s);
//    console += s;
}

QString getNums(QString s)
{
    QString num = "";
    s += ' ';
    int added = 0;
    bool putSpace = false;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i].isDigit() || s[i] == '/' || (!putSpace && s[i] == '-'))
        {
            num += s[i];
            putSpace = true;
        }
        else if (putSpace)
        {
            added++;
            num += " ";
            putSpace = false;
        }

        if (added > vars) break;
    }

    for (int i = added; i <= vars; i++) num += "0 ";

    return num;
}

void MainWindow::on_add_clicked()
{
    ui->equations->addItem(getNums(ui->text->text()));
    ui->text->setText("");
    eq++;
    if (eq > 0)
    {
        ui->update->setEnabled(true);
        ui->remove->setEnabled(true);
    }
}

void setRationalValue(QString s, int x, int y)
{
    QString num1 = "", num2 = "";
    bool num = true;
    for (int i = 0; i < s.length(); i++)
    {
        if (s[i] == '/')
        {
            num = false;
            continue;
        }

        if (num) num1 += s[i];
        else num2 += s[i];
    }

    if (num2 == "") num2 = "1";
    arr[x][y].assign(num1.toInt(), num2.toInt());
}

void MainWindow::addRow(int i)
{
    int v = 0;
    QString s = ui->equations->itemText(i);
    QString num = "";

    for (int j = 0; j < s.size(); j++)
    {

        if (s[j] == ' ')
        {
            setRationalValue(num, i, v++);
            num = "";
        }
        else
            num += s[j];

        if (v > vars) break;
    }

    if (num != "") setRationalValue(num, i, v);
}

void MainWindow::init()
{
    eq = ui->equations->count();

    results = new rational<int>[vars];

    arr = new rational<int>*[eq];

    for (int i = 0; i < eq; i++)
        arr[i] = new rational<int>[vars+1];

    copy = new rational<int>*[eq];

    for (int i = 0; i < eq; i++)
        copy[i] = new rational<int>[vars+1];

    for (int i = 0; i < eq; i++)
    {
        addRow(i);
    }
}

void setNextColumn()
{
    for (int i = 0; i < eq; i++)
    {
        if (arr[i][lastColumn] != 0) break;
        else if (arr[i][lastColumn] == 0 && i == eq -1)
        {
            lastColumn++;
            i = -1;
            continue;
        }
    }
}

QString MainWindow::CheckIfAllZeroesAndSwap()
{

    if (arr[lastRow][lastColumn] == 0)
    {
        for (int i = lastRow+1; i < eq; i++)
        {
            if (arr[i][lastColumn] != 0)
            {
                rational<int> *temp = arr[lastRow];
                arr[lastRow] = arr[i];
                arr[i] = temp;
                return "Substituting R" + QString::number(lastRow+1) + ", R" + QString::number(i+1);
            }
            else if (i == eq - 1)
            {
                results[lastColumn] = 1000000;
                i = 0;
                bool infinite = true;
                for (int j = lastRow; j >= 0; j--) if (arr[j][lastColumn] != 0) infinite = false;

                lastColumn++;

                if (infinite)
                    write("\nx" + QString::number(lastColumn) + " has an infinte number of values.\n");

                if (lastColumn >= vars) break;
            }
        }
    }
    return "";
}

QString getRational(rational<int> r)
{
    if (r.numerator() == 1000000) return "∞";

    if (r.denominator() == 1) return QString::number(r.numerator());

    return QString::number(r.numerator()) + '/' + QString::number(r.denominator());
}

QString setToOne()
{
    rational<int> val = arr[lastRow][lastColumn];
    if (val != 1 && val != 0)
    {
        for (int i = 0; i <= vars; i++)
            arr[lastRow][i] /= val;

        return "Dividing R" + QString::number(lastRow + 1) + " by " + getRational(val);
    }
    return "";
}

void copyArr()
{
    for (int i = 0; i < eq; i++)
    {
        for (int j = 0; j <= vars; j++)
            copy[i][j] = arr[i][j];
    }
}

void MainWindow::printAndCopy(QString text, QString message)
{
    if (text != "")
    {
        text = "═══ " + text + " ";
        for (int i = text.size(); i < 40; i++) text += "═";
        text += '>';

        write(message + "\n");
        print2Matrices(copy, arr, text);
        write("\n\n");
        copyArr();
    }
}

inline void multiplyAndSubtractFromRow(int row, rational<int> val)
{
    for (int i = 0; i <= vars; i++)
    {
        arr[row][i] -= arr[lastRow][i] * val;
    }
}

void MainWindow::setRemainingToZero()
{
   rational<int> val;

   bool proceed = false;
   for (int i = lastRow+1; i < eq; i++)
   {
       if (arr[i][lastColumn] != 0)
       {
           proceed = true;
           break;
       }
   }

   if (!proceed) return;

   write("Transforming elements below M[" + QString::number(lastRow+1) + ',' + QString::number(lastColumn+1) + "] to zero\n");

   for (int i = lastRow+1; i < eq; i++)
   {
       val = arr[i][lastColumn] / arr[lastRow][lastColumn];
       multiplyAndSubtractFromRow(i, val);
       printAndCopy("Adding -" + getRational(val) + " * " + "R" + QString::number(lastRow+1) + " to R" + QString::number(i+1), "");
   }
}

int max(int i, int j)
{
    if (i > j) return i;
    return j;
}

int min(int i, int j)
{
    if (i < j) return i;
    return j;
}

QString nSpaces(int n)
{
    QString res = "";
    for (int i = 0; i < n; i++)
        res += ' ';
    return res;
}

void MainWindow::printLine(int i, rational<int> **array, int biggestNumLen, int firstRow)
{
    QString num, spaces = "";
    write("║ ");
    for (int j = 0; j <= vars; j++)
    {
        num = getRational(array[i][j]);

        if (num == "1000000") num = "∞";

        spaces = "";
        int size = num.size();

        if (j == 0)
            for (int k = size; k < firstRow; k++) spaces += ' ';
        else
            for (int k = size; k < biggestNumLen; k++) spaces += ' ';

        write(spaces);
        write(num);

        if (j == vars - 1) write(" ║");
    }

    write(" ║");
}

void MainWindow::print2Matrices(rational<int> **arr1, rational<int> **arr2 = nullptr, QString text = "")
{
    int biggestNumLen = 0;

    for (int i = 0; i < eq; i++)
    {
        for (int j = 0; j <= vars; j++)
        {
            biggestNumLen = max(biggestNumLen, getRational(arr1[i][j]).length());
            if (arr2 != nullptr) biggestNumLen = max(biggestNumLen, getRational(arr2[i][j]).length());
        }
    }

    int arr1row = 0, arr2row = 0;

    for (int i = 0; i < eq; i++)
    {
        arr1row = max(arr1row, getRational(arr1[i][0]).length());
        if (arr2 != nullptr) arr2row = max(arr1row, getRational(arr2[i][0]).length());
    }

    biggestNumLen += 2;
    int mid = eq/2;
    QString spaces = "  ";
    for (int i = 0; i < text.length()+2; i++) spaces += ' ';

    QString space1 = nSpaces(4 + (vars) * biggestNumLen + arr1row);
    QString space2 = nSpaces(4 + (vars) * biggestNumLen + arr2row);

    write("╔" + space1 + "╗");

    if (arr2 != nullptr)
    {
        write(nSpaces(45) + "╔" + space2 + "╗");
    }

    write("\n");

    for (int i = 0; i < eq; i++)
    {
        printLine(i, arr1, biggestNumLen, arr1row);

        if (arr2 != nullptr)
        {
            if (i == mid) write("  " + text + "  ");
            else write(spaces);

            printLine(i, arr2, biggestNumLen, arr2row);
        }

        write("\n");
    }
    write("╚" + space1 + "╝");

    if (arr2 != nullptr)
    {
        write(nSpaces(45) + "╚" + space2 + "╝");
    }
    write("\n");
}

void MainWindow::print()
{
    QString s;
    for (int i = 0; i < eq; i++)
    {
        for (int j = 0; j <= vars; j++)
        {
            s = QString::number(arr[i][j].numerator());
            if (arr[i][j].denominator() != 1)
                s += '/' + QString::number(arr[i][j].denominator());

            write(s + ' ');
        }
        write("\n");
    }
    write("\n");
}

bool MainWindow::allValuesInRowIsZero(int row)
{
    for (int i = 0; i < vars; i++) if (arr[row][i] != 0) return false;
    return true;
}

void swapRows(int row1, int row2)
{
    auto temp = arr[row1];
    arr[row1] = arr[row2];
    arr[row2] = temp;
}

void moveRowToTheButtom(int row)
{
    for (int i = row; i < eq - 1; i++)
    {
        swapRows(i, i+1);
    }
}

bool allZerosFrom(int index)
{
    for (; index < eq; index++)
        for (int i = 0; i <= vars; i++)
            if (arr[index][i] != 0)
                return false;
    return true;
}

void MainWindow::printResults(bool canProceed)
{
    QString text;
    if (canProceed)
    {
        write("Final Matrix:\n");
        print2Matrices(arr);

        write("\n\n");
        rational<int> res;
        QString simplified;
        for (int i = 0; i < eq && lastColumn > 0 && lastRow > 0; i++)
        {
            lastColumn--; lastRow--;
            res = arr[lastRow][vars];
            text = "x" + QString::number(lastColumn+1) + " = " + getRational(res);
            simplified = "x" + QString::number(lastColumn+1) + " = " + getRational(res);

            for (int j = vars-1; j > lastColumn; j--)
            {
                text += " - (" + getRational(arr[lastRow][j]) + " * " + getRational(results[j]) + ")";

                if (arr[lastRow][j] * results[j] >= 0)
                    simplified += " - " + getRational(arr[lastRow][j] * results[j]);
                else
                    simplified += " + " + getRational(arr[lastRow][j] * results[j]);

                res -= arr[lastRow][j] * results[j];
            }
            results[lastRow] = res;
            write(text + "\n");
            if (text != simplified) write(simplified + "\n");
            simplified = "x" + QString::number(lastColumn+1) + " = " + getRational(res);
            if (text != simplified) write(simplified + "\n");
            write("\n");
        }

        for (int i = 0; i < vars; i++)
        {
            double d = static_cast<double>(results[i].numerator()) / results[i].denominator();
            QString s = "x" + QString::number(i+1) + " = " + getRational(results[i]);
            if (results[i].denominator() != 1 && results[i].numerator() != 0) s += "  (" + QString::number(d) + ")";
            ui->results->addItem(s);
        }
    }
    else
    {
        for (int i = 0; i < vars; i++)
        {
            ui->results->addItem("x" + QString::number(i+1) + " = ∞");
            write("x" + QString::number(i+1) + " = ∞\n");
        }
    }
}

bool MainWindow::solve1()
{
    reset1();
    init();

    if (eq == 0)
    {
        write("Add at least " + QString::number(vars) + " equations.\n\n");
        return false;
    }else if (eq < vars)
    {
        write("The number of equations is less than the number of variables, Not enough information, Cannot proceed.\n\n");
        infflag = true;
        return false;
    }

    bool canProceed = true;

    int numOfEq = eq;

    NotFirstTime = true;


    int counter = 0;
    QString text;
    while (lastColumn < vars && lastRow < eq && canProceed)
    {
        counter++;

        if (allValuesInRowIsZero(lastRow))
        {
            if (arr[lastRow][vars] == 0)
            {
                numOfEq--;
                if (lastRow != eq - 1 && !allZerosFrom(lastRow))
                {
                    write("Row " + QString::number(lastRow+1) + " will be moved to the buttom for the ease of reading.\n\n");
                    moveRowToTheButtom(lastRow);
                }
            }
            else
            {
                write("The function number " + QString::number(lastRow+1) + " is indicating that sum of zeroes is " + getRational(arr[lastRow][vars]) + " which is wrong. Cannot proceed.\n\n");
                canProceed = false;
            }
        }

        if (numOfEq < vars && canProceed)
        {
            write("The number of equations is less than the number of variables, Not enough information, Cannot proceed.\n\n");
            infflag = true;
            canProceed = false;
        }

        if (!canProceed) break;

        setNextColumn();

        copyArr();

        bool writeRowNumber = false;

        text = CheckIfAllZeroesAndSwap();

        if (text != "" && !writeRowNumber)
        {
            writeRowNumber = true;
            write("Row Number " + QString::number(counter) + ":════════════════════\n\n");
        }

        printAndCopy(text, "Substituting M[" + QString::number(lastRow+1) + ',' + QString::number(lastColumn+1) + "] with a non-zero element");

        text = setToOne();

        if (text != "" && !writeRowNumber)
        {
            writeRowNumber = true;
            write("Row Number " + QString::number(counter) + ":════════════════════\n\n");
        }

        printAndCopy(text, "Transforming m[" + QString::number(lastRow+1) + ',' + QString::number(lastColumn+1) + "] to 1");

        setRemainingToZero();

        write("\n\n");

        if (lastColumn >= vars || lastRow >= eq) break;

        lastColumn++;
        lastRow++;
    }
    return canProceed;
}

void MainWindow::multAndAddColumn(int c1, rational<int> val, int c2)
{
    for (int i = 0; i <= vars; i++)
    {
        arr[c2][i] += arr[c1][i] * val;
    }
}

void MainWindow::applyOnNonZeroInRow(int n)
{
    rational<int> val;
    for (int i = n-1; i >= 0; i--)
    {
        if (arr[i][n] != 0)
        {
            if (arr[n][n] == 0)
                val = 0;
            else
                val = -arr[i][n]/arr[n][n];
            multAndAddColumn(n, val, i);

            std::string s1 = "Removing all the elements above the leading one in column ";
            std::string s2 = std::to_string(n+1);
            std::string s3 = s1 + s2;
            char const *pchar = s3.c_str();

            printAndCopy("R" + QString::number(i+1) + " = R" + QString::number(i+1) + " + R" + QString::number(n+1) + " * " + getRational(val), pchar);
        }
    }
}

void MainWindow::jordan()
{
    for (int i = vars-1; i >= 0; i--)
    {
        if (infflag) break;
        applyOnNonZeroInRow(i);
    }
}

void MainWindow::on_solve_clicked()
{
    printResults(solve1());
}

void MainWindow::on_reset_clicked()
{
//    ui->equations->addItem("1 1 1 3");
//    ui->equations->addItem("1 2 2 5");
//    ui->equations->addItem("2 1 1 4");
//    ui->equations->addItem("2 4 3 2 0");
//    ui->equations->addItem("1 -2 5 2 2");
//    ui->equations->addItem("0 2 4 5 -1");
//    ui->equations->addItem("0 0 -2 1 -3");
//    ui->equations->addItem("1 1 1 6/7");
//    ui->equations->addItem("1 3 -1 0");
//    ui->equations->addItem("3 2 4 3");
//    ui->equations->addItem("-2 1 2 2");
//    ui->equations->addItem("2 -1 3 4");
//    ui->equations->addItem("-3 2 1 5");
//    ui->equations->addItem("-4 2 -6 -8");
//    ui->equations->addItem("-4 2 -6 1");
//    ui->equations->addItem("5 3 2 -2");
//    ui->equations->addItem("5 3 2 -2");
//    ui->equations->addItem("5 3 2 -2");
//    ui->equations->addItem("1 1 0 2");
//    ui->equations->addItem("2 -1 1 3");
//    eq += 4;
    reset1();
    reset2();
}

void MainWindow::on_cridits_clicked()
{
    write("\n\n\nCredits: \n");
    QFile credits(QDir::current().absolutePath() + "/credits");
    if (!credits.open(QIODevice::ReadOnly))
    {
        write("Ahmed Sadeek\nAhmed Habeeb\nAhmed Elgharbawy\nAhmed Rehan\nOsama Ahmad\nAlsaid Gamal\nAsmaa Mhmd\nAyat Alaa\nEsraa El Ashry\nRokia Elsharkawy\n\n\n");
    }
    else
    {
        QTextStream s(&credits);
        write(s.readAll() + "\n\n\n");
    }


}

void MainWindow::on_update_clicked()
{
    ui->equations->setItemText(currIndex, getNums(ui->text->text()));
}

void MainWindow::on_equations_currentIndexChanged(int index)
{
    currIndex = index;
    ui->text->setText(ui->equations->itemText(index));
}

void MainWindow::on_remove_clicked()
{
    ui->equations->removeItem(currIndex);
    eq--;
    if (eq <= 0)
    {
        ui->update->setEnabled(false);
        ui->remove->setEnabled(false);
    }
}

void MainWindow::on_jordan_clicked()
{
    bool canProceed = solve1();
    jordan();
    printResults(canProceed);
}
