import { Injectable } from '@angular/core';
import { ModalController, NavController, AlertController } from '@ionic/angular';
import { TitlebarDemoPage } from '../pages/titlebar/titlebar-demo/titlebar-demo.page';
import { Router } from '@angular/router';

declare let appManager: AppManagerPlugin.AppManager;
declare let titleBarManager: TitleBarPlugin.TitleBarManager;

@Injectable({
  providedIn: 'root'
})
export class TitlebarService {

  public managers = [
    {
      type: 'Title',
      title: 'Customize Title',
      method: 'setTitle',
      message: 'The title of your app can be changed at any time. This can be useful to display the title of your page, the occuring action or anything you desire.',
      message2: null,
      example: 'assets/titlebar/title.png'
    },
    {
      type: 'Color',
      title: 'Customize Color',
      method: 'setBackgroundColor/setForegroundMode',
      message: 'At any time, you can customize the background and title color of your Titlebar to match the theme of your app.',
      message2: null,
      example: 'assets/titlebar/color.png'
    },
    {
      type: 'Icons',
      title: 'Customize Icons',
      method: 'setIcon',
      message: 'Setting an icon is a special way add buttons to your titlebar.',
      message2: 'To set an icon, make sure to have a listener ready once your app or page is rendered. Then you can add your icon to a page where you want it revealed using the page\'s life cycle.',
      message3: 'Make sure to declare the position of the icon, a key (special name to handle the icon) and the icon itself which you can set the path for or simply use one of elastOS built-in icons.',
      example: 'assets/titlebar/listener.png',
      example2: 'assets/titlebar/setIcon.png',
    },
    {
      type: 'Navigation',
      title: 'Customize Navigation',
      method: 'setIcon',
      message: 'Setting your app\'s navigation is similar to setting an icon to your Titlebar.',
      message2: 'To set naviation in your Titlebar, make sure to have a listener ready once your app or page is rendered. Then you can add (or) remove the elastOS back-icon in a page using a life cycle.',
      message3: 'Make sure to declare the position of the icon (ideally in the outer left of the titlebar), a key (special name to handle the icon) and the icon itself.',
      example: 'assets/titlebar/back-listener.png',
      example2: 'assets/titlebar/setBackIcon.png',
      example3: 'assets/titlebar/setBackIcon2.png',
    },
    {
      type: 'Items',
      title: 'Customize Items',
      method: 'setupMenuItems',
      message: 'Similar to setting icons to your Titlebar, setting up menu items will present an options key to display a list of menu items you provided. This is an alternative option for adding buttons to your Titlebar without cluttering it.',
      message2: 'Like always, make sure to have a Titlebar listener prepared to handle your menu items using the special key you provided. As well as a path to your icon and a title to visually present this action.',
      message3: 'Depending on the state of your app, there are cases you may want to remove your Titlebar items. To do this, make sure to store your menu item as a variable. You can then add it under your listener or remove it under a page\'s life cycle.',
      example: 'assets/titlebar/items.png',
      example2: 'assets/titlebar/removeItems.png',
    }
  ];

  public onItemClickedListener: any;

  async openManager(manager: any) {
    const modal = await this.modalCtrl.create({
      component: TitlebarDemoPage,
      componentProps: {
        manager: manager
      },
    });

    modal.present();
  }

  constructor(
    private modalCtrl: ModalController, 
    private navCtrl: NavController,
    private alertCtrl: AlertController
  ) { }

  init() {
    this.setColor();

    // Listener for titlebar icons
    titleBarManager.addOnItemClickedListener((menuIcon) => {
      if (menuIcon.key === "back") {
        this.modalCtrl.dismiss();
        this.navCtrl.navigateBack(['/home']);

        titleBarManager.setTitle("Demo Template");
        titleBarManager.setBackgroundColor("#181d20");
        titleBarManager.setForegroundMode(TitleBarPlugin.TitleBarForegroundMode.LIGHT);
        titleBarManager.setIcon(TitleBarPlugin.TitleBarIconSlot.INNER_RIGHT, null);
        titleBarManager.removeOnItemClickedListener(this.onItemClickedListener);
      }
      this.onTitlebarItemClicked(menuIcon);
    });
  }

  onTitlebarItemClicked(icon: TitleBarPlugin.TitleBarIcon) {
    switch (icon.key) {
      case 'add':
        console.log('Add icon clicked');
        this.alertTitlebarItemClicked(
          'You clicked a Titlebar icon!', 
          'This action won\'t do anything but demonstrate the Titlebar icon.'
        );
        break;
    }
  }

  /******************** Set Title ********************/
  setTitle() {
    // Set title
    titleBarManager.setTitle("Titlebar Title");
  }

  /******************** Set Color ********************/
  setColor() {
    // Set background color
    titleBarManager.setBackgroundColor("#181d20");
    // Set title color
    titleBarManager.setForegroundMode(TitleBarPlugin.TitleBarForegroundMode.LIGHT);
  }

  /******************** Set Navigation ********************/
  setTitleBarBackKeyShown(show: boolean) {
    if (show) {
      titleBarManager.setIcon(TitleBarPlugin.TitleBarIconSlot.INNER_LEFT, {
        key: "back",
        iconPath: TitleBarPlugin.BuiltInIcon.BACK
      });
    } else {
      titleBarManager.setIcon(TitleBarPlugin.TitleBarIconSlot.INNER_LEFT, null);
    }
  }

  /******************** Set and Handle Icons ********************/
  setIcon() {
    titleBarManager.setIcon(TitleBarPlugin.TitleBarIconSlot.OUTER_RIGHT, {
      // Special key to handle item
      key: "add",
      // Add a path to your own icon or use one of elastOS built-in icons
      iconPath:  TitleBarPlugin.BuiltInIcon.ADD
    });
  }

  /******************** Setup Menu Items ********************/
  setMenuItems() {
    titleBarManager.setupMenuItems(
      [
        {
          // Special key to handle item
          key: "help",
          // Add path to your icon
          iconPath: TitleBarPlugin.BuiltInIcon.HELP,
          // Add title of your item
          title: "Test Menu Item"
        }
      ],
    );
  }

  /******************* Simple alerts to demonstrate titlebar icons and items *******************/
  async alertTitlebarItemClicked(header: string, message: string) {
    const alert = await this.alertCtrl.create({
      mode: "ios",
      header: header,
      message: message,
      buttons: [
        {
          text: "Cool!",
          role: "cancel",
          cssClass: "secondary",
          handler: () => {
          }
        },
      ]
    });
    alert.present();
  }

}
