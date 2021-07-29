(:*******************************************************:)
(: Test: K-SeqExprCast-342                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure when casting xs:dateTime to xs:string that an empty milli seconds part is not serialized. :)
(:*******************************************************:)
xs:string(xs:dateTime("2002-02-15T21:01:23.000")) eq
						 "2002-02-15T21:01:23"