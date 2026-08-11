/**
 * Copyright 2007 Cyclops Inc. All Rights Reserved.
 */

import java.util.*;
import otpagent.*;



class TestApi {

  public static void main(String args[])throws Exception  {
    if(args == null || args.length < 1 )
    {
      System.out.println("???????");
      return;
    }

    /* 요구되는 필드 값 */
    int otherOrg = 0;                   // 당행 0, 타행 1 -> HOST에서 조회
    String venderCode = "002";          // OTP vender 코드 -> HOST에서 조회
    String userCode = new String("123456-123456");    // 실명번호(텔레뱅킹), 이용자번호(인터넷뱅킹) -> Host에서 조회
    String tokSerial = new String("000012345678");  // 토큰 시리얼 번호 -> HOST에서 조회
    String tokCode = new String("123456");          // 사용자 입력값
    int errCount = 0;                   // 자행 인증 인경우, 사용자 오류 회수 -> Host에서 조회

    System.out.println("Test...");

    otp_agent a = null;

    try {
      a = new otp_agent("/var/ace/sdotpagent.properties");
    } catch(Exception e) {
      System.out.println("\n sdotpagent.properties... where? ");
      return;
    }

    String rCode = a.OTP_000;

    try {

      /* 당행 인증 */
      if( args[0].equals("0") )  {
        otherOrg = 0;
        errCount = 1; /*Host조회 시 가져옴 */
        rCode = a.Af_AuthOtpUser( otherOrg, venderCode, userCode, tokSerial, tokCode, errCount );
      }
      /* 타행 인증 */
      else if( args[0].equals("1") )  {
        otherOrg = 1;
        rCode = a.Af_AuthOtpUser( otherOrg, venderCode, userCode, tokSerial, tokCode, 0 );

        /* 타행 인증 실패 인 경우 -> 에러 count 조회 -> host */
        /*if( rCode.equals(a.OTP_001) ) {
          errCount = a.Af_GetAuthErrorCount();
        }*/
      }
      /* 당행 보정 */
      else if( args[0].equals("2") )  {
        otherOrg = 0;
        rCode = a.Af_ResyncOtpUser( otherOrg, venderCode, userCode, tokSerial, tokCode );
      }
      /* 타행 보정 */
      else if( args[0].equals("3") )  {
        otherOrg = 1;
        rCode = a.Af_ResyncOtpUser( otherOrg, venderCode, userCode, tokSerial, tokCode );
      }
    } catch(Exception e) {
      rCode = a.OTP_200;
    }

    System.out.println("\nReturn Code is " + rCode );
    System.out.println();
  }



}






