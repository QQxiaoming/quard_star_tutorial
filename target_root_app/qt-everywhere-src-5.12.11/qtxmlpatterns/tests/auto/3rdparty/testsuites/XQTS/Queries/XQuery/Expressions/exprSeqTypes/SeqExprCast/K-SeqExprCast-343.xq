(:*******************************************************:)
(: Test: K-SeqExprCast-343                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure when casting xs:dateTime to xs:string that milli seconds have no trailing zeros. :)
(:*******************************************************:)
xs:string(xs:dateTime("2002-02-15T21:01:23.100")) eq
						 "2002-02-15T21:01:23.1"