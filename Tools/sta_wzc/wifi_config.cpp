//####################################################################
//# � Copyright 2007 Wi-Fi Alliance.  All Rights Reserved
//#
//# Author: Sandeep Mohan  Bharadwaj;  Email: sbharadwaj@wi-fi.org
//# Modified by: Chandra Sekhar Duba;  Email: cduba@wi-fi.org
//# Modification : Added disable flag to disable 
//# 		       windows to control the interface
//####################################################################

#include "stdafx.h"

bool bHandle = false;
bool bInterface = false;
HANDLE hClientHandle = NULL;
PWLAN_INTERFACE_INFO_LIST ppInterfaceList = NULL;
SC_HANDLE hWzcService;
StringType NewIPStr;
StringType NewMaskStr;

int GetIfaces() 
{
	int retGetIfaces = 0;

	if (  !bHandle   ) 
	{
		DWORD dwServiceVersion;
		DWORD retWlanOpenHandle = WlanOpenHandle( 1, NULL, &dwServiceVersion, &hClientHandle );
		if ( retWlanOpenHandle != ERROR_SUCCESS ) 
		{
			// Error opening Wlan Handle
			retGetIfaces = -1;
		} 
		else 
		{
			bHandle = 1;
			retGetIfaces = 0;
		}
	} 

	if (  bHandle && !bInterface ) 
	{
		DWORD retWlanEnumInterfaces = WlanEnumInterfaces( hClientHandle, NULL, &ppInterfaceList );
		if ( retWlanEnumInterfaces != ERROR_SUCCESS ) 
		{
			// Error Getting Interfaces
			retGetIfaces = -2;
		}
	       	else 
		{
			bInterface = 1;
			retGetIfaces = 0;
		}
	} 

	return( retGetIfaces );
}

int StopServices() {
	if ( bInterface ) {
		WlanFreeMemory( ppInterfaceList );
	}

	if ( bHandle ) {
		WlanCloseHandle( hClientHandle, NULL );
	}

	return( 0 );
}

void print_help( ) {
	printf( "\nThis program\nrequires Windows XP SP2, Microsoft Core XML Services (MSXML) 6.0, WLAN API for Windows XP SP2 Beta, and KB893357.\n\n" );
	printf( "See README.TXT for links and further information.\n\n" );
	printf( "Options:\n" );
	printf( " -enable  Enable the wireless interfaces specified by limit.  If no limit\n" );
	printf( "          specified, enable all wireless interfaces.  This option effectively\n" );
	printf( "          checks the \"Use Windows to configure my wireless network settings\"\n" );
	printf( "          checkbox under the Wireless Network Connection Properties.\n" );
	printf( " -disable Disable the wireless interfaces specified by limit.  If no limit\n" );
	printf( "          specified, disable all wireless interfaces.  This option effectively\n" );
	printf( "          unchecks the \"Use Windows to configure my wireless network settings\"\n" );
	printf( "          checkbox under the Wireless Network Connection Properties.\n" );
	printf( " -Add     Path(s) to an XML file containing the information about the\n" );
	printf( "          Wireless Access point that you wish to setup.\n" );
	printf( " -Delete  Access Point name(s) that you wish to remove from the profile list.\n" );
	printf( " -Nics    List wireless interfaces by their description and GUID.\n" );
	printf( " -Aps     List access points by AP name.\n" );
	printf( " -Limit   GUID of the interface(s) to limit the additions, deletions, and\n" );
	printf( "          access point listings to.  The default is to use all interfaces.\n" );
	printf( " -Help    Displays the command line arguments and sample usage (this screen.)\n\n" );
	printf( "Sample Usage:\n" );
	printf( "Delete access point from all interfaces:\n\twifi_config.exe -delete SAMPLE-AP\n\n" );
	printf( "Delete multiple access points from a specific interface:\n\twifi_config.exe -delete SAMPLE-AP1 SAMPLE-AP2 -limit {GUID}\n\n" );
	printf( "Add multiple profiles:\n\twifi_config.exe -add \"C:\\profile1.xml\" \"C:\\profile2.xml\"\n\n" );
	printf( "List access points:\n\twifi_config.exe -aps\n" );
}

