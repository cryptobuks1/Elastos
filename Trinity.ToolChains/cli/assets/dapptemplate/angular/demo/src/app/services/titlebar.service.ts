import { Injectable } from '@angular/core';
import { ModalController, NavController } from '@ionic/angular';
import { TitlebarDemoPage } from '../pages/titlebar/titlebar-demo/titlebar-demo.page';
import { Router } from '@angular/router';

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
      message: 'The title of your app can be changed whenever and wherever. This can be useful to display the title of your page, the occuring action or anything you desire.',
      message2: null,
      example: 'assets/titlebar/title.png'
    },
    {
      type: 'Color',
      title: 'Customize Color',
      method: 'setBackgroundColor/setForegroundMode',
      message: 'At any time, you can customize the background and title color of your titlebar to match the theme of your app.',
      message2: null,
      example: 'assets/titlebar/color.png'
    },
    {
      type: 'Icons',
      title: 'Customize Icons',
      method: 'setIcon',
      message: 'Setting an icon is a special way add buttons to your titlebar. To set an icon, make sure to have a listener ready once your app is rendered. Then you can add your icon to a page where you want it revealed using the page\'s life cycle.',
      message3: 'Make sure to declare the position of the icon, a key (special name to handle the icon) and the icon itself which you can set the path for or simply use one of elastOS built-in icons.',
      example2: 'assets/titlebar/listener.png',
      example3: 'assets/titlebar/setIcon.png',
    },
    {
      type: 'Navigation',
      title: 'Customize Navigation',
      method: 'setIcon',
      message: 'Setting your app\'s navigation is similar to setting an icon to your titlebar.',
      message2: 'To set naviation in your titlebar, make sure to have a listener ready once your app is rendered. Then you can add (or) remove the elastOS back-icon in a page using a life cycle.',
      message3: 'Make sure to declare the position of the icon (ideally in the outer left of the titlebar), a key (special name to handle the icon) and the icon itself.',
      example: 'assets/titlebar/back-listener.png',
      example2: 'assets/titlebar/setBackIcon.png',
      example3: 'assets/titlebar/setBackIcon2.png',
    },
    {
      type: 'Items',
      title: 'Customize Items',
      method: 'setupMenuItems',
      message: 'One of the best ways to customize your titlebar is to add menu items in it. Declaring this will add an options key to the right corner of your titlebar that displays a list of menu items you provided.',
      message2: 'This will give you the ability to add a list of actions with a custom icon in your title bar which can trigger a callback you have set for it.',
      example: 'assets/titlebar/items.png'
    }
  ];

  async openManager(manager: any) {
    const modal = await this.modalCtrl.create({
      component: TitlebarDemoPage,
      componentProps: {
        manager: manager
      },
    });

    modal.present();
  }

  constructor(private modalCtrl: ModalController, private router: Router) { }

  init() {
    this.setColor();

    titleBarManager.addOnItemClickedListener((menuIcon) => {
      if (menuIcon.key === "back") {
        this.modalCtrl.dismiss();
        this.router.navigate(['/home']);
      }
    });
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
      key: "add",
      iconPath:  TitleBarPlugin.BuiltInIcon.ADD
    });
  }

  /******************** Set Menu Items ********************/
  setMenuItems() {
 /*    titleBarManager.setupMenuItems(
      [
        {
          key: "", // Add uniqute item key
          iconPath: "", // Add path to item icon
          title: "" // Add item title
        }
      ],
    ); */
  }

}
