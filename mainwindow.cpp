#include "mainwindow.h"
#include "lexerbridge.h"
#include "parser.h"
#include "semantic.h"
#include "ui_mainwindow.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Analizador Semantico");
    ui->tablaTokens->horizontalHeader()->setStretchLastSection(true);
    ui->tablaTokens->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tablaTokens->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tablaSimbolos->horizontalHeader()->setStretchLastSection(true);
    ui->tablaSimbolos->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(ui->botonAnalizar, &QPushButton::clicked, this, &MainWindow::analizarCodigo);
    connect(ui->botonLimpiar, &QPushButton::clicked, this, &MainWindow::limpiarCodigo);
    connect(ui->botonEjemplo, &QPushButton::clicked, this, &MainWindow::cargarEjemplo);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::analizarCodigo()
{
    const QString codigo = ui->editorCodigo->toPlainText();
    if (codigo.trimmed().isEmpty()) {
        QMessageBox::information(this, "Analizador Semantico", "Escribe o carga codigo antes de analizar.");
        return;
    }

    const QList<Token> tokens = analizarConFlex(codigo);
    ui->tablaTokens->setRowCount(tokens.size());
    for (int fila = 0; fila < tokens.size(); ++fila) {
        ui->tablaTokens->setItem(fila, 0, new QTableWidgetItem(tokens.at(fila).lexema));
        ui->tablaTokens->setItem(fila, 1, new QTableWidgetItem(tokens.at(fila).tipo));
        ui->tablaTokens->setItem(fila, 2, new QTableWidgetItem(QString::number(tokens.at(fila).linea)));
    }
    const ResultadoSintactico resultado = analizarSintaxis(tokens);
    ui->tablaSimbolos->setRowCount(0);
    if (resultado.valido) {
        const ResultadoSemantico semantica = analizarSemantica(tokens);
        ui->tablaSimbolos->setRowCount(semantica.simbolos.size());
        for (int fila = 0; fila < semantica.simbolos.size(); ++fila) {
            const SimboloSemantico &simbolo = semantica.simbolos.at(fila);
            ui->tablaSimbolos->setItem(fila, 0, new QTableWidgetItem(simbolo.nombre));
            ui->tablaSimbolos->setItem(fila, 1, new QTableWidgetItem(simbolo.tipo));
            ui->tablaSimbolos->setItem(fila, 2, new QTableWidgetItem(simbolo.categoria));
            ui->tablaSimbolos->setItem(fila, 3, new QTableWidgetItem(simbolo.ambito));
            ui->tablaSimbolos->setItem(fila, 4, new QTableWidgetItem(QString::number(simbolo.linea)));
        }
        if (semantica.valido) {
            ui->labelEstado->setText("Sintaxis y semantica validas");
            ui->labelEstado->setStyleSheet("color: #147a32; font-weight: bold;");
            ui->resultadoSintactico->setPlainText("ANALISIS COMPLETADO SIN ERRORES\n\nLa tabla de simbolos contiene las declaraciones encontradas.");
        } else {
            ui->labelEstado->setText(QString("Semantica invalida: %1 error(es)").arg(semantica.errores.size()));
            ui->labelEstado->setStyleSheet("color: #b42318; font-weight: bold;");
            ui->resultadoSintactico->setPlainText("ERRORES SEMANTICOS\n\n" + semantica.errores.join('\n'));
        }
    } else {
        ui->labelEstado->setText(QString("Sintaxis invalida: %1 error(es)").arg(resultado.errores.size()));
        ui->labelEstado->setStyleSheet("color: #b42318; font-weight: bold;");
        ui->resultadoSintactico->setPlainText("ERRORES SINTACTICOS\n\n" + resultado.errores.join('\n') +
                                              "\n\nArbol parcial\n" + resultado.arbol);
    }
    ui->labelResumen->setText(QString("Tokens analizados: %1").arg(tokens.size()));
}

void MainWindow::limpiarCodigo()
{
    ui->editorCodigo->clear();
    ui->tablaTokens->setRowCount(0);
    ui->tablaSimbolos->setRowCount(0);
    ui->resultadoSintactico->clear();
    ui->labelEstado->setText("Sin analizar");
    ui->labelEstado->setStyleSheet(QString());
    ui->labelResumen->setText("Tokens analizados: 0");
}

void MainWindow::cargarEjemplo()
{
    ui->editorCodigo->setPlainText(
        "int main() {\n"
        "    int edad = 22;\n"
        "    double promedio = 89.5;\n"
        "    bool aprobado = promedio >= 70;\n"
        "    if (aprobado == true) {\n"
        "        edad = edad + 1;\n"
        "    }\n"
        "    return edad;\n"
        "}\n");
}

