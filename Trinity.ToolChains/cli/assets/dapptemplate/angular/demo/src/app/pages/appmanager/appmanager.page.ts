import { Component, OnInit } from '@angular/core';
import { AppmanagerService } from 'src/app/services/appmanager.service';
import { TitlebarService } from 'src/app/services/titlebar.service';

declare let titleBarManager: TitleBarPlugin.TitleBarManager;

@Component({
  selector: 'app-appmanager',
  templateUrl: './appmanager.page.html',
  styleUrls: ['./appmanager.page.scss'],
})
export class AppmanagerPage implements OnInit {

  constructor(
    public appManagerService: AppmanagerService,
    private titlebarService: TitlebarService
  ) { }

  ngOnInit() {
  }

  ionViewWillEnter() {
    titleBarManager.setTitle("App Manager Guide");
    this.titlebarService.setTitleBarBackKeyShown(true);
  }

  ionViewWillLeave() {
    this.titlebarService.setTitleBarBackKeyShown(false);
  }
}
