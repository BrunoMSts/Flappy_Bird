#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <random>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#define FPS 70.0
#define W_HEIGHT 600
#define W_WIDTH 280

ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;
ALLEGRO_FONT *fonte = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_SAMPLE *som_pulo = NULL, *som_ponto = NULL, *som_morreu = NULL, *som_bate = NULL, *som_inicio = NULL;
ALLEGRO_BITMAP *base = NULL, *base1 = NULL, *message = NULL,
*passaro_mid = NULL,
*passaro_down = NULL, *passaro_up = NULL,
*game_over = NULL;
std::vector<ALLEGRO_BITMAP *> scores, fundo, cano, cano_invertido;
ALLEGRO_BITMAP *passaros[3][3];

void error_msg(const char *text){
    al_show_native_message_box(janela,"ERRO",
                               "Ocorreu o seguinte erro e o programa sera finalizado:",
                               text,NULL,ALLEGRO_MESSAGEBOX_ERROR);
}

int inicializar(){
   if(!al_init()){
      error_msg("Falha ao inicializar Allegro");
      return 0;
   }

   al_init_font_addon();
   if(!al_init_ttf_addon()){
      error_msg("Falha ao inicializar allegro_ttf");
      return 0;
   }

   if(!al_init_image_addon()){
      error_msg("Falha ao inicializar allegro_image");
      return 0;
   }

   if(!al_install_keyboard()){
      error_msg("Falha ao inicializar teclado");
      return 0;
   }

   if(!al_install_mouse()){
      error_msg("Falha ao inicializar mouse");
      return 0;
   }

   janela = al_create_display(W_WIDTH, W_HEIGHT);
   if(!janela){
      error_msg("Falha ao criar janela");
      return 0;
   }

   al_set_window_title(janela, "Eventos do Teclado");

   fonte = al_load_font("arial.ttf", 72, 0);
   if(!fonte){
      error_msg("Falha ao carregar \"arial.ttf\"");
      al_destroy_display(janela);
      return 0;
   }

   fila_eventos = al_create_event_queue();
   if(!fila_eventos){
      error_msg("Falha ao criar fila de eventos");
      al_destroy_display(janela);
      return 0;
   }

   al_install_audio();
   al_init_acodec_addon();
   al_reserve_samples(5);

   timer = al_create_timer(1 / FPS);

   message = al_load_bitmap("assets\\sprites\\message.png");
   base = al_load_bitmap("assets\\sprites\\base.png");
   base1 = al_load_bitmap("assets\\sprites\\base.png");
   passaro_mid = al_load_bitmap("assets\\sprites\\bluebird-midflap.png");
   passaro_down = al_load_bitmap("assets\\sprites\\bluebird-downflap.png");
   passaro_up = al_load_bitmap("assets\\sprites\\bluebird-upflap.png");
   game_over = al_load_bitmap("assets\\sprites\\gameover.png");
   som_morreu = al_load_sample("assets\\audio\\die.wav");
   som_ponto = al_load_sample("assets\\audio\\point.wav");
   som_pulo = al_load_sample("assets\\audio\\wing.wav");
   som_inicio = al_load_sample("assets\\audio\\swoosh.wav");
   som_bate = al_load_sample("assets\\audio\\hit.wav");
   fundo.push_back(al_load_bitmap("assets\\sprites\\background-night.png"));
   fundo.push_back(al_load_bitmap("assets\\sprites\\background-day.png"));
   cano.push_back(al_load_bitmap("assets\\sprites\\pipe-green.png"));
   cano.push_back(al_load_bitmap("assets\\sprites\\pipe-red.png"));
   cano_invertido.push_back(al_load_bitmap("assets\\sprites\\pipe-green.png"));
   cano_invertido.push_back(al_load_bitmap("assets\\sprites\\pipe-red.png"));

   passaros[0][0] = al_load_bitmap("assets\\sprites\\bluebird-midflap.png");
   passaros[0][1] = al_load_bitmap("assets\\sprites\\bluebird-downflap.png");
   passaros[0][2] = al_load_bitmap("assets\\sprites\\bluebird-upflap.png");
   passaros[1][0] = al_load_bitmap("assets\\sprites\\redbird-midflap.png");
   passaros[1][1] = al_load_bitmap("assets\\sprites\\redbird-downflap.png");
   passaros[1][2] = al_load_bitmap("assets\\sprites\\redbird-upflap.png");
   passaros[2][0] = al_load_bitmap("assets\\sprites\\yellowbird-midflap.png");
   passaros[2][1] = al_load_bitmap("assets\\sprites\\yellowbird-downflap.png");
   passaros[2][2] = al_load_bitmap("assets\\sprites\\yellowbird-upflap.png");

   for(int i = 0; i <= 9; i++){
      std::string aux = "assets\\sprites\\" + std::to_string(i) + ".png";
      const char *sprite = aux.data();
      scores.push_back(al_load_bitmap(sprite));
   }

   al_register_event_source(fila_eventos, al_get_mouse_event_source());
   al_register_event_source(fila_eventos, al_get_keyboard_event_source());
   al_register_event_source(fila_eventos, al_get_display_event_source(janela));
   al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
   al_start_timer(timer);

   return 1;
}

