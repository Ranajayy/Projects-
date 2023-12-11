byte bcdToDec(byte val){
  return((val/16*10) + (val%16));
}
byte decToBcd(byte val){
  return((val/10*16)+(val%10));
}

void printBinary(int value){
 for(int mask = 0x80; mask; mask >>= 1){
   if(mask  & value)
       Serial.print('1');
   else
       Serial.print('0');
 }
}