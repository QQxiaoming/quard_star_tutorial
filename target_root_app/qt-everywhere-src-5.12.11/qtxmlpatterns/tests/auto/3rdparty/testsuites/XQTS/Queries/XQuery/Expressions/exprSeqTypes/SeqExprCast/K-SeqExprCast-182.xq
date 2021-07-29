(:*******************************************************:)
(: Test: K-SeqExprCast-182                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting a xs:duration with zeroed time components to xs:string. :)
(:*******************************************************:)
xs:string(xs:dayTimeDuration("-P5DT0H0M0.000S")) eq "-P5D"