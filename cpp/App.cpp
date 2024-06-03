#include "../header/App.h"
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <stdexcept>

App::App(QWidget *parent) : QWidget(parent), logger("../logs/app_log.txt") {
    setWindowTitle("Word Occurrence Table");
    resize(800, 600);

    QGridLayout * gridLayout = new QGridLayout(this);

    plainTextEdit = new QPlainTextEdit(this);
    plainTextEdit->setReadOnly(true);
    QPushButton *button = new QPushButton("Select Files To Process", this);

    gridLayout->addWidget(plainTextEdit, 0, 0, 1, 2);
    gridLayout->addWidget(button, 1, 0, 1, 2);

    connect(button, &QPushButton::clicked, this, &App::processFiles);
}

std::string App::removePunctuation(const std::string &word) {
    std::string cleanedWord;
    std::remove_copy_if(word.begin(), word.end(), std::back_inserter(cleanedWord), [](char c) {
        return std::ispunct(c);
    });
    return cleanedWord;
}

void App::processFiles() {
    try {
        QString wordsFileName = QFileDialog::getOpenFileName(this, "Select Words File", "../files/",
                                                             "Text Files (*.txt);");
        if (wordsFileName.isEmpty()) return;

        QString keywordsFileName = QFileDialog::getOpenFileName(this, "Select Keywords File", "../files/",
                                                                "Text Files (*.txt);");
        if (keywordsFileName.isEmpty()) return;

        std::ifstream wordsFile(wordsFileName.toStdString());
        std::ifstream keywordsFile(keywordsFileName.toStdString());

        if (!wordsFile.is_open() || !keywordsFile.is_open()) {
            throw std::runtime_error("Unable to open words or keywords file");
        }

        std::set<std::string> keywords;
        std::string keyword;
        while (keywordsFile >> keyword) {
            keywords.insert(removePunctuation(keyword));
        }

        std::map<std::string, std::vector<std::pair<int, int>>> wordOccurrences;
        std::string line;
        int lineNum = 1;

        while (std::getline(wordsFile, line)) {
            std::istringstream lineStream(line);
            std::string word;
            int columnNum = 1;

            while (lineStream >> word) {
                std::string cleanedWord = removePunctuation(word);
                if (keywords.count(cleanedWord) == 0) {
                    wordOccurrences[cleanedWord].emplace_back(lineNum, columnNum);
                }
                columnNum++;
            }
            lineNum++;
        }

        std::vector<std::string> sortedWords;
        for (const auto &entry: wordOccurrences) {
            sortedWords.push_back(entry.first);
        }
        std::sort(sortedWords.begin(), sortedWords.end());

        QString outputFileName = QFileDialog::getSaveFileName(this, "Save Output File", "../files/",
                                                              "Text Files (*.txt);");
        if (outputFileName.isEmpty()) return;

        std::ofstream outputFile(outputFileName.toStdString());
        if (!outputFile.is_open()) {
            throw std::runtime_error("Unable to open output file");
        }

        std::ostringstream outputContent;
        for (const auto &word: sortedWords) {
            outputFile << word << " ";
            outputContent << word << " ";
            for (const auto &pos: wordOccurrences[word]) {
                outputFile << "(" << pos.first << ", " << pos.second << ") ";
                outputContent << "(" << pos.first << ", " << pos.second << ") ";
            }
            outputFile << std::endl;
            outputContent << std::endl;
        }

        outputFile.close();
        plainTextEdit->setPlainText(QString::fromStdString(outputContent.str()));
        logger.logMessage("Processing completed successfully", "INFO");
        QMessageBox::information(this, "Success", "Processing completed successfully");
    } catch (const std::exception &e) {
        logger.logMessage(e.what(), "ERROR");
        QMessageBox::critical(this, "Error", e.what());
    }
}
