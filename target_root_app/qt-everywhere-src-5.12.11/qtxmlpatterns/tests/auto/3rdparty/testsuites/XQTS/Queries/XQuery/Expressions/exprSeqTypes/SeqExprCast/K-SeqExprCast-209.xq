(:*******************************************************:)
(: Test: K-SeqExprCast-209                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting xs:dayTimeDuration to xs:yearMonthDuration. :)
(:*******************************************************:)

			xs:string(xs:yearMonthDuration(xs:dayTimeDuration("P31DT3H2M10.001S")))
			eq "P0M"