//
// Created by 傅凌玥 on 2019/5/5.
//

#ifndef TRAINTICKET_CONSTANT_H
#define TRAINTICKET_CONSTANT_H

namespace sjtu{
  const int MAXTICKET = 2000;

  const int NAME_SIZE = 41;
  const int USER_NAME_SIZE = 41 - 24;
  const int PASSWORD_SIZE = 21;
  const int EMAIL_SIZE = 21;
  const int PHONE_SIZE = 21;
  const int ID_SIZE = 21;
  const int LOCATION_SIZE = 21;
  const int DATE_SIZE = 11;
  const int CATALOG_SIZE = 11;
  const int TICKET_KIND_SIZE = 21;
  const int TRAIN_ID_SIZE = 21;
  const int TIME_SIZE = 6;
  const int COMMAND_SIZE = 21;
  const int PRIVILEGE_SIZE = 4;
  const int START_USER_ID = 2018;
  const int MOD = (int)1e9 + 7;
  const int BIT = 129;
  const int MAX_TRAIN_NUM = 5940;
  const int USER_SIZE = USER_NAME_SIZE + PASSWORD_SIZE + EMAIL_SIZE + PHONE_SIZE + sizeof(int);
  const int LOC_SIZE = LOCATION_SIZE + 1 + 2 * sizeof(short) + 5 * sizeof(float) + 31 * 5 * sizeof(short);
  const int TRAIN_SIZE = 2 * sizeof(int) + 2 * sizeof(bool) + sizeof(short) +
      TRAIN_ID_SIZE + 1 + NAME_SIZE + 2 + 2 + 5 * TICKET_KIND_SIZE;
  const int STATION_SIZE = 186 * sizeof(unsigned int);
  const int RECORD_SIZE = 4 * sizeof(unsigned int);
}

#endif
