__int64 __fastcall GetPrinterResponse(unsigned __int8 *pCmd, unsigned __int8 *apRespData, int a3, int *apRespDataLen)
{
  int *pRespDataLen; // r12
  int v5; // er15
  unsigned __int8 *pRespData; // rbp
  unsigned __int8 *v7; // r14
  int v9; // eax
  unsigned int v10; // edi
  int v11; // esi
  signed int v12; // eax

  pRespDataLen = apRespDataLen;
  v5 = a3;
  pRespData = apRespData;
  v7 = pCmd;
  if ( a3 > 2048 )
    return 0i64;
  if ( !dword_7FFF80E398F8 )
  {
    if ( g_bPipeCommMode )
      return Pipe_GetResponse(pCmd, apRespData, a3, apRespDataLen);
    v10 = 1;
    v11 = 0;
    while ( _InterlockedExchange(&dword_7FFF80E39900, 1) )
    {
      Sleep(0xAu);
      if ( ++v11 > 200 )
        return 0i64;
    }
    if ( v7 )
    {
      if ( dword_7FFF80E398F0 )
        *v7 += 10;
      if ( v5 < 1 )
        return 0i64;
      if ( (unsigned int)sub_7FFF80DBBCB0(1i64, v7, (unsigned int)v5, 1000i64) != v5 )
        v10 = 0;
      if ( dword_7FFF80E398F0 )
        *v7 -= 10;
      if ( *v7 == 47 || *v7 == 99 )
      {
LABEL_34:
        _InterlockedExchange(&dword_7FFF80E39900, 0);
        return v10;
      }
    }
    else if ( !pRespData )
    {
      return sub_7FFF80DBAAE0();
    }
    v12 = sub_7FFF80DBB580(0i64, &unk_7FFF80E39C10, 2048i64, 1000i64);
    if ( v12 < *pRespDataLen )
      *pRespDataLen = v12;
    if ( v12 >= 1 )
    {
      if ( pRespData )
        memmove(pRespData, &unk_7FFF80E39C10, *pRespDataLen);
    }
    else
    {
      v10 = 0;
    }
    goto LABEL_34;
  }
  if ( !dword_7FFF80E398FC )
  {
    v9 = sub_7FFF80DBA740(&unk_7FFF80E3AC10, pCmd, apRespData, a3, apRespDataLen, dword_7FFF80E398F0);
    if ( v9 == -1 )
    {
      dword_7FFF80E398FC = 1;
    }
    else if ( v9 )
    {
      return v9 == 1;
    }
  }
  return 0i64;
}


__int64 ClearProgramBuf(void)
{
  __int16 v1; // [rsp+30h] [rbp+8h]
  int v2; // [rsp+38h] [rbp+10h]

  v2 = 2048;
  v1 = 0x43;
  return GetPrinterResponse((unsigned __int8 *)&v1, 0i64, 1, &v2);
}