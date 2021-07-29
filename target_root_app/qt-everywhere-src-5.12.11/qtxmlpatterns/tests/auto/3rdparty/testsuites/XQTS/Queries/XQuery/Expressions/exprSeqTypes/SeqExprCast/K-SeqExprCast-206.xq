(:*******************************************************:)
(: Test: K-SeqExprCast-206                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that a xs:yearMonthDuration value with a large year and month component is serialized properly. :)
(:*******************************************************:)
xs:string(xs:yearMonthDuration("-P543Y456M")) eq "-P581Y"