bool vector_search( vector <StringType> aVector, StringType search ) 
{
	bool found = false;

	for ( int iCnt = 0; iCnt < (int)aVector.size(); iCnt++ ) 
	{
		if ( aVector[ iCnt ] == search ) 
		{
			found = true;
		}
	}

	return( found );
}
StringType Guid2String( GUID Guid ) {
	StringType sGuid;
	char csGuid[39];
	sprintf_s( csGuid, sizeof( csGuid ), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", Guid.Data1, Guid.Data2, Guid.Data3, Guid.Data4[0], Guid.Data4[1], Guid.Data4[2], Guid.Data4[3], Guid.Data4[4], Guid.Data4[5], Guid.Data4[6], Guid.Data4[7] );
	sGuid = csGuid;
	return( sGuid );
}

int OpenXmlFile( StringType sFileName, StringType &sXmlData ) {
	FILE *pFile;
	long lSize = 0;
	char *buffer = NULL;

	if ( fopen_s( &pFile, sFileName.c_str(), "r" ) != NULL ) {
		return( -1 );
	}

	fseek( pFile ,0 ,SEEK_END );
	lSize = ftell( pFile );
	rewind( pFile );

	buffer = (char*)malloc( lSize );
	if ( buffer == NULL ) {
		return( -2 );
	}

	fread( buffer, 1, lSize, pFile );

	fclose( pFile );
	sXmlData = buffer;
	free( buffer );

	/* I'm doing something wrong with the file read and it is ending up with 
	   trailing garbage.  This removes it. "Well just a second there, professor.
	   We uh, we fixed the *glitch*. So he won't be receiving a paycheck anymore,
	   so it will just work itself out naturally." */
	size_t notwhite = sXmlData.find_last_of( ">" );
    sXmlData.erase( notwhite + 1 ); 

	return( 0 );
}

std::wstring s2ws( const StringType &sConvert ) {
	int cchWideChar;
	int sLength = (int)sConvert.length() + 1;
	cchWideChar = MultiByteToWideChar( CP_ACP, 0, sConvert.c_str(), sLength, 0, 0 );
	wchar_t* buffer = new wchar_t[ cchWideChar ];
	MultiByteToWideChar( CP_ACP, 0, sConvert.c_str(), sLength, buffer, cchWideChar );
	std::wstring wsConvert( buffer );
	delete[] buffer;
	return wsConvert;
}

int get_nics() {
	int retGetIfaces = GetIfaces( );
	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}

	bool bHasOutput = false;
	
	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		if ( !bHasOutput ) 
		{
			bHasOutput = true;
			printf( "Wireless NICs:\n" );
		}
		printf( "%s / ", Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid ).c_str() );
		wprintf( L"%s\n", ppInterfaceList->InterfaceInfo[ dwNicCnt ].strInterfaceDescription );
	}

	if ( bHasOutput ) 
	{
		printf( "\n" );
	}

	return( 0 );
}

