(:*******************************************************:)
(: Test: K-SeqExprCast-183                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that a xs:dayTimeDuration value with a large day component is serialized properly. :)
(:*******************************************************:)
xs:string(xs:dayTimeDuration("P9876DT1M")) eq "P9876DT1M"