(:*******************************************************:)
(: Test: K-SeqExprCast-347                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure when casting xs:dateTime to xs:string that a milli seconds part of '11' is handled properly. :)
(:*******************************************************:)
xs:string(xs:dateTime("2002-02-15T21:01:23.11")) eq
						 "2002-02-15T21:01:23.11"