int get_aps( vector <StringType> aLimitGuids ) 
{
	int retGetIfaces = GetIfaces();
	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}

	bool bHasOutput = false;
	
	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		StringType sNicGuid = Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid );
		if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
		{
			continue;
		}
		PWLAN_PROFILE_INFO_LIST ppProfileList = NULL;
		WlanGetProfileList( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, NULL, &ppProfileList );
		if ( ( ppProfileList->dwNumberOfItems > 0 ) && ( !bHasOutput ) ) 
		{
			bHasOutput = true;
			printf( "Access Points By Interface:\n" );
		}
		for ( DWORD dwProfCnt = 0; dwProfCnt < ppProfileList->dwNumberOfItems; dwProfCnt++ ) 
		{
			wprintf( L"%s / %s\n", ppInterfaceList->InterfaceInfo[ dwNicCnt ].strInterfaceDescription, ppProfileList->ProfileInfo[ dwProfCnt ].strProfileName );
		}
		WlanFreeMemory( ppProfileList );
	}

	if ( bHasOutput ) 
	{
		printf( "\n" );
	}

	return( 0 );
}
DWORD
StringWToSsid(
    __in LPCWSTR strSsid, 
    __out PDOT11_SSID pSsid
)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    BYTE pbSsid[DOT11_SSID_MAX_LENGTH + 1] = {0};

    if (strSsid == NULL || pSsid == NULL)
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
    }
    else
    {
        pSsid->uSSIDLength = WideCharToMultiByte (CP_ACP,
                                                   0,
                                                   strSsid,
                                                   -1,
                                                   (LPSTR)pbSsid,
                                                   sizeof(pbSsid),
                                                   NULL,
                                                   NULL);

        pSsid->uSSIDLength--;
        memcpy(&pSsid->ucSSID, pbSsid, pSsid->uSSIDLength);
    }

    return dwRetCode;
}
int connect( StringType sSSID,vector <StringType> aLimitGuids ) 
{
	int retGetIfaces = GetIfaces();
	LPWSTR strXML;
	WLAN_CONNECTION_PARAMETERS wlanConnPara;
        DOT11_SSID dot11Ssid = {0};
	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}
	std::wstring swTemp = s2ws( sSSID );
	LPCWSTR wssid = swTemp.c_str();
        wlanConnPara.wlanConnectionMode = wlan_connection_mode_profile;
        wlanConnPara.strProfile = wssid;
        if (( StringWToSsid(wssid, &dot11Ssid)) != ERROR_SUCCESS)
        {
		printf( " - Error starting Wifi API\n" );
		return( -1 );
        }
        wlanConnPara.pDot11Ssid = &dot11Ssid;
        wlanConnPara.dot11BssType = dot11_BSS_type_infrastructure;
        wlanConnPara.pDesiredBssidList = NULL;
        wlanConnPara.dwFlags = 0;

	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		StringType sNicGuid = Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid );
		if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
		{
			continue;
		}
		//WlanGetProfile( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, wssid,NULL,&strXML,NULL,NULL);
		//wprintf( L"%s / %s\n", ppInterfaceList->InterfaceInfo[ dwNicCnt ].strInterfaceDescription, ppProfileList->ProfileInfo[ dwProfCnt ].strProfileName );
		//PWLAN_PROFILE_INFO_LIST ppProfileList = NULL;
		//WlanGetProfileList( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, NULL, &ppProfileList );
		
		//for ( DWORD dwProfCnt = 0; dwProfCnt < ppProfileList->dwNumberOfItems; dwProfCnt++ ) 
		//{
		//	wprintf( L"%s / %s\n", ppInterfaceList->InterfaceInfo[ dwNicCnt ].strInterfaceDescription, ppProfileList->ProfileInfo[ dwProfCnt ].strProfileName );
		//}
		//WlanFreeMemory( ppProfileList );
		printf("\n Connecting...\n");
		//WlanDisconnect( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid,NULL);
		WlanConnect( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, &wlanConnPara,NULL);
	}


	return( 0 );
}
int launch_query( vector <StringType> aLimitGuids,int param ) 
{
	DWORD dwDataSize=0;
	PVOID pData=0;
	int i;
	WLAN_INTERFACE_STATE isState;
        PWLAN_CONNECTION_ATTRIBUTES pCurrentNetwork = NULL;
	int retGetIfaces = GetIfaces();
	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}

	
	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		StringType sNicGuid = Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid );
		if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
		{
			continue;
		}
		//WlanQueryInterface( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, wlan_intf_opcode_interface_state ,NULL,&dwDataSize,&pData,NULL);
		WlanQueryInterface( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, wlan_intf_opcode_current_connection ,NULL,&dwDataSize,&pData,NULL);
                 pCurrentNetwork = (PWLAN_CONNECTION_ATTRIBUTES)pData;
        	if (pCurrentNetwork == NULL)
        	{
            	// no connection information
			printf( " - Error in connection info\n" );
			return( -1 );
        	}
		 //isState = *((PWLAN_INTERFACE_STATE)pData);
		 switch(param)
		 {
			 case STAT:
				 	isState = pCurrentNetwork->isState;
					 if(isState == wlan_interface_state_connected)
						printf("COMPLETED\n");
					 else
						printf("NOT COMPLETED\n");
					 break;
			case BSSID:
        				for (i = 0; i < 6; i++)
        				{
					    if(i < 5)
				             printf("%X:",pCurrentNetwork->wlanAssociationAttributes.dot11Bssid[i]);
					    else
				             printf("%X",pCurrentNetwork->wlanAssociationAttributes.dot11Bssid[i]);
				        }
						printf("\n");
				        //wcout << endl;
					 break;
		}

	}
	WlanFreeMemory(pData);

	return( 0 );
}
int get_ip( vector <StringType> aLimitGuids, int param ) 
{
    DWORD Err;
    int i;
    PFIXED_INFO pFixedInfo;
    DWORD FixedInfoSize = 0;
    /*DWORD FixedInfoSize = 0;*/
    PIP_ADAPTER_INFO pAdapterInfo, pAdapt;
    DWORD AdapterInfoSize=0;
    PIP_ADDR_STRING pAddrStr;
	int retGetIfaces = GetIfaces();
	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}

    //
    // Get the main IP configuration information for this machine using a FIXED_INFO structure
    //
    if ((Err = GetNetworkParams(NULL, &FixedInfoSize)) != 0)
    {
	    if (Err != ERROR_BUFFER_OVERFLOW)
	    {
	        printf("GetNetworkParams sizing failed with error %d\n", Err);
	        return -1;
	    }
    }
    // Allocate memory from sizing information
    if ((pFixedInfo = (PFIXED_INFO) GlobalAlloc(GPTR, FixedInfoSize)) == NULL)
    {
        printf("Memory allocation error\n");
        return -1;
    }
    if ((Err = GetNetworkParams(pFixedInfo, &FixedInfoSize)) != 0)
    {
        printf("GetNetworkParams failed with error %d\n", Err);
        return -1;
    }

	
	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		StringType sNicGuid = Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid );
		if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
		{
			continue;
		}
		if ((Err = GetAdaptersInfo(NULL, &AdapterInfoSize)) != 0)
		{
			 if (Err != ERROR_BUFFER_OVERFLOW)
			 {
			     printf("GetAdaptersInfo sizing failed with error %d\n", Err);
			     return -1;
			 }
		}
    // Allocate memory from sizing information
		if ((pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc(GPTR, AdapterInfoSize)) == NULL)
		{
		    printf("Memory allocation error\n");
		    return -1;
		}
	// Get actual adapter information
		if ((Err = GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize)) != 0)
		{
		    printf("GetAdaptersInfo failed with error %d\n", Err);
		    return -1;
		}
		pAdapt = pAdapterInfo;
		while (pAdapt)
		{
			sNicGuid = pAdapt->AdapterName; 
			if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
			{
				pAdapt=pAdapt->Next;
				continue;
			}
			else
			{
				switch(param)
				{
					case IPADD:
						pAddrStr = &(pAdapt->IpAddressList);
						printf("%s\n", pAddrStr->IpAddress.String);
						break;
					case DHCP:
						printf("%d\n", pAdapt->DhcpEnabled);
						break;
					case DNS:
						printf("%s\n", pFixedInfo->DnsServerList.IpAddress.String);
						pAddrStr = pFixedInfo->DnsServerList.Next;
						while(pAddrStr)
					        {
							printf("%s\n", pAddrStr->IpAddress.String);
						        pAddrStr = pAddrStr->Next;
						}
						break;
					case MASK:
						pAddrStr = &(pAdapt->IpAddressList);
						printf("%s\n", pAddrStr->IpMask.String);
						break;
					case MAC:
						for (i=0; i<pAdapt->AddressLength; i++)
						{
							if (i == (pAdapt->AddressLength - 1))
							        printf("%.2X\n",(int)pAdapt->Address[i]);
							else
								printf("%.2X:",(int)pAdapt->Address[i]);
						}        
						break;
					case ALL:
						printf("%d\n", pAdapt->DhcpEnabled);
						pAddrStr = &(pAdapt->IpAddressList);
						printf("%s\n", pAddrStr->IpAddress.String);
						printf("%s\n", pAddrStr->IpMask.String);
						printf("%s\n", pFixedInfo->DnsServerList.IpAddress.String);
						pAddrStr = pFixedInfo->DnsServerList.Next;
						//while(pAddrStr)
						if(pAddrStr)
					        {
							printf("%s\n", pAddrStr->IpAddress.String);
						        pAddrStr = pAddrStr->Next;
						}
						break;
					case ADDIP:
    						ULONG NTEContext = 0;
						ULONG NTEInstance;
    						CHAR IPStr[128];
    						CHAR MaskStr[128];
						strcpy(IPStr,NewIPStr.c_str());
						strcpy(MaskStr,NewMaskStr.c_str());
        					IPAddr NewIP = inet_addr(IPStr);
        					IPAddr NewMask = inet_addr(MaskStr);
					            printf("AddIPAddress IPs are  %s, %s\n", NewIPStr.c_str(),NewMaskStr.c_str());
        					if ((Err = AddIPAddress(NewIP, NewMask, pAdapt->Index, &NTEContext, &NTEInstance)) != 0)
        					{
					            printf("AddIPAddress failed with error %d, %d\n", NTEContext, Err);
					            return -1;
					        }
						break;
				}
			}
			pAdapt=pAdapt->Next;
		}

	}


	return( 0 );
}

