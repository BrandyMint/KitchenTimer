#include "pagesettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QScrollBar>
#include <QPainter>
#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QScreen>


#define KITCHENTIMER_SLIDE_VIBRATION_ON_TIMEOUT_default 200
#define KITCHENTIMER_SLIDE_VIBRATION_SILENCE_DURATION_default 100
#define KITCHENTIMER_SLIDE_VIBRATION_VIBRATION_DURATION_default 1
#define KITCHENTIMER_ANALOG_TIMER_MAX_FOLLOW_EXTENT_default 90
#define KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS_default 100
#define KITCHENTIMER_EDIT_HOLD_TIMEOUT_MS_default 150
#define KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS_default 20
#define KITCHENTIMER_INITIAL_EDIT_HOLD_TIMEOUT_MS_default 30
#define KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS_default 1000
#define KITCHENTIMER_ANALOG_TIMER_MODE_default 1
#define KITCHENTIMER_ANALOG_TIMER_MODE_count 3
#define KITCHENTIMER_DIGITAL_TIMER_MODE_default 0
#define KITCHENTIMER_DIGITAL_TIMER_MODE_count 2

int KITCHENTIMER_SLIDE_VIBRATION_ON_TIMEOUT = KITCHENTIMER_SLIDE_VIBRATION_ON_TIMEOUT_default;
int KITCHENTIMER_SLIDE_VIBRATION_SILENCE_DURATION = KITCHENTIMER_SLIDE_VIBRATION_SILENCE_DURATION_default;
int KITCHENTIMER_SLIDE_VIBRATION_VIBRATION_DURATION = KITCHENTIMER_SLIDE_VIBRATION_VIBRATION_DURATION_default;
int KITCHENTIMER_ANALOG_TIMER_MAX_FOLLOW_EXTENT = KITCHENTIMER_ANALOG_TIMER_MAX_FOLLOW_EXTENT_default;
int KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS = KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS_default;
int KITCHENTIMER_EDIT_HOLD_TIMEOUT_MS = KITCHENTIMER_EDIT_HOLD_TIMEOUT_MS_default;
int KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS = KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS_default;
int KITCHENTIMER_INITIAL_EDIT_HOLD_TIMEOUT_MS = KITCHENTIMER_INITIAL_EDIT_HOLD_TIMEOUT_MS_default;
int KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS = KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS_default;
int KITCHENTIMER_ANALOG_TIMER_MODE = KITCHENTIMER_ANALOG_TIMER_MODE_default;
int KITCHENTIMER_DIGITAL_TIMER_MODE = KITCHENTIMER_DIGITAL_TIMER_MODE_default;
bool KITCHENTIMER_SHOW_DEBUG_OVERLAY = false;
bool KITCHENTIMER_USE_ANALOG_TIMER_SLIDE_SOUND = false;
bool KITCHENTIMER_USE_VIBRATION = true;

PageSettings::PageSettings (QWidget *parent)
    : QScrollArea (parent), font (qApp->font ())
{
    QRadioButton *radio_button;
    QGroupBox *group_box;
    QButtonGroup *button_group;
    QHBoxLayout *hlayout;

    QWidget *scroll_widget = new QWidget (this);
    setWidget (scroll_widget);
    setWidgetResizable (true);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen)
	font.setPixelSize (screen->physicalDotsPerInch ()*0.3);
    else
	font.setPixelSize (20);
    setFont (font);

    QVBoxLayout *layout = new QVBoxLayout (scroll_widget);

    layout->addStretch (1);

    QGridLayout *grid_layout = new QGridLayout ();

    grid_layout->setColumnStretch (1, 1);
    grid_layout->setColumnMinimumWidth (1, 150);

    int row = 0;
    
