(:*******************************************************:)
(: Test: K-DateTimeEQ-2                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:dateTime.         :)
(:*******************************************************:)
not(xs:dateTime("2004-08-12T23:01:04.12") eq
			       xs:dateTime("2004-08-12T23:01:04.13"))