int main(){
   srand(time(0));
   int random = 1, rand_bird = rand() % 3, rand_pipe = rand() % 2;
   int sair = 0, inicia = 0, respawn = 0, desenha = 1;
   int unidade = 0, dezena = 0, centena = 0, score = 0;
   float gravidade = 0.8, speed = 10, angulo = 0;
   int pos_x_base = 0, pos_y_base = 500;
   int pos_x_base1 = 336, pos_y_base1 = 500;
   int pos_x_fundo = 0, pos_y_fundo = 0;
   int pos_x_fundo1 = 288, pos_y_fundo1 = 0;
   int pos_x_passaro = 50, pos_y_passaro = 200;
   int pos_x_cano = 300;
   int pos_x_cano1 = 500;
   int pos_x_cano2 = 680;
   int pos_y_cano_invertido = rand() % 100 + 200; //int pos_y_cano_invertido = 300 , pos_y_cano = 280;
   int pos_y_cano_invertido1 = rand() % 100 + 180;
   int pos_y_cano_invertido2 = rand() % 100 + 150;
   int pos_x_unidade = 130, pos_x_dezena = 125, pos_x_centena = 120;
   int frame = 0;
   bool subir = false, morto = false;

   if(!inicializar()){
      return -1;
   }

   while(!sair){
      ALLEGRO_EVENT evento;
      al_wait_for_event(fila_eventos, &evento);

      if(evento.type == ALLEGRO_EVENT_TIMER){
         if(!inicia){
            pos_y_passaro = 150;
            pos_x_cano = 300;
            pos_x_cano1 = 500;
            pos_x_cano2 = 680;
            pos_x_base -= 3;
            pos_x_base1 -= 3;
            pos_x_fundo--;
            pos_x_fundo1--;
            if(pos_x_base + 340 <= 0){
               pos_x_base = 330;
            }
            if(pos_x_base1 + 340 <= 0){
               pos_x_base1 = 330;
            }
            if(pos_x_fundo + 288 <= 0){
               pos_x_fundo = 288;
            }
            if(pos_x_fundo1 + 288 <= 0){
               pos_x_fundo1 = 288;
            }
            if(frame > 5){
               frame = 0;
            }
            frame++;
            desenha = 1;
         } else {
            if(!morto){
               pos_x_base -= 3;
               pos_x_base1 -= 3;
               pos_x_fundo--;
               pos_x_fundo1--;
               pos_x_cano -= 3;
               pos_x_cano1 -= 3;
               pos_x_cano2 -= 3;
               if(speed >= 10){
                  speed = 10;
               }
               if(subir && speed >= 0.5){
                  speed = -speed-1.25;
                  angulo = -angulo-0.4;
                  subir = false;
               }
               if(pos_y_passaro <= 5){
                  pos_y_passaro = 5;
               }
               if(pos_x_base + 340 <= 0){
                  pos_x_base = 330;
               }
               if(pos_x_base1 + 340 <= 0){
                  pos_x_base1 = 330;
               }
               if(pos_x_fundo + 288 <= 0){
                  pos_x_fundo = 288;
               }
               if(pos_x_fundo1 + 288 <= 0){
                  pos_x_fundo1 = 288;
               }
               if(pos_x_cano + 280 <= 0){
                  pos_x_cano = 280;
                  pos_y_cano_invertido = rand() % 200 + 100;
               }
               if(pos_x_cano1 + 280 <= 0){
                  pos_x_cano1 = 280;
                  pos_y_cano_invertido1 = rand() % 180 + 100;
               }
               if(pos_x_cano2 + 280 <= 0){
                  pos_x_cano2 = 280;
                  pos_y_cano_invertido2 = rand() % 250 + 80;
               }
               if(frame > 5){
                  frame = 0;
               }
               if((pos_x_passaro >= pos_x_cano+10 && pos_x_passaro <= pos_x_cano+11) || (pos_x_passaro >= pos_x_cano1 && pos_x_passaro <= pos_x_cano1+1) || (pos_x_passaro >= pos_x_cano2 && pos_x_passaro <= pos_x_cano2+1)){
                  if(unidade >= 9){
                     dezena++;
                     if(dezena > 9){
                        centena++;
                     }
                  }
                  if(score == 9){
                     pos_x_unidade += 20;
                  }
                  if(score == 99){
                     pos_x_unidade += 22;
                     pos_x_dezena += 20;
                  }
                  unidade++;
                  score++;
                  centena = (centena <= 9) ? centena : 1;
                  unidade = (unidade <= 9) ? unidade : 0;
                  dezena = (dezena <= 9) ? dezena : 0;
                  al_play_sample(som_ponto, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               }
               if(pos_y_passaro >= pos_y_base-45 || ((pos_x_passaro >= pos_x_cano - 38 && pos_x_passaro <= pos_x_cano + 38)   && (pos_y_passaro >= pos_y_cano_invertido + 100 || pos_y_passaro <= pos_y_cano_invertido - 15)) ||
                                                    ((pos_x_passaro >= pos_x_cano1 - 38 && pos_x_passaro <= pos_x_cano1 + 38) && (pos_y_passaro >= pos_y_cano_invertido1 + 100 || pos_y_passaro <= pos_y_cano_invertido1 - 15)) ||
                                                    ((pos_x_passaro >= pos_x_cano2 - 38 && pos_x_passaro <= pos_x_cano2 + 38) && (pos_y_passaro >= pos_y_cano_invertido2 + 100 || pos_y_passaro <= pos_y_cano_invertido2 - 15))){
                  al_play_sample(som_bate, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                  morto = true;
               }
               if(angulo >= 0.5){
                  angulo = 0.5;
               }
               pos_y_passaro += speed;
               speed += gravidade;
               angulo += 0.05;
               desenha = 1;
               frame++;
            }
         }
      } else if(evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP || evento.type == ALLEGRO_EVENT_KEY_DOWN){
         if(evento.keyboard.keycode == 84){
            subir = true;
            if(!morto){
               al_play_sample(som_pulo, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            if(!inicia){
               al_play_sample(som_inicio, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
               inicia = 1;
            }
            if(morto){
               random = rand() % 2;
               rand_bird = rand() % 3;
               rand_pipe = rand() % 2;
               inicia = 0;
               score = unidade = dezena = centena = 0;
               pos_x_unidade = 130, pos_x_dezena = 125, pos_x_centena = 120;
               morto = false;
            }
         }
      } else if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
         sair = 1;
      }
      if(desenha && al_is_event_queue_empty(fila_eventos)){
         if(!inicia){
            al_draw_bitmap(fundo[random], pos_x_fundo, pos_y_fundo,0);
            al_draw_bitmap(fundo[random], pos_x_fundo1, pos_y_fundo1,0);
            al_draw_bitmap(base, pos_x_base, pos_y_base, 0);
            al_draw_bitmap(base1, pos_x_base1, pos_y_base1, 0);
            al_draw_bitmap(message, 50, 150, 0);
         } else {
            al_clear_to_color(al_map_rgb(0,0,0));

            al_draw_bitmap(fundo[random], pos_x_fundo, pos_y_fundo,0);
            al_draw_bitmap(fundo[random], pos_x_fundo1, pos_y_fundo1,0);

            al_draw_bitmap(cano[rand_pipe], pos_x_cano, pos_y_cano_invertido + 120, 0);
            al_draw_bitmap(cano[rand_pipe], pos_x_cano1, pos_y_cano_invertido1 + 120, 0);
            al_draw_bitmap(cano[rand_pipe], pos_x_cano2, pos_y_cano_invertido2 + 120, 0);

            al_draw_rotated_bitmap(cano_invertido[rand_pipe], 52, 0, pos_x_cano, pos_y_cano_invertido, 3.14, 1);
            al_draw_rotated_bitmap(cano_invertido[rand_pipe], 52, 0, pos_x_cano1, pos_y_cano_invertido1, 3.14, 1);
            al_draw_rotated_bitmap(cano_invertido[rand_pipe], 52, 0, pos_x_cano2, pos_y_cano_invertido2, 3.14, 1);

            al_draw_bitmap(base, pos_x_base, pos_y_base, 0);
            al_draw_bitmap(base1, pos_x_base1, pos_y_base1, 0);

            al_draw_bitmap(scores[unidade], pos_x_unidade, 100, 0);
            if(score > 9){
               al_draw_bitmap(scores[dezena], pos_x_dezena, 100, 0);
               if(score > 99){
                  al_draw_bitmap(scores[centena], pos_x_centena, 100, 0);
               }
            }

            al_draw_rotated_bitmap(passaros[rand_bird][0], 0, 0, pos_x_passaro, pos_y_passaro, angulo, 0);
            if(frame == 0){ al_draw_rotated_bitmap(passaros[rand_bird][0], 0, 0, pos_x_passaro, pos_y_passaro, angulo, 0); }
            else if (frame == 1) { al_draw_rotated_bitmap(passaros[rand_bird][1], 0, 0, pos_x_passaro, pos_y_passaro, angulo, 0); }
            else if (frame == 2) { al_draw_rotated_bitmap(passaros[rand_bird][2], 0, 0, pos_x_passaro, pos_y_passaro, angulo, 0); }

            if(morto){
               al_draw_bitmap(game_over, 50, W_HEIGHT/3, 0);
            }
         }
         desenha = 0;
      }
      al_flip_display();
   }
}

/*
MENU *OPCIONAL
MOSTRAR SCORES ANTERIORES
*/
