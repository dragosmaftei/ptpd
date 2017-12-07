## ptpd changes

###files changed:

structs & constants

* src/ptp_datatypes.h
    * struct: SecurityTLV (operate w/ secTLV.def)
* src/datatypes.h
    * security error counters
	* IntegrityAlgTyp enum
    * SecurityOpts struct
        * RunTimeOpts holds SecurityOpts and 'securityEnabled' bool
    * SecurityTiming struct for measuring elapsed time processing security
        * PtpClock holds SecurityTiming struct 
* src/constants.h
    * SECURITY=0x000D
    * other constants...
        * SEC\_TLV\_CONSTANT\_LEN
	    * MAX\_SEC\_KEY\_LEN
		* IV\_LEN
	    * SPI\_DISCLOSED\_KEY
		* HMAC\_SHA256\_OID, GMAC\_OID, MAX\_OID\_LEN
	    * MAX\_NUM\_TIMING\_MEASUREMENTS
* src/ptpd.h
    * define DM\_MSGS, DEFINE\_SECURITY
    * include dm\_security.h 
<p></p>
functions

* src/dep/ptpd_dep.h
    * prototypes functions in msg.c below
* src/dep/msg.c
    * msgPackSecurityTLV, msgUnpackSecurityTLV
    * **addSecurityTLV**
	* calculateAndPackICV, calculateAndVerifyICV
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
<p></p>
options and measurements

* src/Makefile.am
    * build with / without security
* configure.ac
	* configure time switch to enable security with preproc #DEFINEs
* src/dep/configdefaults.c
    * set default alg type 
* src/dep/daemonconfig.c
    * tohex and stringToBinary
    * read security options from config file
* src/dep/sys.c
    * added security errors to status file
* src/display.c
    * dump security errors and timing measurements to log on SIGUSR2
* gitignore
	* .idea
    * config files
    * CMakeLists.txt ??
    * cmake-build-debug??
    * src/.libs/??


###files added:

* src/def/securityTLV/securityTLV.def
* src/dm_security.c
    * wrappers around HMAC and EVP\_sha256 to avoid define conflicts
	* dm\_GMAC function to calculate ICV with GMAC
* src/dm_security.h
    * prototypes...
* test/pp_slave.conf
* test/pp_master.conf

