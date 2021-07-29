(:*******************************************************:)
(: Test: K-SeqExprCast-207                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The canonical form of the xs:yearMonthDuration value -P0M is P0M. :)
(:*******************************************************:)
xs:string(xs:yearMonthDuration("-P0M")) eq "P0M"