int DeleteAp( StringType sApName, vector <StringType> aLimitGuids ) 
{
	int retGetIfaces = GetIfaces();
	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}

	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		StringType sNicGuid = Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid );
		if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
		{
			continue;
		}
		PWLAN_PROFILE_INFO_LIST ppProfileList = NULL;
		WlanGetProfileList( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, NULL, &ppProfileList );
		for ( DWORD dwProfCnt = 0; dwProfCnt < ppProfileList->dwNumberOfItems; dwProfCnt++ ) 
		{
			char csApName[256];
			WideCharToMultiByte( CP_ACP, 0, ppProfileList->ProfileInfo[ dwProfCnt ].strProfileName, (int)wcslen( ppProfileList->ProfileInfo[ dwProfCnt ].strProfileName ) + 1, csApName, (int)sizeof( csApName ), NULL, NULL );
			if ( csApName == sApName ) 
			{
				DWORD retWlanDeleteProfile = WlanDeleteProfile( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, ppProfileList->ProfileInfo[ dwProfCnt ].strProfileName, NULL );
				if ( retWlanDeleteProfile != ERROR_SUCCESS ) 
				{
					printf( " - Error(%d): Unable to delete AP '%s' from interface '%s'.\n", retWlanDeleteProfile, csApName, sNicGuid.c_str() );
				} else 
				{
					printf( " - Deleted AP '%s' from interface '%s'.\n", csApName, sNicGuid.c_str() );
				}
			}
		}
		WlanFreeMemory( ppProfileList );
	}

	return( 0 );
}
int del_aps( vector <StringType> aLimitGuids ) 
{
	int retGetIfaces = GetIfaces();
	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}

	bool bHasOutput = false;
	
	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		StringType sNicGuid = Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid );
		if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
		{
			continue;
		}
		PWLAN_PROFILE_INFO_LIST ppProfileList = NULL;
		WlanGetProfileList( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, NULL, &ppProfileList );
		if ( ( ppProfileList->dwNumberOfItems > 0 ) && ( !bHasOutput ) ) 
		{
			bHasOutput = true;
			printf( "Access Points By Interface:\n" );
		}
		for ( DWORD dwProfCnt = 0; dwProfCnt < ppProfileList->dwNumberOfItems; dwProfCnt++ ) 
		{
			wprintf( L"%s / %s\n", ppInterfaceList->InterfaceInfo[ dwNicCnt ].strInterfaceDescription, ppProfileList->ProfileInfo[ dwProfCnt ].strProfileName );
			DWORD retWlanDeleteProfile = WlanDeleteProfile( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, ppProfileList->ProfileInfo[ dwProfCnt ].strProfileName, NULL );
		}
		WlanFreeMemory( ppProfileList );
	}

	if ( bHasOutput ) 
	{
		printf( "\n" );
	}

	return( 0 );
}

