#ifdef	_LINUX_

void gr_init(int width,int height,int bpp,int color){}	//�����.
void gr_exit(int rc){}									//��λ.
void gr_cls(int color){}									//���̥��ꥢ.
void gr_pset(int x,int y,int color){}					//�ɥå��Ǥ�.
int *gr_point(int x,int y){return 0;}
void gr_line(int x0,int y0,int x1,int y1,int color){}	//������.
void gr_hline(int x0,int y0,int x1,int y1,int color){}	//������.
void gr_vline(int x0,int y0,int x1,int y1,int color){}	//������.
void gr_box(int x0,int y0,int width,int height,int color){}		//Ȣ(�ȤΤ�).
void gr_boxfill(int x0,int y0,int width,int height,int color){}	//Ȣ(�����ɤ�Ĥ֤�).
void gr_circle( int cx,int cy,int r,int c){}
void gr_circle_arc( int cx,int cy,int rx,int ry,int c,int begin,int end){}
void gr_puts(int x,int y,char *s,int color,int bkcolor,int size){}	// ʸ��������.
int gr_flip(int flag){return 0;}									//���贰λ����.
void gr_close(void){}									//��close
int gr_break(void){return 0;}										//��λ�����å�.

void Sleep(int msec){}

#endif
