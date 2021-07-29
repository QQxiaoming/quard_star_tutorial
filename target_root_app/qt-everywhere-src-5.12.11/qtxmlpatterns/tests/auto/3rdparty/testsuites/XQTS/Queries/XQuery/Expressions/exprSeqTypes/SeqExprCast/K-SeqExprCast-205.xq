(:*******************************************************:)
(: Test: K-SeqExprCast-205                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that a xs:yearMonthDuration value with a large year component is serialized properly. :)
(:*******************************************************:)
xs:string(xs:yearMonthDuration("P2Y323M")) eq "P28Y11M"