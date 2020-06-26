import { Component, OnInit } from '@angular/core';
import { NavParams } from '@ionic/angular';
import { TitlebarService } from 'src/app/services/titlebar.service';

declare let titleBarManager: TitleBarPlugin.TitleBarManager;

@Component({
  selector: 'app-titlebar-demo',
  templateUrl: './titlebar-demo.page.html',
  styleUrls: ['./titlebar-demo.page.scss'],
})
export class TitlebarDemoPage implements OnInit {

  public manager;

  constructor(
    private navParams: NavParams,
    private titlebarService: TitlebarService
  ) { }

  ngOnInit() {
    this.manager = this.navParams.get('manager');
    console.log('Titlebar Example', this.manager);
  }

  ionViewWillEnter() {
    titleBarManager.setTitle('Titlebar Demo');
    this.titlebarService.setTitleBarBackKeyShown(true);
  }

  ionViewWillLeave() {
    titleBarManager.setTitle("Demo Template");
    this.titlebarService.setTitleBarBackKeyShown(false);
  }

}