int AddAp( StringType FileName, vector <StringType> aLimitGuids ) 
{
	int retGetIfaces = GetIfaces();
	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}
        del_aps(aLimitGuids);
	StringType strProfileXml;
	OpenXmlFile( FileName, strProfileXml );
	std::wstring swTemp = s2ws( strProfileXml );
	LPCWSTR wsProfileXml = swTemp.c_str();

	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		StringType sNicGuid = Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid );
		if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
		{
			continue;
		}

		DWORD pdwReasonCode = NULL;
		DWORD retWlanSetProfile = WlanSetProfile( hClientHandle, &ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid, 0, wsProfileXml, NULL, TRUE, NULL, &pdwReasonCode );
		if ( retWlanSetProfile != ERROR_SUCCESS ) 
		{
			WCHAR swcReason[256];
			WlanReasonCodeToString( pdwReasonCode, _countof( swcReason ), swcReason, NULL );
			printf( " - Error adding AP from file '%s' to interface '%s': (%d)", FileName.c_str(), sNicGuid.c_str(), pdwReasonCode );
			wprintf( L"%s\n", swcReason );
		} else 
		{
			printf( " - Added AP from file '%s' to interface '%s'.\n", FileName.c_str(), sNicGuid.c_str() );
		}
	}

	return( 0 );
}

