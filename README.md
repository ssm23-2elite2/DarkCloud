DarkCloud
=========

안녕하세요. Dark Cloud System Repository에 오신 여러분을 환영합니다.
Dark Cloud System은 기존 Cloud System이 가진 동기화 방식의 한계점을 탈피하고 직접참조 방식의 Cloud System을 구현한 것입니다.



#1.Dark Cloud Client 시작하기
(모든 시작하기의 내용은 ubuntu 12.04 를 기준으로 작성되있습니다.)

먼저, Dark Cloud System을 활용한 Client를 구성하기 위해서 Android Framework Source를 다운로드 해야합니다.
  Android Framework를 다운로드 받고 넥서스 S에 포팅하는 방법은 아래 링크를 참고하세요.

    http://yjaeseok.tistory.com/296
 
다음으로, Dark Cloud Full Source를 다운로드 받습니다. Dark Cloud를 다운로드 받을 폴더를 추가합니다.

    $mkdir ~/DarkCloud

Dark Cloud 소스를 다운받습니다.

    $git full https://github.com/ssm23-2elite2/DarkCloud.git

Dark Cloud 소스는 다음과 같이 구성되어있습니다.
   
    /src					-- 소스 루트
	  /android              -- client 소스 루트
	     /Settings			-- Android Framework 설정화면을 위한 정보
		 /SystemUI			-- 
      /server				-- server 소스 루트
	     /fileStorage		-- cloud의 파일들이 저장되는 경로
		 /html				-- server의 html 소스
		 /node_modules		-- nodejs에서 활용되는 모듈
		 /public			
		    /stylesheets
		 /routes			
		 /views

Android Framework Source의 일부를 Dark Cloud Repository에서 받은 파일로 교체합니다.

    $cp -rf ~/DarkCloud/src/android/settings ~/framework/packages/apps/Settings/src/com/android/settings
    $cp -rf ~/DarkCloud/src/android/SystemUI ~/framework/frameworks/base/packages/SystemUI




감사한 분들:

http://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/
https://www.ibm.com/developerworks/linux/library/l-fuse/
https://github.com/jcline/fuse-google-drive
