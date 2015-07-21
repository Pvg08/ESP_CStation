
char* getMessageParam(char* message, char const * param_name, bool break_line)
{
  int param_len = strlen(param_name);
  if (strncmp(message, param_name, param_len)==0) {
      message += param_len;
      int message_len = strlen(message);
      int i;
      
      if (break_line) {
        for(i = 0; i<message_len && message[i]!='\r'; i++);
      } else {
        i = message_len-1;
      }
      
      message[i]=0;
      return message;
  }
  return NULL;
}

unsigned long int readIntFromString(const char* message, unsigned from_pos)
{
  unsigned long int result = 0;

  while (message && message[from_pos]>='0' && message[from_pos]<='9') {
    result = 10*result + message[from_pos] - '0';
    from_pos++;
  }

  return result;
}

bool readLineToStr(const char* message, char* buff, unsigned buff_maxlen, unsigned from_pos, unsigned* next_line_pos) 
{
  if (message && buff && buff_maxlen && message[from_pos]) {
    unsigned i = 0;
    buff_maxlen--;
    message+=from_pos;
    
    for(i=0; i<buff_maxlen && message[i] && message[i]!='\r' && message[i]!='\n'; i++) {
      buff[i] = message[i];
    }
    buff[i] = 0;
    
    if (next_line_pos) {
      if (message[i]=='\r' || message[i]=='\n') i++;
      if (message[i]=='\r' || message[i]=='\n') i++;
      *next_line_pos = from_pos+i;
    }
    return true;
  }
  return false;
}

bool replyIsOK(char* reply)
{
  bool foundOK = false;

  if (reply) {
    int i;
    int reply_len = strlen(reply);
    for (i=0; i<reply_len-1; i++)
    {
      if ( (reply[i]=='O') && (reply[i+1]=='K') ) { 
        foundOK = true;
        break;
      }
    }
  }

  return foundOK;
}
