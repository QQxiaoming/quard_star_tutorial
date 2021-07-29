(:*******************************************************:)
(: Test: K-DateTimeEQ-4                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'ne' for xs:dateTime.         :)
(:*******************************************************:)
not(xs:dateTime("2004-07-12T23:01:04.12") ne
			       xs:dateTime("2004-07-12T23:01:04.12"))