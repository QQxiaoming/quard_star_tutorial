(:*******************************************************:)
(: Test: K-SeqExprCast-211                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a negative xs:dayTimeDuration to xs:yearMonthDuration. :)
(:*******************************************************:)

			xs:string(xs:yearMonthDuration(xs:dayTimeDuration("-P31DT3H2M10.001S")))
			eq "P0M"