import { Component, ViewChild } from '@angular/core';
import { Platform, IonRouterOutlet, NavController } from '@ionic/angular';
import { Router } from '@angular/router';
import { DAppService } from './services/dapp.service';
import { TitlebarService } from './services/titlebar.service';

@Component({
  selector: 'my-app',
  templateUrl: 'app.html'
})
export class MyApp {
  @ViewChild(IonRouterOutlet, {static: true}) routerOutlet: IonRouterOutlet;

  constructor(
    private platform: Platform, 
    private navController: NavController, 
    private dappService: DAppService,
    private titlebarService: TitlebarService
  ) {
    this.initializeApp();
  }

  initializeApp() {
    this.platform.ready().then(async () => {
      // Make sure to wait for platform to be ready before navigating to the first screen. Otherwise
      // plugins such as AppManager or TitleBarManager are not ready.

      this.dappService.init();
      this.titlebarService.init();
      this.navController.navigateRoot("/home");
    });
  }

  /**
   * Listen to back key events. If the default router can go back, just go back.
   * Otherwise, exit the application.
   */
  setupBackKeyNavigation() {
    this.platform.backButton.subscribeWithPriority(0, () => {
      console.log("BACK")
      if (this.routerOutlet && this.routerOutlet.canGoBack()) {
        this.routerOutlet.pop();
      } else {
        navigator['app'].exitApp();
      }
    });
  }
}