int WaitForService( SC_HANDLE hService, DWORD dwPending, DWORD dwEnding ) 
{
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint; 
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;
	if ( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof( SERVICE_STATUS_PROCESS ), &dwBytesNeeded ) ) 
	{
		return( 0 );
	}
	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;
	while ( ssStatus.dwCurrentState == dwPending ) 
	{
		dwWaitTime = ssStatus.dwWaitHint / 10;

		if ( dwWaitTime < 1000 ) 
		{
			dwWaitTime = 1000;
		} else if ( dwWaitTime > 10000 ) 
		{
			dwWaitTime = 10000;
		}

		Sleep( dwWaitTime );
		if ( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof( SERVICE_STATUS_PROCESS ), &dwBytesNeeded ) ) 
		{
			break;
		}

		if ( ssStatus.dwCheckPoint > dwOldCheckPoint ) 
		{
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		} else 
		{
			if( ( GetTickCount() - dwStartTickCount ) > ssStatus.dwWaitHint ) 
			{
                break;
			}
		}
	}

	if ( ssStatus.dwCurrentState == dwEnding ) 
	{
		return( 1 );
	} else 
	{
		return( 0 );
	}
}

int StopWzcService( SC_HANDLE hService ) 
{
	SERVICE_STATUS ssStatus;
	if ( ControlService( hService, SERVICE_CONTROL_STOP, &ssStatus ) ) 
	{
		return( WaitForService( hService, SERVICE_STOP_PENDING, SERVICE_STOPPED ) );
	} else 
	{
		return( 0 );
	}
}

int StartWzcService( SC_HANDLE hService ) 
{
	if ( StartService( hService, NULL, NULL ) ) 
	{
		return( WaitForService( hService, SERVICE_START_PENDING, SERVICE_RUNNING ) );
	} else 
	{
		return( 0 );
	}
}

SC_HANDLE GetWzcService() {
	SC_HANDLE hSCManager = OpenSCManager( L"", SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS );
	if ( hSCManager != NULL ) 
	{
		SC_HANDLE hService = OpenService( hSCManager, L"WZCSVC", SC_MANAGER_ALL_ACCESS );
		if ( hService != NULL ) 
		{
			StartWzcService( hService );
			return( hService );
		} else 
		{

			return( NULL );
		}
	} else 
	{
		return( NULL );
	}
}

int EnableInterface( vector <StringType> aLimitGuids ) 
{
	int retGetIfaces = GetIfaces();
	bool bStopService = false;

	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}

	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		StringType sNicGuid = Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid );
		if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
		{
			continue;
		}
		HKEY phkResult = NULL;
		StringType slpSubKey( "SOFTWARE\\Microsoft\\WZCSVC\\Parameters\\Interfaces\\" );
		slpSubKey.append( sNicGuid );
		std::wstring swlpSubKey = s2ws( slpSubKey );

		if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, swlpSubKey.c_str(), 0, KEY_READ | KEY_SET_VALUE, &phkResult ) == ERROR_SUCCESS ) 
		{
			DWORD lpcbData = ( sizeof( DWORD ) );
			DWORD lpData = 0;
			if ( RegQueryValueEx( phkResult, L"ControlFlags", NULL, NULL, (LPBYTE)&lpData, &lpcbData ) == ERROR_SUCCESS ) 
			{
				if ( ( lpData | 0x04008000 ) != lpData ) 
				{
					lpData |= 0x04008000;
					if ( !bStopService ) {
						bStopService = true;
						// Stop WZCSVC before making changes to the registry
						StopWzcService( hWzcService );
					}
					RegSetValueEx( phkResult, L"ControlFlags", NULL, REG_DWORD, (LPBYTE)&lpData, lpcbData );
				}
			}
			RegCloseKey( phkResult );
		}
	}

	// Restart WZCSVC so changes take effect
	StartWzcService( hWzcService );
	return( 0 );
}


