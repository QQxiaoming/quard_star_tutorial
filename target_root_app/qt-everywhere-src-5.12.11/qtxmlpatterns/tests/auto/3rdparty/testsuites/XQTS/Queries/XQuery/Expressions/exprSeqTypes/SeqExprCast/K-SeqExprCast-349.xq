(:*******************************************************:)
(: Test: K-SeqExprCast-349                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple xs:dateTime involving time with no milli seconds. :)
(:*******************************************************:)
string(xs:dateTime("2000-08-01T12:44:05")) eq
		 "2000-08-01T12:44:05"