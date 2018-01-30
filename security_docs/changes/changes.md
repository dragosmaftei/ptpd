## ptpd changes

### files changed:

structs & constants

* src/ptp_datatypes.h
    * struct: SecurityTLV (operate w/ secTLV.def)
* src/datatypes.h
    * security error counters, delayed/TESLA info counters
	* IntegrityAlgTyp enum
    * SecurityOpts struct (immutable security variables set at runtime)
        * RunTimeOpts holds SecurityOpts and 'securityEnabled' bool
	* SecurityDS struct (for mutable security data)
		* PtpClock holds SecurityDS
    * SecurityTiming struct for measuring elapsed time processing security
        * PtpClock holds SecurityTiming struct 
* src/constants.h
    * SECURITY=0x000D
    * other constants...
        * SEC\_TLV\_CONSTANT\_LEN
	    * MAX\_SEC\_KEY\_LEN
	    * SPI\_DISCLOSED\_KEY
		* MAX\_OID\_LEN, HMAC\_SHA256\_OID, GMAC\_OID, 
		* HMAC\_SHA256\_ICV\_LEN, GMAC\_ICV\_LEN, GMAC\_IV\_LEN
		* MAX\_NUM\_TIMING\_MEASUREMENTS
* src/ptpd.h
    * define SEC\_MSGS, DEFINE\_SECURITY
    * include security.h and sec\_buffers.h
<p></p>
functions

* src/dep/ptpd_dep.h
    * prototypes functions in msg.c below
* src/dep/msg.c
    * msgPackSecurityTLV, msgUnpackSecurityTLV
    * **addSecurityTLV**
	* calculateAndPackICV, calculateAndVerifyICV
* src/dep/startup.c
	* freeSecurityOpts
	* in ptpdShutdown, freeing buffers and secOpts
	* in ptpdStartup, initializing securityDS
* src/protocol.c
    * recordTimingMeasurement
    * processMessage logic for receive side
    * send side, calling addSecurityTLV() in:
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
    * read security options from config file w/ configMapType
	* additional logic section, initializing options, mallocs, etc...
* src/dep/sys.c
    * added security errors and delayed/TESLA info counters to status file
* src/display.c
    * dump security errors, delayed/TESLA info counters, and timing measurements to log on SIGUSR2
* gitignore
	* .idea
    * config files
    * CMakeLists.txt ??
    * cmake-build-debug??
    * src/.libs/??


### files added:

* src/def/securityTLV/securityTLV.def - constant fields only
* src/security.c and security.h
    * wrappers around HMAC and EVP\_sha256 to avoid define conflicts
	* GMAC function to calculate ICV with GMAC
	* delayed/TESLA helpers:
		* generate\_chain
		* generate\_icv\_key
		* verify\_key
* src/sec\_buffers.c and sec\_buffers.h
    * for delayed/TESLA, BufferedMsg and Buffer structs and functions for buffering incoming messages
* test/pp_slave.conf
* test/pp_master.conf

