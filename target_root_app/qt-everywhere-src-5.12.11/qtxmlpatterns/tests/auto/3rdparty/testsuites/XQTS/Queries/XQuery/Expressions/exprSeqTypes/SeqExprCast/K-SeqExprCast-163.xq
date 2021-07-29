(:*******************************************************:)
(: Test: K-SeqExprCast-163                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that a xs:duration value with a small second component is serialized properly. :)
(:*******************************************************:)
xs:string(xs:duration("P31DT3H2M10.001S"))
		eq "P31DT3H2M10.001S"