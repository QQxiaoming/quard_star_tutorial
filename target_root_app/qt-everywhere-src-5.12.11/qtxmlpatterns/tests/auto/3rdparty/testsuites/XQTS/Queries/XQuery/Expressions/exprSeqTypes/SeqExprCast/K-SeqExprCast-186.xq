(:*******************************************************:)
(: Test: K-SeqExprCast-186                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that a xs:dayTimeDuration value with a large second component is serialized properly. :)
(:*******************************************************:)
xs:string(xs:dayTimeDuration("PT1M1231.432S")) eq "PT21M31.432S"