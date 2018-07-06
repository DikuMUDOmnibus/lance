/*
This is the code for the slot machines
*/

void deduct_bet (CHAR_DATA * ch, int bet)
{
    int gold = 0, steel = 0;

    gold = UMIN (ch->gold, bet);

    if (gold < bet)
    {
        steel = ((bet - gold + 99) / 100);
        gold = bet - 100 * steel;
    }

    ch->steel -= steel;
    ch->gold -= gold;

    if (ch->steel < 0)
    {
        bug ("deduct bet: steel %d < 0", ch->steel);
        ch->steel = 0;
    }
    if (ch->gold < 0)
    {
        bug ("deduct bet: gold %d < 0", ch->gold);
        ch->gold = 0;
    }
}


// - Slot Machines by Nikola(manga.thedigitalrealities.com port: 6969)
// This short function is for showing the result after it is selected.

void show_slot(CHAR_DATA *ch, int num)
{
    switch(num)
    {
        case 1:
          send_to_char("   {Rcherry{X   ",ch);
          break;
        case 2:
          send_to_char("    {mplum{X    ",ch);
          break;
        case 3:
          send_to_char("   {Mgrape{X    ",ch);
          break;
        case 4:
          send_to_char(" {Gwatermelon{X ",ch);
          break;
        case 5:
          send_to_char("   {yorange{X   ",ch);
          break;
        case 6:
          send_to_char("   {Ylemon{X    ",ch);
          break;
        case 7:
          send_to_char("    {Dbar{X     ",ch);
          break;
        case 8:
          send_to_char("    {Y2{Dbar{X    ",ch);
          break;
        case 9:
          send_to_char("    {R3{Dbar{X    ",ch);
          break;
        case 10:
          send_to_char("   {Bseven{X    ",ch);
          break;
    }
return;
}

// Rolls the slots.

int roll(void)
{
        int num;
        num = number_range(1,10);
        return num;
}



// The main Slots function