int DisableZeroConfig( vector <StringType> aLimitGuids ) 
{
	int retGetIfaces = GetIfaces();
	bool bStopService = false;

	if ( retGetIfaces != 0 ) 
	{
		printf( " - Error starting Wifi API\n" );
		return( -1 );
	}

	for( DWORD dwNicCnt = 0; dwNicCnt < ppInterfaceList->dwNumberOfItems; dwNicCnt++ ) 
	{
		StringType sNicGuid = Guid2String( ppInterfaceList->InterfaceInfo[ dwNicCnt ].InterfaceGuid );
		if ( ( aLimitGuids.size() != 0 ) && ( !vector_search( aLimitGuids, sNicGuid ) ) ) 
		{
			continue;
		}
		HKEY phkResult = NULL;
		StringType slpSubKey( "SOFTWARE\\Microsoft\\WZCSVC\\Parameters\\Interfaces\\" );
		slpSubKey.append( sNicGuid );
		std::wstring swlpSubKey = s2ws( slpSubKey );

		if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, swlpSubKey.c_str(), 0, KEY_READ | KEY_SET_VALUE, &phkResult ) == ERROR_SUCCESS ) 
		{
			DWORD lpcbData = ( sizeof( DWORD ) );
			DWORD lpData = 0;
			if ( RegQueryValueEx( phkResult, L"ControlFlags", NULL, NULL, (LPBYTE)&lpData, &lpcbData ) == ERROR_SUCCESS ) 
			{
				if ( ( lpData & 0x00008000 ) != 0 ) 
				{
					lpData &= 0xffff7fff;
					if ( !bStopService ) {
						bStopService = true;
						// Stop WZCSVC before making changes to the registry
						StopWzcService( hWzcService );
					}
					RegSetValueEx( phkResult, L"ControlFlags", NULL, REG_DWORD, (LPBYTE)&lpData, lpcbData );
				}
			}
			RegCloseKey( phkResult );
		}
	}

	// Restart WZCSVC so changes take effect
	StartWzcService( hWzcService );
	return( 0 );
}


