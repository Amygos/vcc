/*
 * Sony VAIO Control Center for Linux
 *
 * Copyright (C) 2011 Tu Vuong <vanhtu1987@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "main_window.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon( QIcon::fromTheme( "computer-laptop" ) );

    QPalette plt = palette();
        plt.setColor( QPalette::WindowText , QColor(255,255,255) );

    QFont fnt = font();
        fnt.setBold( true );
        fnt.setItalic( true );
        fnt.setPixelSize( 13 );

    icon = new SPictureWidget();
        icon->setFixedSize( 48 , 48 );

    item_label = new QLabel();
        item_label->setPalette( plt );
        item_label->setFont( fnt );

    picture = new SPictureWidget();
        picture->drawFile( ":/pics/vcc/vaio.png" );
        picture->setFixedHeight( 90 );

    layout = new QHBoxLayout( picture );
        layout->addWidget( icon );
        layout->addWidget( item_label );
        layout->addStretch();

    ui->verticalLayout->insertWidget( 0 , picture );

    ui->listWidget->setIconSize( QSize(22,22) );
    ui->listWidget->item(0)->setIcon( QIcon::fromTheme( "battery" ) );
    ui->listWidget->item(1)->setIcon( QIcon::fromTheme( "input-keyboard" ) );
    ui->listWidget->item(2)->setIcon( QIcon::fromTheme( "input-mouse" ) );
    ui->listWidget->item(3)->setIcon( QIcon::fromTheme( "video-projector" ) );
    ui->listWidget->item(4)->setIcon( QIcon::fromTheme( "system-suspend-hibernate" ) );
    ui->listWidget->item(5)->setIcon( QIcon::fromTheme( "drive-optical" ) );
    ui->listWidget->item(6)->setIcon( QIcon::fromTheme( "cpu" ) );

    setup_ui();

    connect( ui->listWidget , SIGNAL(currentRowChanged(int)) , SLOT(list_index_changed(int)) );
    ui->listWidget->setCurrentRow( 0 );
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setup_ui() {
    // Battery Charge
    if (check_file(SONY_BATTERY_HIGHSPEED_CHRG)) {
        ui->chk_battery_fast_charge->setChecked(read_int_from_file(SONY_BATTERY_HIGHSPEED_CHRG));

        connect(ui->chk_battery_fast_charge, SIGNAL(stateChanged(int)), this, SLOT(chk_battery_fast_charge_changed(int)));
    }
    else ui->chk_battery_fast_charge->setEnabled(false);
    if (check_file(SONY_BATTERY_CHARGE_LIMITER)) {
        ui->btngrp_battery_protection->setId(ui->rad_battery_max_protection, 50);
        ui->btngrp_battery_protection->setId(ui->rad_battery_medium_protection, 80);
        ui->btngrp_battery_protection->setId(ui->rad_battery_no_protection, 0);
        switch(read_int_from_file(SONY_BATTERY_CHARGE_LIMITER)) {
            case 50:
                ui->rad_battery_max_protection->setChecked(true);
                break;
            case 80:
                ui->rad_battery_medium_protection->setChecked(true);
                break;
            case 100:
            case 0:
                ui->rad_battery_no_protection->setChecked(true);
                break;
        }

        connect(ui->btngrp_battery_protection, SIGNAL(buttonClicked(int)), this, SLOT(btngrp_battery_protection_button_clicked(int)));
    }
    else ui->frm_battery_protection->setEnabled(false);

    // Keyboard Backlight
    if (check_file(SONY_KBD_BL)) {
        int const kbd_backlight = read_int_from_file(SONY_KBD_BL);
        ui->chk_enable_kbd_bl->setChecked(kbd_backlight);
        ui->hslider_kbd_timeout->setEnabled(kbd_backlight);
        int const kbd_backlight_timeout = read_int_from_file(SONY_KBD_BL_TIMEOUT);
        ui->hslider_kbd_timeout->setValue(kbd_backlight_timeout);
        hslider_kbd_timeout_value_changed(kbd_backlight_timeout);

        connect(ui->chk_enable_kbd_bl, SIGNAL(stateChanged(int)), this, SLOT(chk_enable_kbd_bl_state_changed(int)));
        connect(ui->hslider_kbd_timeout, SIGNAL(valueChanged(int)), this, SLOT(hslider_kbd_timeout_value_changed(int)));
    }
    else ui->frm_kbd_bl->setEnabled(false);

    // Touchpad
    if (check_file(SONY_TOUCHPAD)) {
        ui->chk_enable_touchpad->setChecked(read_int_from_file(SONY_TOUCHPAD));

        connect(ui->chk_enable_touchpad, SIGNAL(stateChanged(int)), this, SLOT(chk_enable_touchpad_state_changed(int)));
    }
    else ui->chk_enable_touchpad->setEnabled(false);

    // ALS
    if (check_file(SONY_ALS_POWER)) {
        int const als_power = read_int_from_file(SONY_ALS_POWER);
        ui->chk_enable_als_power->setChecked(als_power);

        connect(ui->chk_enable_als_power, SIGNAL(stateChanged(int)), this, SLOT(chk_enable_als_power_state_changed(int)));
	if(check_file(SONY_ALS_LUX))
        	connect(&_als_timer, SIGNAL(timeout()), this, SLOT(update_als_lux_data()));
	if(check_file(SONY_ALS_KELVIN))
        	connect(&_als_timer, SIGNAL(timeout()), this, SLOT(update_als_kelvin_data()));
        _als_timer.start(500);
    }
    else ui->frm_als->setEnabled(false);

    // Lid
    if (check_file(SONY_LID_RESUME_S3)) {
        int const status = read_int_from_file(SONY_LID_RESUME_S3);

	ui->chk_lid_s3->setEnabled(true);
	ui->chk_lid_s3->setChecked(status);

        connect(ui->chk_lid_s3, SIGNAL(stateChanged(int)), this, SLOT(chk_lid_s3_changed(int)));
    }
    if (check_file(SONY_LID_RESUME_S4)) {
        int const status = read_int_from_file(SONY_LID_RESUME_S4);

	ui->chk_lid_s4->setEnabled(true);
	ui->chk_lid_s4->setChecked(status);

        connect(ui->chk_lid_s4, SIGNAL(stateChanged(int)), this, SLOT(chk_lid_s4_changed(int)));
    }
    if (check_file(SONY_LID_RESUME_S5)) {
        int const status = read_int_from_file(SONY_LID_RESUME_S5);

	ui->chk_lid_s5->setEnabled(true);
	ui->chk_lid_s5->setChecked(status);

        connect(ui->chk_lid_s5, SIGNAL(stateChanged(int)), this, SLOT(chk_lid_s5_changed(int)));
    }

    // Optical device
    if (check_file(SONY_OPTICAL_DEV)) {
        ui->chk_enable_optdev->setChecked(read_int_from_file(SONY_OPTICAL_DEV));

        connect(ui->chk_enable_optdev, SIGNAL(stateChanged(int)), this, SLOT(chk_enable_optdev_state_changed(int)));
    }
    else ui->chk_enable_optdev->setEnabled(false);

    // Thermal Control
    if (check_file(SONY_THERMAL_NUM)) {
	std::string profiles;
        char const *profile;
	char buf[64];

	profiles = read_str_from_file(SONY_THERMAL_NUM, buf, sizeof(buf)/sizeof(buf[0]));

	if (profiles.find("silent", 0) != std::string::npos)
            ui->btngrp_thermal->setId(ui->rad_thermal_silent, 2);
	else
            ui->rad_thermal_silent->setEnabled(0);

        ui->btngrp_thermal->setId(ui->rad_thermal_performance, 1);
        ui->btngrp_thermal->setId(ui->rad_thermal_balanced, 0);

	profile = read_str_from_file(SONY_THERMAL, buf, sizeof(buf)/sizeof(buf[0]));
	if (!strncmp(profile, "silent", strlen("silent")))
                ui->rad_thermal_silent->setChecked(true);
	else if (!strncmp(profile, "performance", strlen("silent")))
                ui->rad_thermal_performance->setChecked(true);
	else if (!strncmp(profile, "balanced", strlen("silent")))
                ui->rad_thermal_balanced->setChecked(true);
        
        connect(ui->btngrp_thermal, SIGNAL(buttonClicked(int)), this, SLOT(btngrp_thermal_button_clicked(int)));
    }
    else ui->frm_thermal->setEnabled(false);

    if (check_file(SONY_FAN_SPEED)) {
	    ui->lbl_fan_speed_val->setEnabled(1);
	    ui->lbl_fan_speed->setEnabled(1);
	    connect(&_fan_speed_timer, SIGNAL(timeout()), this, SLOT(update_fan_speed_data()));
	    _fan_speed_timer.start(500);
    }
    if (check_file(SONY_FAN_FORCED)) {
	    ui->lbl_fan_forced->setEnabled(1);
	    ui->btn_fan_forced->setEnabled(1);
	    connect(&_fan_forced_timer, SIGNAL(timeout()), this, SLOT(update_fan_forced_status()));
	    connect(ui->btn_fan_forced, SIGNAL(clicked()), this, SLOT(btn_fan_forced_clicked()));

	    _fan_forced_timer.start(500);
    }



}

void MainWindow::chk_battery_fast_charge_changed(int state) {
    write_int_to_file(SONY_BATTERY_HIGHSPEED_CHRG, state == 2 ? 1 : 0);
}
void MainWindow::btngrp_battery_protection_button_clicked(int id) {
    write_int_to_file(SONY_BATTERY_CHARGE_LIMITER, id);
}

void MainWindow::chk_enable_kbd_bl_state_changed(int state) {
    ui->hslider_kbd_timeout->setEnabled(state);
    write_int_to_file(SONY_KBD_BL, state == 2 ? 1 : 0);
}
void MainWindow::hslider_kbd_timeout_value_changed(int val) {
    char const* txt = NULL;
    switch (val) {
        case 0:
            txt = "10 seconds";
            break;
        case 1:
            txt = "30 seconds";
            break;
        case 2:
            txt = "60 seconds";
            break;
        case 3:
            txt = "Always on";
            break;
    }
    ui->lbl_kbd_timeout_val->setText(txt);
    write_int_to_file(SONY_KBD_BL_TIMEOUT, val);
}

void MainWindow::chk_enable_touchpad_state_changed(int state) {
    write_int_to_file(SONY_TOUCHPAD, state == 2 ? 1 : 0);
}

void MainWindow::chk_enable_als_power_state_changed(int state) {
    write_int_to_file(SONY_ALS_POWER, state == 2 ? 1 : 0);
}

void MainWindow::update_als_lux_data() {
    char buf[64];
    ui->lbl_als_lux_val->setText(read_str_from_file(SONY_ALS_LUX, buf, sizeof(buf)/sizeof(buf[0])));
}

void MainWindow::update_als_kelvin_data() {
    char buf[64];
    ui->lbl_als_kelvin_val->setText(read_str_from_file(SONY_ALS_KELVIN, buf, sizeof(buf)/sizeof(buf[0])));
}

void MainWindow::update_fan_speed_data() {
    char buf[64];
    int i;

    read_str_from_file(SONY_FAN_SPEED, buf, sizeof(buf)/sizeof(buf[0]));

    //Remove newline
    for(i = 0 ; (buf[i] != '\n') && (i < 64); ++i);
    buf[i] = '\0';

    ui->lbl_fan_speed_val->setText(buf);
}

void MainWindow::update_fan_forced_status() {
	int status = read_int_from_file(SONY_FAN_FORCED);

	if(status)
		ui->btn_fan_forced->setText("Stop");
	else
		ui->btn_fan_forced->setText("Start");
}

void MainWindow::btn_fan_forced_clicked() {
	int status = read_int_from_file(SONY_FAN_FORCED);

	write_int_to_file(SONY_FAN_FORCED, !status);
}

void MainWindow::chk_lid_s3_changed(int state) {
    write_int_to_file(SONY_LID_RESUME_S3, state == 2 ? 1 : 0);
}

void MainWindow::chk_lid_s4_changed(int state) {
    write_int_to_file(SONY_LID_RESUME_S4, state == 2 ? 1 : 0);
}

void MainWindow::chk_lid_s5_changed(int state) {
	printf("%d\n",state);
    write_int_to_file(SONY_LID_RESUME_S5, state == 2 ? 1 : 0);
}

void MainWindow::chk_enable_optdev_state_changed(int state) {
    write_int_to_file(SONY_OPTICAL_DEV, state == 2 ? 1 : 0);
}

void MainWindow::btngrp_thermal_button_clicked(int id) {

    switch(id) {
	case 2:
		write_str_to_file(SONY_THERMAL, "silent");
		break;
	case 1:
		write_str_to_file(SONY_THERMAL, "performance");
		break;
	case 0:
		write_str_to_file(SONY_THERMAL, "balanced");
                break;
    }
}

void MainWindow::list_index_changed( int index )
{
    QListWidgetItem *item = ui->listWidget->item(index);

    ui->stackedWidget->setCurrentIndex( index );

    item_label->setText( item->text() );
    icon->drawIcon( item->icon() , QSize(48,48) );
}
