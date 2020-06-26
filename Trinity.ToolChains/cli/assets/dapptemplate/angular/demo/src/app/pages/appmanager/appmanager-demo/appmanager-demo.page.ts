import { Component, OnInit } from '@angular/core';
import { NavParams } from '@ionic/angular';
import { TitlebarService } from 'src/app/services/titlebar.service';

declare let titleBarManager: TitleBarPlugin.TitleBarManager;

@Component({
  selector: 'app-appmanager-demo',
  templateUrl: './appmanager-demo.page.html',
  styleUrls: ['./appmanager-demo.page.scss'],
})
export class AppmanagerDemoPage implements OnInit {

  public manager;

  constructor(
    private navParams: NavParams,
    private titlebarService: TitlebarService
  ) { }

  ngOnInit() {
    this.manager = this.navParams.get('manager');
    console.log('App Manager Example', this.manager);
  }

  ionViewWillEnter() {
    titleBarManager.setTitle('App Manager Demo');
    this.titlebarService.setTitleBarBackKeyShown(true);
  }

  ionViewWillLeave() {
    this.titlebarService.setTitleBarBackKeyShown(false);
  }
}
