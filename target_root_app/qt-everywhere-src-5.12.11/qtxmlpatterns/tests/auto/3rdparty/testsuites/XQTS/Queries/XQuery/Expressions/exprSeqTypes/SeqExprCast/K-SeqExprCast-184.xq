(:*******************************************************:)
(: Test: K-SeqExprCast-184                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that a xs:dayTimeDuration value with a large hour component is serialized properly. :)
(:*******************************************************:)
xs:string(xs:dayTimeDuration("PT9876H1M")) eq "P411DT12H1M"