int main( int argc,  char** argv )
{
	CCmdLine cmdLine;

	if ( cmdLine.SplitLine( argc, argv ) < 1 ) 
	{
		print_help();
		return( 0 );
	}

	if ( cmdLine.HasSwitch( "-help" ) ) 
	{
		print_help();
		return( 0 );
	}

	//  GUID Limit gather Section
	vector <StringType> aLimitGuids;
	if ( cmdLine.HasSwitch( "-limit" ) ) 
	{
		StringType sGuid;
		int iArgCnt = 0;
		
		sGuid = cmdLine.GetSafeArgument( "-limit", iArgCnt, "" );
		while( sGuid != "" ) 
		{
			if ( !vector_search( aLimitGuids, sGuid ) ) 
			{
				aLimitGuids.push_back( sGuid );
			}

			iArgCnt++;
			sGuid = cmdLine.GetSafeArgument( "-limit", iArgCnt, "" );
		}
	}
	// Ensure WZCSVC is started
	hWzcService = GetWzcService();
	if ( hWzcService == NULL ) 
	{
		printf( "Unable to access Wireless Zero Configuration service.\n" );
		return( 1 );
	}

	// Enable Interface section
	bool RequiredArgs = false;
	if ( cmdLine.HasSwitch( "-enable" ) ) 
	{
		RequiredArgs = true;
		EnableInterface( aLimitGuids );
	}

	// Disable Winidows to control the interface section
	if ( cmdLine.HasSwitch( "-disable" ) ) 
	{
		RequiredArgs = true;
		DisableZeroConfig(aLimitGuids );
	}


	// Delete, + Add section
	vector <StringType> aSwitches;
	aSwitches.push_back( "-delete" );
	aSwitches.push_back( "-add" );

	for ( int iSwitchCnt = 0; iSwitchCnt < (int)aSwitches.size() ; iSwitchCnt++ ) 
	{
		if ( cmdLine.HasSwitch( aSwitches[ iSwitchCnt ].c_str() ) ) 
		{
			StringType sSwitchAction;
			int iArgCnt = 0;

			sSwitchAction = cmdLine.GetSafeArgument( aSwitches[ iSwitchCnt ].c_str(), iArgCnt, "" );
			while ( sSwitchAction != "" ) 
			{
				RequiredArgs = true;
				if ( aSwitches[ iSwitchCnt ] == "-delete" ) 
				{
					DeleteAp( sSwitchAction, aLimitGuids );
				} else if ( aSwitches[ iSwitchCnt ] == "-add" ) 
				{
					AddAp( sSwitchAction, aLimitGuids );
				}
				iArgCnt++;
				sSwitchAction = cmdLine.GetSafeArgument( aSwitches[ iSwitchCnt ].c_str(), iArgCnt, "" );
			}
		}
	}

	// NIC + AP list section
	if ( cmdLine.HasSwitch( "-nics" ) ) 
	{
		RequiredArgs = true;
		get_nics();
	}
	if ( cmdLine.HasSwitch( "-aps" ) ) 
	{
		RequiredArgs = true;
		get_aps( aLimitGuids );
	}
	if ( cmdLine.HasSwitch( "-connect" ) ) 
	{
		RequiredArgs = true;
		StringType sSSID;
		int iArgCnt = 0;
		sSSID = cmdLine.GetSafeArgument( "-connect", iArgCnt, "" );
		connect( sSSID,aLimitGuids );
	}
	if ( cmdLine.HasSwitch( "-query" ) ) 
	{
		RequiredArgs = true;
		StringType qPAR;
		int iArgCnt = 0;
		qPAR = cmdLine.GetSafeArgument( "-query", iArgCnt, "" );
		if(!(strcmp(qPAR.c_str(),"status")))
			launch_query( aLimitGuids,STAT );
		if(!(strcmp(qPAR.c_str(),"bssid")))
			launch_query( aLimitGuids,BSSID );
	}
	if ( cmdLine.HasSwitch( "-getip" ) ) 
	{
		RequiredArgs = true;
		get_ip( aLimitGuids,IPADD );
	}
	if ( cmdLine.HasSwitch( "-dns" ) ) 
	{
		RequiredArgs = true;
		get_ip( aLimitGuids,DNS );
	}
	if ( cmdLine.HasSwitch( "-mask" ) ) 
	{
		RequiredArgs = true;
		get_ip( aLimitGuids,MASK );
	}
	if ( cmdLine.HasSwitch( "-dhcp" ) ) 
	{
		RequiredArgs = true;
		get_ip( aLimitGuids,DHCP );
	}
	if ( cmdLine.HasSwitch( "-mac" ) ) 
	{
		RequiredArgs = true;
		get_ip( aLimitGuids,MAC );
	}
	if ( cmdLine.HasSwitch( "-all" ) ) 
	{
		RequiredArgs = true;
		get_ip( aLimitGuids,ALL );
	}
	if ( cmdLine.HasSwitch( "-ip" ) ) 
	{
		RequiredArgs = true;
		int iArgCnt = 0;
		NewIPStr = cmdLine.GetSafeArgument( "-ip", iArgCnt, "" );
	}
	if ( cmdLine.HasSwitch( "-mask" ) ) 
	{
		RequiredArgs = true;
		int iArgCnt = 0;
		NewMaskStr = cmdLine.GetSafeArgument( "-mask", iArgCnt, "" );
	}
	if ( cmdLine.HasSwitch( "-debo" ) ) 
	{
		RequiredArgs = true;
		get_ip( aLimitGuids,DEBO );
	}
	if ( cmdLine.HasSwitch( "-addip" ) ) 
	{
		RequiredArgs = true;
		get_ip( aLimitGuids,ADDIP );
	}
	if ( cmdLine.HasSwitch( "-delap" ) ) 
	{
		RequiredArgs = true;
		del_aps( aLimitGuids );
	}

	if ( !RequiredArgs ) 
	{
		print_help();
	}

	CloseServiceHandle( hWzcService );
	StopServices();
	return( 0 );
}