void do_slot(CHAR_DATA *ch, char *argument) 
{
        OBJ_DATA *slot=NULL, *obj;
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        char buf[MSL];
        bool CanPlay = TRUE;
        bool winner = FALSE;
        int winnings=0, basebet=0;
        int sa=0, sb=0, sc=0, sd=0, se=0, sf=0, sg=0, sh=0, si=0;

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );

        // No mobs playing slots
        if(IS_NPC(ch))
        {
          send_to_char("Mobs cannot gamble.\n\r",ch);
          return;
        }

	    // Check the room for the object.
        for ( obj = object_list; obj != NULL; obj = obj->next )
        {
          if(!obj->in_room)
            continue;
          if(obj->in_room != ch->in_room)
            continue;
          if (obj->item_type == ITEM_SLOT_MACHINE )
          {
            slot = obj;
            break;
          }
        }
        // No Slot Machine, end it.
        if(slot == NULL)
        {
          send_to_char("There's no slot machine here.\n\r",ch);
          return;
        }

        if(arg1[0] == '\0')
        {
         switch (number_range (0,4))
            {
            case 0:
                send_to_char("The lovely waitress walks up and says '{gYou need to type Slot Play (amount){x'\n\r"
                             "She then turns away and walks off with a sultry swing to her perfect behind.\n\r",ch);
                   break;
            case 1:
                   send_to_char("A rather homely waitress passes by with the drink tray.\n\r"
                                "The waitress says '{gHey, you gotta type 'Slot Play (amount){x'\n\r"
                                "She turns and walks away with a limp.\n\r",ch);
                   break;
            case 2:
                   send_to_char("An amazing waitress walks over and bends down in front of you at the machine.\n\r"
                                "The waitress says '{gSugar, you need to type 'Slot Play (amount){x'\n\r"
                                "\n\r...You're not sure if you heard her though, has her amazing breasts brushed your arm!\n\r",ch);
                   break;
            case 3:
                   send_to_char("A waiter named Juan walks up with a swinging gate.\n\r"
                                "Juan says with a Cuban accent '{gHoney, you need to type 'Slot Play (amount){x'\n\r"
                                "He gives you a wink and walks away with a saunter.\n\r",ch);
                   break;
            case 4:
                   send_to_char("An amazing waitress walks over and bends down giving you an eyefull.\n\r"
                                "The waitress says '{gSugar, you need to type 'Slot Play (amount){x'\n\r"
                                "\n\r",ch);
                   send_to_char("{r      ,8X--------8,{x                  {r`88,\n\r"
                                "{r     ,8X---------X8,{x     I,          {r8X-8,\n\r"
                                "{r    ,8X-----------X8,{x     I,        {r`8X-88,\n\r"
                                "{r   ,8X-------------X8b,{x    I,        {r8XXX-88I,\n\r"
                                "{r   8X----------------XX8,{x   I        {r8XX---XX8,\n\r"
                                "{r  8X-------------------X8,{x  I        {r8X------XX8,\n\r",ch);
                   send_to_char("{r  8X------Xz--zX--------X8,{x 8        {r8X-----X---X8,\n\r"
                                "{r  8X-------XzzX---------X8,{x 8       {r(8X------XXX-8,\n\r"
                                "{r  `8X--------------------X,{x 8,     {r,8X---------X8\n\r"
                                "{r    88X------------------X,{x 8     {r,8X---------X8{x\n\r"
                                "\n\r...But you barely hear a word as your eyes are glued to her phenomenal chest.\n\r",ch);
                   break;
            default:
                   send_to_char("You have to use the proper syntax, slot play (bet amount).\n\r", ch);
            }
           return;
        }


	//Now for the fun part.

        else if(!strcmp(arg1, "play"))
        {
          if(arg2[0] == '\0')
          {
         switch (number_range (0,4)) // A little humor and spice for the mistyped or 
            {
            case 0:
                send_to_char("The lovely waitress walks up and says '{gYou need to type Slot Play (amount){x'\n\r"
                             "She then turns away and walks off with a sultry swing to her perfect behind.\n\r",ch);
                   break;
            case 1:
                   send_to_char("A rather homely waitress passes by with the drink tray.\n\r"
                                "The waitress says '{gHey, you gotta type 'Slot Play (amount){x'\n\r"
                                "She turns and walks away with a limp.\n\r",ch);
                   break;
            case 2:
                   send_to_char("An amazing waitress walks over and bends down in front of you at the machine.\n\r"
                                "The waitress says '{gSugar, you need to type 'Slot Play (amount){x'\n\r"
                                "\n\r...You're not sure if you heard her though, has her amazing breasts brushed your arm!\n\r",ch);
                   break;
            case 3:
                   send_to_char("A waiter named Juan walks up with a swinging gate.\n\r"
                                "Juan says with a Cuban accent '{gHoney, you need to type 'Slot Play (amount){x'.\n\r"
                                "He gives you a wink and walks away with a saunter.\n\r",ch);
                   break;
            case 4:
                   send_to_char("An amazing waitress walks over and bends down giving you an eyefull.\n\r"
                                "The waitress says '{gSugar, you need to type 'Slot Play (amount){x'.\n\r"
                                "\n\r",ch);
                   send_to_char("{r      ,8X--------8,{x                  {r`88,\n\r"
                                "{r     ,8X---------X8,{x     I,          {r8X-8,\n\r"
                                "{r    ,8X-----------X8,{x     I,        {r`8X-88,\n\r"
                                "{r   ,8X-------------X8b,{x    I,        {r8XXX-88I,\n\r"
                                "{r   8X----------------XX8,{x   I        {r8XX---XX8,\n\r"
                                "{r  8X-------------------X8,{x  I        {r8X------XX8,\n\r",ch);
                   send_to_char("{r  8X------Xz--zX--------X8,{x 8        {r8X-----X---X8,\n\r"
                                "{r  8X-------XzzX---------X8,{x 8       {r(8X------XXX-8,\n\r"
                                "{r  `8X--------------------X,{x 8,     {r,8X---------X8\n\r"
                                "{r    88X------------------X,{x 8     {r,8X---------X8{x\n\r"
                                "\n\r...But you barely hear a word as your eyes are glued to her phenomenal chest.\n\r",ch);
                   break;
            default:
                   send_to_char("You have to use the proper syntax, slot play (bet amount).\n\r", ch);
            }
           return;
          }


   //I made it so that only certain amounts can be wagered, it would be
   //entirely possible to remove this and go with however much they want
   //to wager, however I added this to give the slots a touch of reality. - NS

   // I completely changed the argument structure, to allow people some freedom in how
   // they call money, you'll see below. - DSL

  if ((!strcmp(arg2, "quarter"))
   || (!strcmp(arg2, "25"))
   || (!strcmp(arg2, "25 cents"))
   || (!strcmp(arg2, ".25")))
    {
      CanPlay = TRUE;
      basebet = 25;
    }
  else if ((!strcmp(arg2, "50 cents"))
   || (!strcmp(arg2, "50"))
   || (!strcmp(arg2, ".50")))
    {
      CanPlay = TRUE;
      basebet = 50;
    }
  else if ((!strcmp(arg2, "1 buck"))
   || (!strcmp(arg2, "1"))
   || (!strcmp(arg2, "1 dollar"))
   || (!strcmp(arg2, "dollar"))
   || (!strcmp(arg2, "buck"))
   || (!strcmp(arg2, "100")))
    {
      CanPlay = TRUE;
      basebet = 100;
    }
  else if ((!strcmp(arg2, "2 bucks"))
   || (!strcmp(arg2, "2"))
   || (!strcmp(arg2, "2 dollars"))
   || (!strcmp(arg2, "200")))
    {
      CanPlay = TRUE;
      basebet = 200;
    }
  else if ((!strcmp(arg2, "4 bucks"))
   || (!strcmp(arg2, "4"))
   || (!strcmp(arg2, "4 dollars"))
   || (!strcmp(arg2, "400")))
    {
      CanPlay = TRUE;
      basebet = 400;
    }
  else if ((!strcmp(arg2, "8 bucks"))
   || (!strcmp(arg2, "8"))
   || (!strcmp(arg2, "8 dollars"))
   || (!strcmp(arg2, "800")))
    {
      CanPlay = TRUE;
      basebet = 800;
    }
  else if ((!strcmp(arg2, "10 bucks"))
   || (!strcmp(arg2, "10"))
   || (!strcmp(arg2, "10 dollars"))
   || (!strcmp(arg2, "1000")))
    {
      CanPlay = TRUE;
      basebet = 1000;
    }
  else if ((!strcmp(arg2, "20 bucks"))
   || (!strcmp(arg2, "20"))
   || (!strcmp(arg2, "20 dollars"))
   || (!strcmp(arg2, "2000")))
    {
      CanPlay = TRUE;
      basebet = 2000;
    }
  else
    {
      CanPlay = FALSE;
      send_to_char("You can bet a quarter, 50 cents, 1,2,4,8,10 or 20 dollars.\n\r", ch);
      return;
    }

     // The original steel/gold check was buggy...
     // Had to change it all, funky math and it messed up, so I changed it. - DSL

        if ((ch->gold + 100 * ch->steel) < basebet)
        {
        switch (number_range (0, 5))
           {
           case 0:
              send_to_char ("Man, you're so pour you could afford the cheap drinks here.\n\r", ch);
              break;
           case 1:
              send_to_char ("You scrounge through your pockets and find some...\n\r"
                            " ...lint... but not enough money to play.\n\r", ch);
              break;
           case 2:
              send_to_char ("Someone must have lifted your wallet pal, you don't have enough cash.\n\r", ch);
              break;
           case 3:
              send_to_char ("You got a copier that makes $100s?\n\r"
                            "Because you don't have the greenbacks baby.\n\r", ch);
              break;
           case 4:
              send_to_char ("Go hold up a liquor store or something, you don't have enough to cover that bet.\n\r", ch);
              break;
           case 5:
              send_to_char ("With what, wooden nickles? Get more cash.\n\r", ch);
              break;
           default:
              send_to_char ("You don't have that much money.\n\r", ch);
              break;
           }
            return;
        }

        deduct_bet (ch, basebet);
        // Progressive jackpot DSL... IT WORKS!!
        if (slot->value[0] == SLOT_IS_PROGRESSIVE)
        slot->value[1] += basebet;

	    //'steel' and 'gold' are 'dollars' and 'cents',

          if(basebet == 25)
          {
            send_to_char ("You put a quarter into the slot machine and pull the handle.\n\r",ch);
            act("$n puts a coin in the slot machine and pulls the handle.",ch,NULL,NULL,TO_ROOM);
          }
          else if(basebet == 50)
          {
            send_to_char ("You put two quarters into the slot machine and pull the handle.\n\r",ch);
            act("$n puts a couple coins in the slot machine and pulls the handle.",ch,NULL,NULL,TO_ROOM);
          }
          else if(basebet == 100)
          {
            send_to_char("You slide a dollar into the slot machine and pull the handle.\n\r",ch);
            act("$n slides a dollar in the slot machine and pulls the handle.",ch,NULL,NULL,TO_ROOM);
          }
          else if(basebet > 100)
          {
            sprintf(buf,"You slide %d dollars into the slot machine and pull the handle.\n\r",basebet/100);
            send_to_char(buf,ch);
            act("$n slides some bills in the slot machine and pulls the handle.",ch,NULL,NULL,TO_ROOM);
         }
        send_to_char ("The machine spins and whirs for a bit.\n\r", ch);

        // A slight wait for mortals
        if (!IS_IMMORTAL(ch))
        WAIT_STATE (ch, PULSE_VIOLENCE / 3);

	      //Roll the slots!
          sa = roll();
          sb = roll();
          sc = roll();
          sd = roll();
          se = roll();
          sf = roll();
          sg = roll();
          sh = roll();
          si = roll();

	  //The following section reveals the slots to the player.

            send_to_char ("                    .--------.\n\r", ch);
            send_to_char ("                    |Jackpot |\n\r", ch);
            send_to_char ("   _________________|________|_________________\n\r", ch);
            send_to_char ("  |==____________==____________==____________==|\n\r", ch);
            send_to_char ("  |||            ||            ||            |||\n\r", ch);
            send_to_char ("  |||",ch);
            show_slot(ch,sa);
            send_to_char(                  "||",ch);
            show_slot(ch,sb);
            send_to_char(                                "||",ch);
            show_slot(ch,sc);
            send_to_char(                                              "||| __\n\r", ch);
            send_to_char ("  |||____________||____________||____________|||(__)\n\r", ch);
            send_to_char ("  |==____________==____________==____________==| ||\n\r", ch);
            send_to_char ("  |||            ||            ||            ||| ||\n\r", ch);
            send_to_char ("  |||",ch);
            show_slot(ch,sd);
            send_to_char(                  "||",ch);
            show_slot(ch,se);
            send_to_char(                                "||",ch);
            show_slot(ch,sf);
            send_to_char(                                              "||| ||\n\r",ch);
            send_to_char ("  |||____________||____________||____________|||_//\n\r", ch);
            send_to_char ("  |==____________==____________==____________==|_/\n\r", ch);
            send_to_char ("  |||            ||            ||            |||\n\r", ch);
            send_to_char ("  |||",ch);
            show_slot(ch,sg);
            send_to_char(                  "||",ch);
            show_slot(ch,sh);
            send_to_char(                                "||",ch);
            show_slot(ch,si);
            send_to_char(                                              "|||\n\r",ch);
            send_to_char ("  |||____________||____________||____________|||\n\r", ch);
            send_to_char ("  |==________________________________________==|\n\r", ch);
            send_to_char ("  | |________________________________________| |\n\r", ch);
            send_to_char ("  |  |      TOP SECRET - SLOT MACHINES      |  |\n\r", ch);
            send_to_char (" _|  |______________________________________|  |_\n\r", ch);
            send_to_char ("(________________________________________________)\n\r", ch);


	  //Decide whether or not they won.

          if(sa == 1 && sb != 1)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet + (basebet/4);
          }
          else if(sa == 1 && sb == 1 && sc != 1)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet + (basebet/2);
          }

          if(sd == 1 && se != 1)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet + (basebet/4);
          }
          else if(sd == 1 && se == 1 && sf != 1)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet + (basebet/2);
          }

          if(sg == 1 && sh != 1)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet + (basebet/4);
          }
          else if(sg == 1 && sh == 1 && si != 1)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet + (basebet/2);
          }

          if(sa == sb && sb == sc)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet * sb;
          }

          if(sd == se && se == sf)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet * se;
          }

          if(sg == sh && sh == si)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet * sh;
          }

          if(sa == se && se == si)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet * se;
          }

          if(sg == se && se == sc)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet * se;
          }

          if(sa == sd && sd == sg)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet * sd;
          }

          if(sb == se && se == sh)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet * se;
          }

          if(sc == sf && sf == si)
          {
            if(winner == FALSE)
              winner = TRUE;
            winnings += basebet * sf;
          }

	  //This is the JACKPOT!  It gives a special message and is therefore
	  //separated from the regular winning check.  I have never hit the
	  //jackpot, so I don't really know if it works right. :-p - NS

      // But I have, it works perfectly now. - DSL
          if(sa == 10 && se == 10 && si == 10 && sg == 10 && sc == 10)
          {
            send_to_char("The red light on top of your machine lights up and spins!!!\n\r\n\r\n\r", ch);
            send_to_char("{WWOOHOOOO!!!{x\n\r{R",ch);
            send_to_char("       ___   ____       ____      ___   ___ \n\r"
                         "      |   |.'    `.   .'    `.   |   | |   |\n\r"
                         "      |   ||    _  `-.|    _  `-.|   | |   |\n\r"
                         "      |   ||   | `   ||   | `   ||   | |   |\n\r"
                         "      |   ||   |_|   ||   | |__.'|   |_/  .'\n\r"
                         " ___  |   ||    _    ||   |  ___ |    _   `.\n\r"
                         "|   | |   ||   | |   ||   | |   ||   | `   |\n\r"
                         "|   | |   ||   | |   ||   | |   ||   | |   |\n\r"
                         "|   `_|   ||   | |   ||   |_/   ||   | |   |\n\r"
                         "`-._____.-'|___| |___|`-._____.-'|___| |___|\n\r", ch);
            send_to_char("      ____       ____      _________  ___ \n\r"
                         "    .'    `.   .'    `.   |         ||   |\n\r"
                         "    |    _  `-.|    _  `-.|__     __||   |\n\r"
                         "    |   | `   ||   | `   |   |   |   |   |\n\r"
                         "    |   | |   ||   | |   |   |   |   |   |\n\r"
                         "    |   |_/   ||   | |   |   |   |   |   |\n\r"
                         "    |    ___.-'|   | |   |   |   |   |___|\n\r"
                         "    |   |      |   | |   |   |   |   _____\n\r"
                         "    |   |      |   |_/   |   |   |   |   |\n\r"
                         "    |___|      `-._____.-'   |___|   |___|\n\r{x", ch);

            act("The red light on top of $n's slot machine lights up and spins!", ch,NULL,NULL,TO_ROOM);
            act       ("{R       ___   ____       ____      ___   ___ \n\r"
                         "      |   |.'    `.   .'    `.   |   | |   |\n\r"
                         "      |   ||    _  `-.|    _  `-.|   | |   |\n\r"
                         "      |   ||   | `   ||   | `   ||   | |   |\n\r"
                         "      |   ||   |_|   ||   | |__.'|   |_/  .'\n\r"
                         " ___  |   ||    _    ||   |  ___ |    _   `.\n\r"
                         "|   | |   ||   | |   ||   | |   ||   | `   |\n\r"
                         "|   | |   ||   | |   ||   | |   ||   | |   |\n\r"
                         "|   `_|   ||   | |   ||   |_/   ||   | |   |\n\r"
                         "`-._____.-'|___| |___|`-._____.-'|___| |___|", ch, NULL, NULL, TO_ROOM);
            act       ("{R      ____       ____      _________  ___ \n\r"
                         "    .'    `.   .'    `.   |         ||   |\n\r"
                         "    |    _  `-.|    _  `-.|__     __||   |\n\r"
                         "    |   | `   ||   | `   |   |   |   |   |\n\r"
                         "    |   | |   ||   | |   |   |   |   |   |\n\r"
                         "    |   |_/   ||   | |   |   |   |   |   |\n\r"
                         "    |    ___.-'|   | |   |   |   |   |___|\n\r"
                         "    |   |      |   | |   |   |   |   _____\n\r"
                         "    |   |      |   |_/   |   |   |   |   |\n\r"
                         "    |___|      `-._____.-'   |___|   |___|{x", ch, NULL, NULL, TO_ROOM);
            if (slot->value[0] == SLOT_IS_PROGRESSIVE)
            {
//                winnings += basebet * 200; // Was 50
                winnings += slot->value[1];
                slot->value[1] = 500000; // Reset the machine to $5,000
                send_to_char ("A guard pushes a money cart out and unlocks the machine.\n\r"
                              "He proceeds to put a large amount of money back into it.\n\r", ch);
                act ("$n sits back as a guard pushes a money cart out and unlocks $s machine.\n\r"
                     "The guard proceeds to put a large sum of money into the machine.\n\r"
                     "He then turns and pushes the cart away.", ch, NULL, NULL, TO_ROOM);

            }
            else
            {
                  winnings += slot->value[1];
            }
            if(winnings < 100)
            {
              sprintf(buf,"{WYou receive %d cents!{x\n\r",winnings);
              send_to_char(buf,ch);
              ch->gold += winnings;
            }
            else if(winnings == 100)
            {
              send_to_char("{WYou won a buck!{x\n\r",ch);
              ch->steel += 1;
            }
            else if(winnings > 100) // I added in cents so that the player got ALL the winnings. - DSL
            {
              sprintf(buf,"{WYou won $%d.%2.2d!{X\n\r",winnings/100, (winnings - ((winnings/100) * 100)));
              send_to_char(buf,ch);
              ch->steel += winnings/100;
              ch->gold += (winnings - ((winnings/100) * 100));
            }
          }

	  //Regular winner.

          else if(winner == TRUE)
          {
            send_to_char("{YDING! DING! DING! A winner!{X\n\r",ch);
            act("$n's slot machine flashes and makes loud noises.",ch,NULL,NULL,TO_ROOM);
            if(winnings < 100)
            {
              sprintf(buf,"{WYou won %d cents!{X\n\r",winnings);
              send_to_char(buf,ch);
              ch->gold += winnings;
            }
            else if(winnings == 100)
            {
              send_to_char("{WYou won a buck!{X\n\r",ch);
              ch->steel += 1;
            }
            else if(winnings > 100)
            {
              sprintf(buf,"{WYou won $%d.%2.2d!{X\n\r",winnings/100, (winnings - ((winnings/100) * 100)));
              send_to_char(buf,ch);
              ch->steel += winnings/100;
              ch->gold += (winnings - ((winnings/100) * 100));
            }
            if (slot->value[0] == SLOT_IS_PROGRESSIVE)
              {
                  slot->value[1] -= winnings;
                  if (slot->value[1] <= 0) // No negative amounts now ;p - DSL
                  {
                     slot->value[1] = 250000;
                     send_to_char ("A guard pushes a money cart out and unlocks the machine.\n\r"
                                   "He proceeds to put a large amount of money back into it.\n\r", ch);
                     act ("$n sits back as a guard pushes a money cart out and unlocks $s machine.\n\r"
                          "The guard proceeds to put a large sum of money into the machine.\n\r"
                          "He then turns and pushes the cart away.", ch, NULL, NULL, TO_ROOM);
                  }
              }
          }

	  //Loser. :(

          else
          {
         switch (number_range (0,21)) // A little humor and spice for the mistyped or 
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                  send_to_char("\n\r{RDamn.{x\n\rThe one arm bandit struck again... you lost.\n\r",ch);
                   break;
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
                   send_to_char("\n\rHOLY CRAP!!! You won... not a damned thing.\n\r",ch);
                   break;
            case 19:
                   send_to_char("\n\rAn beautiful waitress walks over and whispers into your ear \"{yYou lost hon{x\".\n\r",ch);
                   break;
            case 20:
                   send_to_char("\n\rYou see a sign flashing near the door that reads \n\r"
                                "\"1-800-NO-GAMBLE\"\n\r"
                                "Then below it \"Gamblers Anonymous\"... You lost pal.\n\r",ch);
                   break;
            case 21:
                   send_to_char("\n\rA sexy chick walks by with her tits bouncing like melons.\n\r"
                                "\n\r",ch);
                   send_to_char("{b      ,8X--------8,{x                  {b`88,\n\r"
                                "{b     ,8X---------X8,{x     I,          {b8X-8,\n\r"
                                "{b    ,8X-----------X8,{x     I,        {b`8X-88,\n\r"
                                "{b   ,8X-------------X8b,{x    I,        {b8XXX-88I,\n\r"
                                "{b   8X----------------XX8,{x   I        {b8XX---XX8,\n\r"
                                "{b  8X-------------------X8,{x  I        {b8X------XX8,\n\r",ch);
                   send_to_char("{b  8X------Xz--zX--------X8,{x 8        {b8X-----X---X8,\n\r"
                                "{b  8X-------XzzX---------X8,{x 8       {b(8X------XXX-8,\n\r"
                                "{b  `8X--------------------X,{x 8,     {b,8X---------X8\n\r"
                                "{b    88X------------------X,{x 8     {b,8X---------X8{x\n\r"
                                "\n\r...You were so mesmerized you didn't even notice that you lost your bet.\n\r",ch);
                   break;
            default:
                   send_to_char("\n\rHey, does one of each count for anything?\n\r Well shit, try again.\n\r", ch);
            }
          }
        return;
        }
        else
        {
          send_to_char("Slot commands{W: <{XPLAY{W>{X\n\rFor more information see {W'{XHELP SLOTS{W'{X.\n\r",ch);
          return;
        }
return;
}