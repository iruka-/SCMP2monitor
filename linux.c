#ifdef	_LINUX_

void gr_init(int width,int height,int bpp,int color){}	//初期化.
void gr_exit(int rc){}									//終了.
void gr_cls(int color){}									//画面クリア.
void gr_pset(int x,int y,int color){}					//ドット打ち.
int *gr_point(int x,int y){return 0;}
void gr_line(int x0,int y0,int x1,int y1,int color){}	//線引き.
void gr_hline(int x0,int y0,int x1,int y1,int color){}	//線引き.
void gr_vline(int x0,int y0,int x1,int y1,int color){}	//線引き.
void gr_box(int x0,int y0,int width,int height,int color){}		//箱(枠のみ).
void gr_boxfill(int x0,int y0,int width,int height,int color){}	//箱(内部塗りつぶし).
void gr_circle( int cx,int cy,int r,int c){}
void gr_circle_arc( int cx,int cy,int rx,int ry,int c,int begin,int end){}
void gr_puts(int x,int y,char *s,int color,int bkcolor,int size){}	// 文字列描画.
int gr_flip(int flag){return 0;}									//描画完了処理.
void gr_close(void){}									//窓close
int gr_break(void){return 0;}										//終了チェック.

void Sleep(int msec){}

#endif
