(:*******************************************************:)
(: Test: K-SeqExprCast-185                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that a xs:dayTimeDuration value with a large minute component is serialized properly. :)
(:*******************************************************:)
xs:string(xs:dayTimeDuration("PT6000M")) eq "P4DT4H"