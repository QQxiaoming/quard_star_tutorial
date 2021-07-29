(:*******************************************************:)
(: Test: K-DateTimeEQ-3                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'ne' for xs:dateTime.         :)
(:*******************************************************:)
xs:dateTime("2004-08-12T23:01:05.12") ne
			   xs:dateTime("2004-08-12T23:01:04.12")