#define ADD_INT_EDIT_BLOCK(var,title,min_value,max_value) {			\
	grid_layout->addWidget (new QLabel (("" title " (") + QString::number ( \
						var ## _default) + "):", scroll_widget), row, 0); \
	grid_layout->addWidget (slider_ ## var = new QScrollBar (Qt::Horizontal, scroll_widget), row, 1); \
	(slider_ ## var)->setRange (min_value, max_value);		\
	(slider_ ## var)->setValue (var);				\
	connect (slider_ ## var, SIGNAL (valueChanged (int)),		\
		 this, SLOT (update_ ## var (int)));			\
	grid_layout->addWidget (spin_box_ ## var = new QSpinBox (scroll_widget), row, 2); \
	(spin_box_ ## var)->setRange (min_value, max_value);		\
	(spin_box_ ## var)->setValue (var);				\
	connect (spin_box_ ## var, SIGNAL (valueChanged (int)),		\
		 this, SLOT (update_ ## var (int)));			\
	row++;								\
    }

    ADD_INT_EDIT_BLOCK (KITCHENTIMER_SLIDE_VIBRATION_ON_TIMEOUT, "Vibro on timeout", 1, 500)
    ADD_INT_EDIT_BLOCK (KITCHENTIMER_SLIDE_VIBRATION_SILENCE_DURATION, "Vibro silence dur", 1, 200)
    ADD_INT_EDIT_BLOCK (KITCHENTIMER_SLIDE_VIBRATION_VIBRATION_DURATION, "Vibro vibro dur", 1, 200)
    ADD_INT_EDIT_BLOCK (KITCHENTIMER_ANALOG_TIMER_MAX_FOLLOW_EXTENT, "Angle follow extent", 1, 170)
    ADD_INT_EDIT_BLOCK (KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS, "Edit transition to MS", 1, 2000)
    ADD_INT_EDIT_BLOCK (KITCHENTIMER_EDIT_HOLD_TIMEOUT_MS, "edit hold to MS", 1, 2000)
    ADD_INT_EDIT_BLOCK (KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS, "In edit transition tm MS", 1, 2000)
    ADD_INT_EDIT_BLOCK (KITCHENTIMER_INITIAL_EDIT_HOLD_TIMEOUT_MS, "In edit hold tm MS", 1, 2000)
    ADD_INT_EDIT_BLOCK (KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS, "Leave edit mode", 1, 5000)

    layout->addLayout (grid_layout, 0);

    button_group = new QButtonGroup (this);
    group_box = new QGroupBox ("Analog timer mode");
    hlayout = new QHBoxLayout (group_box);
    for (int i = 0; i < KITCHENTIMER_ANALOG_TIMER_MODE_count; ++i) {
	radio_button = new QRadioButton ("M" + QString::number (i));
	hlayout->addWidget (radio_button);
	button_group->addButton (radio_button, i);
	if (i == KITCHENTIMER_ANALOG_TIMER_MODE_default)
	    radio_button->setChecked (true);
    }
    hlayout->addStretch (1);
    layout->addWidget (group_box);
    connect (button_group, SIGNAL (buttonClicked (int)), this, SLOT (update_KITCHENTIMER_ANALOG_TIMER_MODE (int)));

    button_group = new QButtonGroup (this);
    group_box = new QGroupBox ("Digital timer mode");
    hlayout = new QHBoxLayout (group_box);
    for (int i = 0; i < KITCHENTIMER_DIGITAL_TIMER_MODE_count; ++i) {
	radio_button = new QRadioButton ("M" + QString::number (i));
	hlayout->addWidget (radio_button);
	button_group->addButton (radio_button, i);
	if (i == KITCHENTIMER_DIGITAL_TIMER_MODE_default)
	    radio_button->setChecked (true);
    }
    hlayout->addStretch (1);
    layout->addWidget (group_box);
    connect (button_group, SIGNAL (buttonClicked (int)), this, SLOT (update_KITCHENTIMER_DIGITAL_TIMER_MODE (int)));

    QCheckBox *show_debug_overlay_check_box = new QCheckBox ("Show debug overlay", this);
    show_debug_overlay_check_box->setChecked (KITCHENTIMER_SHOW_DEBUG_OVERLAY);
    connect (show_debug_overlay_check_box, SIGNAL (toggled (bool)), this, SLOT (update_KITCHENTIMER_SHOW_DEBUG_OVERLAY (bool)));
    layout->addWidget (show_debug_overlay_check_box);

    QCheckBox *use_slide_sound_check_box = new QCheckBox ("Use slide sound", this);
    use_slide_sound_check_box->setChecked (KITCHENTIMER_USE_ANALOG_TIMER_SLIDE_SOUND);
    connect (use_slide_sound_check_box, SIGNAL (toggled (bool)), this, SLOT (update_KITCHENTIMER_USE_ANALOG_TIMER_SLIDE_SOUND (bool)));
    layout->addWidget (use_slide_sound_check_box);

    QCheckBox *use_vibration_check_box = new QCheckBox ("Use vibration", this);
    use_vibration_check_box->setChecked (KITCHENTIMER_USE_VIBRATION);
    connect (use_vibration_check_box, SIGNAL (toggled (bool)), this, SLOT (update_KITCHENTIMER_USE_VIBRATION (bool)));
    layout->addWidget (use_vibration_check_box);

    hlayout = new QHBoxLayout ();
    hlayout->addStretch (1);
    QPushButton *close_button = new QPushButton ("Close", scroll_widget);
    connect (close_button, SIGNAL (clicked ()), this, SIGNAL (leavePage ()));
    hlayout->addWidget (close_button);
    hlayout->addStretch (1);
    layout->addLayout (hlayout);

    layout->addStretch (1);
}
PageSettings::~PageSettings ()
{
}

#define ADD_INT_CHANGE_HANDLER(var)			\
    void PageSettings::update_ ## var (int new_value)	\
    {							\
	var = new_value;				\
	(slider_ ## var)->setValue (new_value);		\
	(spin_box_ ## var)->setValue (new_value);	\
    }
ADD_INT_CHANGE_HANDLER (KITCHENTIMER_SLIDE_VIBRATION_ON_TIMEOUT)
ADD_INT_CHANGE_HANDLER (KITCHENTIMER_SLIDE_VIBRATION_SILENCE_DURATION)
ADD_INT_CHANGE_HANDLER (KITCHENTIMER_SLIDE_VIBRATION_VIBRATION_DURATION)
ADD_INT_CHANGE_HANDLER (KITCHENTIMER_ANALOG_TIMER_MAX_FOLLOW_EXTENT)
ADD_INT_CHANGE_HANDLER (KITCHENTIMER_EDIT_TRANSITION_TIMEOUT_MS)
ADD_INT_CHANGE_HANDLER (KITCHENTIMER_EDIT_HOLD_TIMEOUT_MS)
ADD_INT_CHANGE_HANDLER (KITCHENTIMER_INITIAL_EDIT_TRANSITION_TIMEOUT_MS)
ADD_INT_CHANGE_HANDLER (KITCHENTIMER_INITIAL_EDIT_HOLD_TIMEOUT_MS)
ADD_INT_CHANGE_HANDLER (KITCHENTIMER_LEAVE_EDIT_MODE_TIMEOUT_MS)
void PageSettings::update_KITCHENTIMER_ANALOG_TIMER_MODE (int new_mode)
{
    KITCHENTIMER_ANALOG_TIMER_MODE = new_mode;
}
void PageSettings::update_KITCHENTIMER_DIGITAL_TIMER_MODE (int new_mode)
{
    KITCHENTIMER_DIGITAL_TIMER_MODE = new_mode;
}
void PageSettings::update_KITCHENTIMER_SHOW_DEBUG_OVERLAY (bool new_value)
{
    KITCHENTIMER_SHOW_DEBUG_OVERLAY = new_value;
}
void PageSettings::update_KITCHENTIMER_USE_ANALOG_TIMER_SLIDE_SOUND (bool new_value)
{
    KITCHENTIMER_USE_ANALOG_TIMER_SLIDE_SOUND = new_value;
}
void PageSettings::update_KITCHENTIMER_USE_VIBRATION (bool new_value)
{
    KITCHENTIMER_USE_VIBRATION = new_value;
}
