#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#ifdef  _WIN32
  #include <conio.h>
  static void raw_init(void)  {}
  static void raw_restore(void) {}
  static int  get_key(void)   { return _getch(); }   // 立即返回、无回显
#else
  #include <termios.h>
  #include <unistd.h>

  static struct termios g_old;

  static void raw_init(void) {
      tcgetattr(STDIN_FILENO, &g_old);
      struct termios t = g_old;
      t.c_lflag &= ~(ICANON | ECHO); // 关行缓冲 + 回显
      t.c_cc[VMIN]  = 1;             // 至少读1字节才返回
      t.c_cc[VTIME] = 0;             // 不超时
      tcsetattr(STDIN_FILENO, TCSANOW, &t);
  }

  static void raw_restore(void) {
      tcsetattr(STDIN_FILENO, TCSANOW, &g_old);
  }

  static int get_key(void) {
      return getchar(); // raw 模式下立即得到一个字节
  }
#endif
void act_on(){printf("lamp is on\n");}
void act_off(){printf("lamp is off\n");}
void act_dim(){printf("lamp is dim\n");}
typedef enum {E_SPACE,E_QUIT,E_ENTER,E_NONE,E_N} event_t;
typedef enum {OFF,DIM,ON,S_N} State;
typedef void(*action_t)(void);
event_t read_event(){
	char c=getchar();
	event_t ev=E_NONE;
	if(c==' '){ev=E_SPACE;return ev;}
	if(c=='q'||c=='Q')ev=E_QUIT;
	if(c=='\n'||c=='\r')ev=E_ENTER;
	return ev;
}
void show_state(State s){
	char *name=(s==OFF)?"OFF":(s==DIM)?"DIM":"ON";
	printf("[state]:%s\n",name);
}
void handle_signal(int sig){
	raw_restore();
	_exit(1);
}
int main(){
	raw_init();
    	atexit(raw_restore);   // 程序结束自动恢复

	// 捕捉常见的异常信号
    	signal(SIGINT, handle_signal);   // Ctrl+C
    	signal(SIGTERM, handle_signal);  // kill
    	signal(SIGSEGV, handle_signal);  // 段错误);

	State s=OFF;
	action_t actions[S_N]={act_off,act_dim,act_on};
	State next[S_N]={DIM,ON,OFF};
	event_t ev=E_NONE;
	printf("press space TOGGLE,enter show state,q quit:\n");
	while(1){
		ev=read_event();
		if(ev==E_QUIT)break;
		if(ev==E_NONE)continue;
		if(ev==E_SPACE){
			s=next[s];
			actions[s]();
		}else if(ev==E_ENTER)show_state(s);
		fflush(stdout);
	}
}
