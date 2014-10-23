#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Graphics\Texture.h>
#include <QMainWindow>

class QLabel;
class PrjSetupDialog;
class EntBrowserDialog;
class EnvBrowserDialog;
class LayerBrowserDialog;
class ConfigDialog;
class PrjSettingsDialog;
class AnimManagerDialog;
class AudioManagerDialog;
class ItemManagerDialog;

template <typename Resource>
struct Asset;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
        void init();

        inline Ui::MainWindow* getUI() const;
		inline QLabel* getStatusMain() const;
		inline QLabel* getStatusPos() const;
		inline QLabel* getStatusTile() const;

    public slots:
        void onNewTrigger();
        void onOpenTrigger();
		void onSaveTrigger();
		void onSaveAsTrigger();
		void onExportTrigger();
        void onExitTrigger();

    private:
		void closeEvent(QCloseEvent*);

        Ui::MainWindow *ui;
		QLabel* statusMain;
		QLabel* statusPos;
		QLabel* statusTile;
        PrjSetupDialog* prjsetupDialog;
		EntBrowserDialog* entbrowseDialog;
        EnvBrowserDialog* envbrowseDialog;
		Asset<kry::Graphics::Texture>* prevAsset = nullptr;
		LayerBrowserDialog* layerbrowseDialog;
		ConfigDialog* configDialog;
		PrjSettingsDialog* prjsettingsDialog;
		AnimManagerDialog* animmanagerDialog;
		AudioManagerDialog* audiomanagerDialog;
		ItemManagerDialog* itemmanagerDialog;
		bool saved;
};


Ui::MainWindow* MainWindow::getUI() const
{
    return ui;
}

QLabel* MainWindow::getStatusMain() const
{
	return statusMain;
}

QLabel* MainWindow::getStatusPos() const
{
	return statusPos;
}

QLabel* MainWindow::getStatusTile() const
{
	return statusTile;
}

#endif // MAINWINDOW_H
