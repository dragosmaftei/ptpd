## ptpd changes

Files changed:

* src/dep/ptpd_dep.h
    * prototypes for addSecurityTLV and msgUnpackSecurityTLV
* src/dep/msg.c
    * packICV, unpackICV
    * msgPackSecurityTLV, msgUnpackSecurityTLV
    * **addSecurityTLV**
* src/dep/configdefaults.c
    * set default length field in rtOpts securityOpts
* src/dep/daemonconfig.c
    * tohex and keyStringToBinary
    * read security options from config file
* src/dep/net.c
    * debugs, trying to see what's defined
* src/dep/sys.c
    * include datatypes.h
    * added security errors to status file
<p></p>
* src/Makefile.am
* src/ptp_datatypes.h
    * structs: ICV, SecurityTLV (operate w/ secTLV.def)
* src/constants.h
    * SECURITY=0x000D
    * other constants...
        * SEC\_TLV\_IMM\_HMACSHA256\_LENGTH
	    * MAX\_SECURITY\_KEY\_LENGTH
	    * GDOI, TESLA
	    * MAX\_NUM\_TIMING\_MEASUREMENTS
* src/datatypes.h
    * security error counters
    * SecurityOpts struct
        * RunTimeOpts holds SecurityOpts and 'securityEnabled' bool
    * SecurityTiming struct for measuring elapsed time processing security
        * PtpClock holds SecurityTiming struct 
* src/ptpd.h
    * define DM\_MSGS
    * include dm\_security.h 
* src/protocol.c
    * recordTimingMeasurement
    * processMessage logic for receive side
    * send side:
        * issueAnnounceSingle
	    * issueSyncSingle
	    * issueFollowup
	    * issuePdelayRequest
	    * issuePdelayResponse
	    * issuePdelayRespFollowUp
* src/display.c
    * dump security errors and timing measurements to log on SIGUSR2
* gitignore
    * /src/Makefile.am
    * config files
    * CMakeLists.txt ??
    * build scripts
    * cmake-build-debug??
    * src/.libs/??


Files added:

* src/def/securityTLV/securityTLV.def
* src/dm_security.c
    * wrappers around HMAC and EVP\_sha256 to avoid define conflicts
* src/dm_security.h
    * prototypes...
* test/pp_slave.conf
* test/pp_master.conf

