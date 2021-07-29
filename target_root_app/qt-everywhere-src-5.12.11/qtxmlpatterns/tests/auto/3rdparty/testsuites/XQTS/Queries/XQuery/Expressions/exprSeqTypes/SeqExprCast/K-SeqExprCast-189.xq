(:*******************************************************:)
(: Test: K-SeqExprCast-189                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting xs:duration to xs:dayTimeDuration. :)
(:*******************************************************:)

			xs:string(xs:dayTimeDuration(xs:duration("P3Y0M31DT3H2M10.001S")))
			eq "P31DT3H2M10.001S"