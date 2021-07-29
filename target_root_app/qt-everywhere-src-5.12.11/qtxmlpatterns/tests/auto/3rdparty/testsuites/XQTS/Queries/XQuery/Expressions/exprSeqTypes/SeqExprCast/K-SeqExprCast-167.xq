(:*******************************************************:)
(: Test: K-SeqExprCast-167                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a negative xs:dayTimeDuration to xs:duration. :)
(:*******************************************************:)

			xs:string(xs:duration(xs:dayTimeDuration("-P31DT3H2M10.001S")))
			eq "-P31DT3H2M10.001S"